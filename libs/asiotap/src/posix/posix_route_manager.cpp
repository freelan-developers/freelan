/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file posix_route_manager.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The POSIX route manager class.
 */

#include "posix/posix_route_manager.hpp"

#include "error.hpp"

#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>

#ifdef LINUX
#include <netlinkplus/manager.hpp>
#endif

#include <executeplus/posix_system.hpp>

namespace asiotap
{
	posix_route_manager::route_type posix_route_manager::get_route_for(const boost::asio::ip::address& host)
	{
#ifdef MACINTOSH
		const std::string inet = host.is_v6() ? "-inet6" : "-inet";
		const std::vector<std::string> real_args { "/sbin/route", "-n", "get", inet, boost::lexical_cast<std::string>(host) };

		std::stringstream ss;
		executeplus::checked_execute(real_args, executeplus::get_current_environment(), &ss);

		//The output is like:
		/*
			   route to: 8.8.8.8
			destination: default
				   mask: default
				gateway: 10.7.0.254
			  interface: en0
				  flags: <UP,GATEWAY,DONE,STATIC,PRCLONING>
			 recvpipe  sendpipe  ssthresh  rtt,msec    rttvar  hopcount      mtu     expire
				   0         0         0         0         0         0      1500         0
		*/
		std::map<std::string, std::string> values;
		std::string line;

		while (std::getline(ss, line))
		{
			const auto colon_pos = line.find_first_of(':');

			if (colon_pos == std::string::npos)
			{
				continue;
			}

			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1, std::string::npos);

			boost::algorithm::trim(key);
			boost::algorithm::trim(value);

			values[key] = value;
		}

		if (values.find("interface") == values.end())
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		const std::string interface = values["interface"];
		boost::optional<boost::asio::ip::address> gw;

		if (values.find("gateway") != values.end())
		{
			gw = boost::asio::ip::address::from_string(values["gateway"]);
		}
#else
#if defined(FREELAN_DISABLE_NETLINK) || !defined(LINUX)
		const std::vector<std::string> real_args { "/bin/ip", "route", "get", boost::lexical_cast<std::string>(host) };

		std::stringstream ss;
		executeplus::checked_execute(real_args, executeplus::get_current_environment(), &ss);

		// The output is like:
		/*
			8.8.8.8 via 37.59.15.254 dev eth0  src 46.105.57.112
				cache
		*/

		std::string host_addr;

		if (!(ss >> host_addr))
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		std::map<std::string, std::string> values;
		std::string key, value;

		while (ss >> key >> value)
		{
			boost::algorithm::trim(key);
			boost::algorithm::trim(value);

			values[key] = value;
		}

		if (values.find("dev") == values.end())
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_output_parsing_error));
		}

		const std::string interface = values["dev"];
		boost::optional<boost::asio::ip::address> gw;

		if (values.find("via") != values.end())
		{
			gw = boost::asio::ip::address::from_string(values["via"]);
		}
#else
		const auto route_info = m_netlink_manager.get_route_for(host);
		const auto interface = route_info.output_interface.name();
		const auto gw = route_info.gateway;
#endif
#endif

		const auto route = to_ip_route(to_network_address(host), gw);
		const posix_route_manager::route_type route_entry = { interface, route, 0 };

		return route_entry;
	}

	void posix_route_manager::ifconfig(const std::string& interface, const ip_network_address& address)
	{
#if defined(FREELAN_DISABLE_NETLINK) || !defined(LINUX)
		const std::vector<std::string> real_args { "/sbin/ifconfig", interface, boost::lexical_cast<std::string>(address) };

		executeplus::checked_execute(real_args, executeplus::get_current_environment());
#else
		m_netlink_manager.add_interface_address(netlinkplus::interface_entry(interface), to_ip_address(address), to_prefix_length(address));
#endif
	}

	void posix_route_manager::ifconfig(const std::string& interface, const ip_network_address& address, const boost::asio::ip::address& remote_address)
	{
#if defined(FREELAN_DISABLE_NETLINK) || !defined(LINUX)
		const std::vector<std::string> real_args { "/sbin/ifconfig", interface, boost::lexical_cast<std::string>(address), boost::lexical_cast<std::string>(remote_address) };

		executeplus::checked_execute(real_args, executeplus::get_current_environment());
#else
		m_netlink_manager.add_interface_address(netlinkplus::interface_entry(interface), remote_address, to_prefix_length(address), to_ip_address(address));
#endif
	}

	void posix_route_manager::set_route(route_action action, const std::string& interface, const ip_network_address& dest)
	{
#if defined(FREELAN_DISABLE_NETLINK) || !defined(LINUX)
		const std::string net_host = is_unicast(dest) ? "-host" : "-net";
#ifdef MACINTOSH
		const std::string command = action == route_action::add ? "add" : "delete";
		const std::string inet = boost::apply_visitor(ip_network_address_ip_address_visitor(), dest).is_v6() ? "-inet6" : "-inet";
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, inet, boost::lexical_cast<std::string>(dest), "-interface", interface };
#else
		const std::string command = action == route_action::add ? "add" : "del";
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), "dev", interface };
#endif

		executeplus::checked_execute(real_args, executeplus::get_current_environment());
#else
		if (action == route_action::add)
		{
			m_netlink_manager.add_route(netlinkplus::interface_entry(interface), to_ip_address(dest), to_prefix_length(dest));
		}
		else
		{
			m_netlink_manager.remove_route(netlinkplus::interface_entry(interface), to_ip_address(dest), to_prefix_length(dest));
		}
#endif
	}

	void posix_route_manager::set_route(route_action action, const std::string& interface, const ip_network_address& dest, const boost::asio::ip::address& gateway)
	{
#if defined(FREELAN_DISABLE_NETLINK) || !defined(LINUX)
		const std::string net_host = is_unicast(dest) ? "-host" : "-net";
		const std::string command = action == route_action::add ? "add" : "del";
#ifdef MACINTOSH
		static_cast<void>(interface);
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), boost::lexical_cast<std::string>(gateway) };
#else
		const std::vector<std::string> real_args { "/sbin/route", "-n", command, net_host, boost::lexical_cast<std::string>(dest), "gw", boost::lexical_cast<std::string>(gateway), "dev", interface };
#endif

		executeplus::checked_execute(real_args, executeplus::get_current_environment());
#else
		if (action == route_action::add)
		{
			m_netlink_manager.add_route(netlinkplus::interface_entry(interface), to_ip_address(dest), to_prefix_length(dest), gateway);
		}
		else
		{
			m_netlink_manager.remove_route(netlinkplus::interface_entry(interface), to_ip_address(dest), to_prefix_length(dest), gateway);
		}
#endif
	}

	void posix_route_manager::register_route(const route_type& route_entry)
	{
		const auto _gateway = gateway(route_entry.route);
		const auto ina = network_address(route_entry.route);

		if (_gateway)
		{
				set_route(route_action::add, route_entry.interface, ina, *_gateway);
		}
		else
		{
				set_route(route_action::add, route_entry.interface, ina);
		}
	}

	void posix_route_manager::unregister_route(const route_type& route_entry)
	{
		const auto _gateway = gateway(route_entry.route);
		const auto ina = network_address(route_entry.route);

		if (_gateway)
		{
				set_route(route_action::remove, route_entry.interface, ina, *_gateway);
		}
		else
		{
				set_route(route_action::remove, route_entry.interface, ina);
		}
	}
}

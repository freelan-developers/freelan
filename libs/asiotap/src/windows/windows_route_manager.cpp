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
 * \file windows_route_manager.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows route manager class.
 */

#include "windows/windows_route_manager.hpp"

#include "windows/netsh.hpp"
#include "error.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <Iphlpapi.h>

#include <executeplus/windows_system.hpp>

namespace asiotap
{
	namespace
	{
		void set_sockaddr_inet(SOCKADDR_INET& result, const boost::asio::ip::address& address)
		{
			if (address.is_v4())
			{
				const auto bytes = address.to_v4().to_bytes();

				std::memcpy(&result.Ipv4.sin_addr, bytes.data(), bytes.size());
				result.Ipv4.sin_family = AF_INET;
			}
			else if (address.is_v6())
			{
				const auto bytes = address.to_v6().to_bytes();

				std::memcpy(&result.Ipv6.sin6_addr, bytes.data(), bytes.size());
				result.Ipv6.sin6_family = AF_INET6;
			}
			else
			{
				throw boost::system::system_error(make_error_code(asiotap_error::invalid_type));
			}
		}

		boost::optional<boost::asio::ip::address> from_sockaddr_inet(const SOCKADDR_INET& sai)
		{
			if (sai.si_family == AF_INET)
			{
				typedef boost::asio::ip::address_v4 addr_type;

				addr_type::bytes_type bytes;
				std::memcpy(bytes.data(), &sai.Ipv4.sin_addr, bytes.size());
				const addr_type result(bytes);

				if (result != addr_type::any())
				{
					return result;
				}
			}
			else if (sai.si_family == AF_INET6)
			{
				typedef boost::asio::ip::address_v6 addr_type;

				addr_type::bytes_type bytes;
				std::memcpy(bytes.data(), &sai.Ipv6.sin6_addr, bytes.size());
				const addr_type result(bytes);

				if (result != addr_type::any())
				{
					return result;
				}
			}

			return boost::none;
		}

		MIB_IPFORWARD_ROW2 make_ip_forward_row(const NET_LUID& interface_luid, const ip_route& route, unsigned int metric)
		{
			const auto ina = network_address(route);
			const auto gw = gateway(route);

			MIB_IPFORWARD_ROW2 entry{};

			::InitializeIpForwardEntry(&entry);

			entry.Protocol = MIB_IPPROTO_NETMGMT;
			entry.InterfaceLuid = interface_luid;
			entry.Metric = metric;

			if (gw)
			{
				set_sockaddr_inet(entry.NextHop, *gw);
			}

			const auto network_ip_address = to_ip_address(ina);
			const auto network_prefix_length = to_prefix_length(ina);

			set_sockaddr_inet(entry.DestinationPrefix.Prefix, network_ip_address);
			entry.DestinationPrefix.PrefixLength = network_prefix_length;

			return entry;
		}

		MIB_UNICASTIPADDRESS_ROW make_unicast_ip_address_row(const NET_LUID& interface_luid, const ip_network_address& network_address)
		{
			MIB_UNICASTIPADDRESS_ROW entry{};

			InitializeUnicastIpAddressEntry(&entry);

			entry.InterfaceLuid = interface_luid;

			const auto network_ip_address = to_ip_address(network_address);
			const auto network_prefix_length = to_prefix_length(network_address);

			set_sockaddr_inet(entry.Address, network_ip_address);
			entry.OnLinkPrefixLength = network_prefix_length;

			return entry;
		}
	}

	void windows_route_manager::register_route(const route_type& route_entry)
	{
		register_route(route_entry.interface, route_entry.route, route_entry.metric);
	}

	void windows_route_manager::unregister_route(const route_type& route_entry)
	{
		unregister_route(route_entry.interface, route_entry.route, route_entry.metric);
	}

	void windows_route_manager::netsh_interface_ip_set_dhcp(const std::string& interface_name, bool persistent)
	{
		std::vector<std::string> args {
			"interface",
			"ip",
			"set",
			"address",
			"dhcp",
			"name=" + interface_name,
			persistent ? "store=persistent" : "store=active"
		};

#ifdef UNICODE
		std::vector<std::wstring> wargs;

		for (auto&& arg : args)
		{
			wargs.push_back(multi_byte_to_wide_char(arg));
		}

		netsh(wargs);
#else
		netsh(args);
#endif
	}

	void windows_route_manager::netsh_interface_ip_set_address(const std::string& interface_name, const ip_network_address& address, bool persistent)
	{
		std::vector<std::string> args;

		if (to_ip_address(address).is_v4())
		{
			args = {
				"interface",
				"ip",
				"set",
				"address",
				"name=" + interface_name,
				"source=static",
				"addr=" + boost::lexical_cast<std::string>(address),
				"gateway=none",
				persistent ? "store=persistent" : "store=active"
			};
		}
		else
		{
			args = {
				"interface",
				"ipv6",
				"set",
				"address",
				"interface=" + interface_name,
				"address=" + boost::lexical_cast<std::string>(address),
				persistent ? "store=persistent" : "store=active"
			};
		}

#ifdef UNICODE
		std::vector<std::wstring> wargs;

		for (auto&& arg : args)
		{
			wargs.push_back(multi_byte_to_wide_char(arg));
		}

		netsh(wargs);
#else
		netsh(args);
#endif
	}

	windows_route_manager::route_type windows_route_manager::get_route_for(const boost::asio::ip::address& host)
	{
		SOCKADDR_INET dest_addr {};
		set_sockaddr_inet(dest_addr, host);

		MIB_IPFORWARD_ROW2 best_route{};
		::InitializeIpForwardEntry(&best_route);
		SOCKADDR_INET best_source_address {};

		const DWORD result = ::GetBestRoute2(NULL, 0, NULL, &dest_addr, 0, &best_route, &best_source_address);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}

		const auto gw = from_sockaddr_inet(best_route.NextHop);
		const auto route = to_ip_route(to_network_address(host), gw);
		const windows_route_manager::route_type route_entry = { best_route.InterfaceLuid, route, 0 };

		return route_entry;
	}

	void windows_route_manager::register_route(const NET_LUID& interface_luid, const ip_route& route, unsigned int metric)
	{
		const auto row = make_ip_forward_row(interface_luid, route, metric);

		const DWORD result = ::CreateIpForwardEntry2(&row);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}
	}

	void windows_route_manager::unregister_route(const NET_LUID& interface_luid, const ip_route& route, unsigned int metric)
	{
		const auto row = make_ip_forward_row(interface_luid, route, metric);

		const DWORD result = ::DeleteIpForwardEntry2(&row);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}
	}

	void windows_route_manager::set_unicast_address(const NET_LUID& interface_luid, const ip_network_address& network_address)
	{
		const auto row = make_unicast_ip_address_row(interface_luid, network_address);

		const DWORD result = ::SetUnicastIpAddressEntry(&row);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}
	}
}

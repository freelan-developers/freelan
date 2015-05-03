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
 * \file windows_dns_servers_manager.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows DNS servers manager class.
 */

#include "windows/windows_dns_servers_manager.hpp"

#include "windows/netsh.hpp"

#include <executeplus/windows_system.hpp>

namespace asiotap
{
	namespace
	{
		void netsh_interface_ip_generic_dns(const std::string& verb, const std::string& interface_name, const boost::asio::ip::address& dns_server)
		{
			std::vector<std::string> args;

			args = {
				"interface",
				dns_server.is_v4() ? "ipv4" : "ipv6",
				verb,
				"dnsservers",
				"name=" + interface_name
			};

			if (verb != "delete") {
				args.push_back("source=static");
			}
			
			args.push_back("address=" + boost::lexical_cast<std::string>(dns_server.to_string()));

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

		void netsh_interface_ip_set_dnsservers(const std::string& interface_name, const boost::asio::ip::address& dns_server)
		{
			return netsh_interface_ip_generic_dns("set", interface_name, dns_server);
		}

		void netsh_interface_ip_add_dnsservers(const std::string& interface_name, const boost::asio::ip::address& dns_server)
		{
			return netsh_interface_ip_generic_dns("add", interface_name, dns_server);
		}
		
		void netsh_interface_ip_delete_dnsservers(const std::string& interface_name, const boost::asio::ip::address& dns_server)
		{
			return netsh_interface_ip_generic_dns("delete", interface_name, dns_server);
		}
	}

	void windows_dns_servers_manager::register_dns_server(const dns_server_type& dns_server_entry)
	{
		auto& ip_addresses = m_references[dns_server_entry.interface_name];

		if (ip_addresses.empty()) {
			netsh_interface_ip_set_dnsservers(dns_server_entry.interface_name, dns_server_entry.dns_server_address.value());
		} else {
			netsh_interface_ip_add_dnsservers(dns_server_entry.interface_name, dns_server_entry.dns_server_address.value());
		}

		ip_addresses.insert(dns_server_entry.dns_server_address);
	}

	void windows_dns_servers_manager::unregister_dns_server(const dns_server_type& dns_server_entry)
	{
		auto& ip_addresses = m_references[dns_server_entry.interface_name];

		const auto index = ip_addresses.find(dns_server_entry.dns_server_address);

		if (index == ip_addresses.end()) {
			// Okay, we try to delete an address that was never added by us. This doesn't seem too good.
			assert(false);
		} else {
			netsh_interface_ip_delete_dnsservers(dns_server_entry.interface_name, dns_server_entry.dns_server_address.value());

			ip_addresses.erase(index);
		}
	}
}

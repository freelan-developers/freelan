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

#include <Iphlpapi.h>

#include "error.hpp"

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

		MIB_IPFORWARD_ROW2 make_row(const windows_route_manager::route_type& route)
		{
			MIB_IPFORWARD_ROW2 entry{};

			::InitializeIpForwardEntry(&entry);

			entry.Protocol = MIB_IPPROTO_NETMGMT;

			if (route.interface)
			{
				entry.InterfaceLuid = *route.interface;
			}

			if (route.gateway)
			{
				set_sockaddr_inet(entry.NextHop, *route.gateway);
			}

			const auto network_ip_address = ip_address(route.network);
			const auto network_prefix_length = prefix_length(route.network);

			set_sockaddr_inet(entry.DestinationPrefix.Prefix, network_ip_address);
			entry.DestinationPrefix.PrefixLength = network_prefix_length;

			return entry;
		}
	}

	void windows_route_manager::register_route(const route_type& route)
	{
		const auto row = make_row(route);

		const DWORD result = ::CreateIpForwardEntry2(&row);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}
	}

	void windows_route_manager::unregister_route(const route_type& route)
	{
		const auto row = make_row(route);

		const DWORD result = ::DeleteIpForwardEntry2(&row);

		if (result != NO_ERROR)
		{
			throw boost::system::system_error(result, boost::system::system_category());
		}
	}
}

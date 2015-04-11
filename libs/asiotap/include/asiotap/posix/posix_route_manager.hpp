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
 * \file posix_route_manager.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The POSIX route manager class.
 */

#pragma once

#include "../os.hpp"
#include "../base_route_manager.hpp"
#include "../types/ip_network_address.hpp"

#include <string>

#ifdef LINUX
#include <netlinkplus/manager.hpp>
#endif

namespace asiotap
{
	typedef base_routing_table_entry<std::string> posix_routing_table_entry;

	class posix_route_manager : public base_route_manager<posix_route_manager, posix_routing_table_entry>
	{
		public:

			explicit posix_route_manager(boost::asio::io_service& io_service_) :
#ifndef LINUX
				base_route_manager<posix_route_manager, posix_routing_table_entry>(io_service_)
#else
				base_route_manager<posix_route_manager, posix_routing_table_entry>(io_service_),
				m_netlink_manager(io_service_)
#endif
			{
			}

			posix_route_manager::route_type get_route_for(const boost::asio::ip::address& host);
			void ifconfig(const std::string& interface, const ip_network_address& address);
			void ifconfig(const std::string& interface, const ip_network_address& address, const boost::asio::ip::address& remote_address);

			enum class route_action {
				add,
				remove
			};

			void set_route(route_action action, const std::string& interface, const ip_network_address& dest);
			void set_route(route_action action, const std::string& interface, const ip_network_address& dest, const boost::asio::ip::address& gateway);

		protected:

			void register_route(const route_type& route);
			void unregister_route(const route_type& route);

		friend class base_route_manager<posix_route_manager, posix_routing_table_entry>;

#ifdef LINUX
		private:
			netlinkplus::manager m_netlink_manager;
#endif
	};
}

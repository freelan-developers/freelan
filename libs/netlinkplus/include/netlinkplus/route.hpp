/*
 * libnetlinkplus - A portable netlink extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libnetlinkplus.
 *
 * libnetlinkplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libnetlinkplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libnetlinkplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file route.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief netlink route classes.
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <string>

#include "protocol.hpp"

namespace netlinkplus
{
	/**
	 * \brief An interface entry class.
	 */
	class interface_entry
	{
		public:

			explicit interface_entry(unsigned int index_ = 0) :
				m_index(index_)
			{
			}

			bool is_null() const
			{
				return (m_index == 0);
			}

			unsigned int index() const
			{
				return m_index;
			}

			std::string name() const;

		private:

			unsigned int m_index;
			mutable std::string m_name_cache;

			friend std::ostream& operator<<(std::ostream& os, const interface_entry& entry)
			{
				if (entry.is_null())
				{
					return os << "none";
				}
				else
				{
					return os << entry.name() << " (" << entry.index() << ")";
				}
			}
	};

	/**
	 * \brief A route entry class.
	 */
	struct route_entry
	{
		route_entry() :
			priority{},
			metric{}
		{
		}

		boost::asio::ip::address destination;
		boost::asio::ip::address source;
		interface_entry input_interface;
		interface_entry output_interface;
		boost::optional<boost::asio::ip::address> gateway;
		unsigned int priority;
		unsigned int metric;
	};

	/**
	 * \brief Manage routes.
	 */
	class route_manager
	{
		public:

			/**
			 * \brief Create a route manager.
			 */
			route_manager(boost::asio::io_service& io_service);

			/**
			 * \brief Get the route entry for the specified host.
			 * \param host The host to get the route for.
			 * \return The route entry, if any.
			 */
			route_entry get_route_for(const boost::asio::ip::address& host);

		private:

			netlink_route_protocol::socket m_socket;
	};
}

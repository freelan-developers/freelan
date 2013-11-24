/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file router.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A router class.
 */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <algorithm>
#include <map>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <asiotap/osi/ipv4_filter.hpp>
#include <asiotap/osi/ipv6_filter.hpp>
#include <asiotap/osi/ipv4_frame.hpp>
#include <asiotap/osi/ipv6_frame.hpp>

#include "router_port.hpp"
#include "configuration.hpp"
#include "ip_network_address.hpp"

namespace freelan
{
	/**
	 * \brief A class that represents a router.
	 */
	class router
	{
		public:

			/**
			 * \brief The base port type.
			 */
			typedef router_port base_port_type;

			/**
			 * \brief The port type.
			 */
			typedef boost::shared_ptr<base_port_type> port_type;

			/**
			 * \brief The group type.
			 */
			typedef unsigned int group_type;

			/**
			 * \brief The port list type.
			 */
			typedef std::map<port_type, group_type> port_list_type;

			/**
			 * \brief Create a new router.
			 * \param configuration The router configuration.
			 */
			router(const router_configuration& configuration);

			/**
			 * \brief Invalidate the routes.
			 */
			void invalidate_routes();

			/**
			 * \brief Register a router port.
			 * \param port The port to register. Cannot be null.
			 * \param group The group of the port.
			 */
			void register_port(port_type port, group_type group = group_type());

			/**
			 * \brief Unregister a port.
			 * \param port The port to unregister. Cannot be null.
			 *
			 * If the port was not registered, nothing is done.
			 */
			void unregister_port(port_type port);

			/**
			 * \brief Check if the specified port is registered.
			 * \param port The port to check.
			 * \return true if the port is registered, false otherwise.
			 */
			bool is_registered(port_type port) const;

			/**
			 * \brief Receive data trough the specified port.
			 * \param port The port from which the data comes. Cannot be null.
			 * \param data The data.
			 */
			void receive_data(port_type port, boost::asio::const_buffer data);

		private:

			template <typename AddressType>
			void receive_data(port_type, const AddressType&, boost::asio::const_buffer);

			router_configuration m_configuration;

			port_list_type m_ports;

			asiotap::osi::filter<asiotap::osi::ipv4_frame> m_ipv4_filter;
			asiotap::osi::filter<asiotap::osi::ipv6_frame> m_ipv6_filter;

			typedef std::multimap<routes_type::value_type, port_type, routes_compare> routes_port_type;

			const routes_port_type& routes() const;
			mutable boost::optional<routes_port_type> m_routes;
	};

	inline router::router(const router_configuration& configuration) :
		m_configuration(configuration)
	{
	}

	inline void router::invalidate_routes()
	{
		m_routes = boost::none;
	}

	inline void router::register_port(port_type port, group_type group)
	{
		m_ports[port] = group;

		// The routes cache must be recompiled.
		invalidate_routes();
	}

	inline void router::unregister_port(port_type port)
	{
		m_ports.erase(port);

		// The routes cache must be recompiled.
		invalidate_routes();
	}

	inline bool router::is_registered(port_type port) const
	{
		return (m_ports.find(port) != m_ports.end());
	}
}

#endif /* ROUTER_HPP */

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
#include <set>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/make_shared.hpp>

#include <asiotap/osi/ipv4_filter.hpp>
#include <asiotap/osi/ipv6_filter.hpp>
#include <asiotap/osi/ipv4_frame.hpp>
#include <asiotap/osi/ipv6_frame.hpp>
#include <asiotap/types/ip_network_address.hpp>

#include "configuration.hpp"
#include "port_index.hpp"
#include "routes_message.hpp"

namespace freelan
{
	/**
	 * \brief A class that represents a router.
	 */
	class router
	{
		public:

			/**
			 * \brief The port group type.
			 */
			typedef unsigned int port_group_type;

			/**
			 * \brief A router port type.
			 */
			class port_type
			{
				public:

					/**
					 * \brief The write handler type.
					 */
					typedef boost::function<void (boost::system::error_code)> write_handler_type;

					/**
					 * \brief A write function type.
					 */
					typedef boost::function<void (boost::asio::const_buffer data, write_handler_type handler)> write_function_type;

					/**
					 * \brief Create a new default port.
					 */
					port_type() :
						m_write_function(),
						m_local_routes(),
						m_group(),
						m_router(NULL)
					{}

					/**
					 * \brief Create a new port.
					 * \param write_function The write function to use.
					 * \param _group The group this port belongs to.
					 */
					port_type(write_function_type write_function, port_group_type _group) :
						m_write_function(write_function),
						m_local_routes(),
						m_group(_group),
						m_router(NULL)
					{}

					/**
					 * \brief Copy constructor.
					 * \param other The other instance.
					 */
					port_type(const port_type& other) :
						m_write_function(other.m_write_function),
						m_local_routes(other.m_local_routes),
						m_group(other.m_group),
						m_router(NULL)
					{}

					/**
					 * \brief Destructor.
					 */
					~port_type()
					{
						dissociate_from_router();
					}

					/**
					 * \brief Assignment operator.
					 * \param other The other instance.
					 * \return *this.
					 */
					port_type& operator=(const port_type& other)
					{
						dissociate_from_router();

						m_write_function = other.m_write_function;
						m_local_routes = other.m_local_routes;
						m_group = other.m_group;

						return *this;
					}

					/**
					 * \brief Write data to the port.
					 * \param data The data to write.
					 * \param handler The handler to call when the write is complete.
					 */
					void async_write(boost::asio::const_buffer data, write_handler_type handler) const
					{
						m_write_function(data, handler);
					}

					const asiotap::ip_route_set& local_routes() const
					{
						return m_local_routes;
					}

					const asiotap::ip_address_set& local_dns_servers() const
					{
						return m_local_dns_servers;
					}

					void set_local_routes(const asiotap::ip_route_set& _local_routes)
					{
						m_local_routes = _local_routes;

						if (m_router)
						{
							m_router->invalidate_routes();
						}
					}

					void set_local_dns_servers(const asiotap::ip_address_set& _local_dns_servers)
					{
						m_local_dns_servers = _local_dns_servers;
					}

					port_group_type group() const
					{
						return m_group;
					}

				private:

					void associate_to_router(router* _router)
					{
						m_router = _router;

						if (m_router)
						{
							m_router->invalidate_routes();
						}
					}

					void dissociate_from_router()
					{
						if (m_router)
						{
							m_router->invalidate_routes();

							m_router = NULL;
						}
					}

					friend class router;

					write_function_type m_write_function;
					asiotap::ip_route_set m_local_routes;
					asiotap::ip_address_set m_local_dns_servers;
					port_group_type m_group;
					router* m_router;
			};

			/**
			 * \brief The port list type.
			 */
			typedef std::map<port_index_type, port_type> port_list_type;

			/**
			 * \brief Create a new router.
			 * \param configuration The router configuration.
			 */
			router(const router_configuration& configuration) :
				m_configuration(configuration)
			{}

			/**
			 * \brief Invalidate the routes cache.
			 */
			void invalidate_routes()
			{
				m_routes = boost::none;
			}

			/**
			 * \brief Register a router port.
			 * \param index The index of the port.
			 * \param port The port to register. Cannot be null.
			 */
			void register_port(port_index_type index, port_type port)
			{
				port_type& local_port = (m_ports[index] = port);

				// This takes care of automatically clearing the route cache whenever needed.
				local_port.associate_to_router(this);
			}

			/**
			 * \brief Unregister a port.
			 * \param index The port to unregister. Cannot be null.
			 *
			 * If the port was not registered, nothing is done.
			 */
			void unregister_port(port_index_type index)
			{
				m_ports.erase(index);
			}

			/**
			 * \brief Check if the specified port is registered.
			 * \param index The port to check.
			 * \return true if the port is registered, false otherwise.
			 */
			bool is_registered(port_index_type index) const
			{
				return (m_ports.find(index) != m_ports.end());
			}

			/**
			 * \brief Get the port associated to a given index, if it exists.
			 * \param index The index of the port to get.
			 * \return A pointer to the port.
			 */
			port_type* get_port(port_index_type index)
			{
				const port_list_type::iterator port_entry = m_ports.find(index);

				if (port_entry == m_ports.end())
				{
					return nullptr;
				}

				return &port_entry->second;
			}

			/**
			 * \brief Receive data trough the specified port.
			 * \param index The port from which the data comes.
			 * \param data The data to write.
			 * \param handler The handler to call when the write is complete.
			 */
			void async_write(port_index_type index, boost::asio::const_buffer data, port_type::write_handler_type handler);

		private:

			std::vector<const port_type*> get_targets_for(port_index_type, boost::asio::const_buffer);

			template <typename AddressType>
			std::vector<const port_type*> get_targets_for(port_index_type, const AddressType&);

			router_configuration m_configuration;

			port_list_type m_ports;

			asiotap::osi::filter<asiotap::osi::ipv4_frame> m_ipv4_filter;
			asiotap::osi::filter<asiotap::osi::ipv6_frame> m_ipv6_filter;

			typedef std::multimap<asiotap::ip_route, port_index_type> routes_port_type;

			const routes_port_type& routes() const;
			mutable boost::optional<routes_port_type> m_routes;
	};
}

#endif /* ROUTER_HPP */

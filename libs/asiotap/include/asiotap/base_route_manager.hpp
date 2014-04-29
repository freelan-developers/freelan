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
 * \file base_route_manager.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The base route manager class.
 */

#ifndef ASIOTAP_BASE_ROUTE_MANAGER_HPP
#define ASIOTAP_BASE_ROUTE_MANAGER_HPP

#include <string>
#include <map>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>

#include "types/ip_route.hpp"

namespace asiotap
{
	/**
	 * \brief A routing table entry.
	 */
	template <typename InterfaceType>
	struct base_routing_table_entry
	{
		typedef InterfaceType interface_type;

		InterfaceType interface;
		ip_route route;
		unsigned int metric;

		friend bool operator==(const base_routing_table_entry& lhs, const base_routing_table_entry& rhs)
		{
			return ((lhs.interface == rhs.interface) && (lhs.route == rhs.route));
		}

		friend bool operator<(const base_routing_table_entry& lhs, const base_routing_table_entry& rhs)
		{
			if (lhs.interface == rhs.interface)
			{
				return (lhs.route < rhs.route);
			}
			else
			{
				return (lhs.interface < rhs.interface);
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const base_routing_table_entry& value)
		{
			return os << value.interface << " - " << value.route << " - metric " << value.metric;
		}
	};

	/**
	 * \brief Handle system routes.
	 */
	template <typename RouteManagerType, typename RouteType>
	class base_route_manager
	{
		public:
			typedef RouteType route_type;

			class entry_type_impl
			{
				public:

					~entry_type_impl()
					{
						if (m_success)
						{
							m_route_manager.unregister_route(m_route);
						}
					}

					entry_type_impl(const entry_type_impl&) = delete;
					entry_type_impl& operator=(const entry_type_impl&) = delete;

					entry_type_impl(entry_type_impl&&) = delete;
					entry_type_impl& operator=(entry_type_impl&&) = delete;

					const route_type& route() const
					{
						return m_route;
					}

				private:

					entry_type_impl(base_route_manager& route_manager, const route_type& _route) :
						m_route_manager(route_manager),
						m_route(_route),
						m_success(m_route_manager.register_route(m_route))
					{
					}

					base_route_manager& m_route_manager;
					route_type m_route;
					bool m_success;

					friend class base_route_manager<RouteManagerType, RouteType>;
			};

			/**
			 * \brief The entry implementation type.
			 */
			typedef boost::shared_ptr<entry_type_impl> entry_type;

			/**
			 * \brief The registration success handler type.
			 */
			typedef boost::function<void (const route_type&)> route_registration_success_handler_type;

			/**
			* \brief The registration failure handler type.
			*/
			typedef boost::function<void(const route_type&, const boost::system::system_error&)> route_registration_failure_handler_type;

			/**
			* \brief The unregistration success handler type.
			*/
			typedef boost::function<void (const route_type&)> route_unregistration_success_handler_type;

			/**
			 * \brief The unregistration failure handler type.
			 */
			typedef boost::function<void(const route_type&, const boost::system::system_error&)> route_unregistration_failure_handler_type;

			explicit base_route_manager(boost::asio::io_service& io_service_) :
				m_io_service(io_service_)
			{
			}

			base_route_manager(const base_route_manager&) = delete;
			base_route_manager& operator=(const base_route_manager&) = delete;

			base_route_manager(base_route_manager&&) = delete;
			base_route_manager& operator=(base_route_manager&&) = delete;

			boost::asio::io_service& io_service()
			{
				return m_io_service;
			}

			void set_route_registration_success_handler(route_registration_success_handler_type handler)
			{
				m_route_registration_success_handler = handler;
			}

			void set_route_registration_failure_handler(route_registration_failure_handler_type handler)
			{
				m_route_registration_failure_handler = handler;
			}

			void set_route_unregistration_success_handler(route_unregistration_success_handler_type handler)
			{
				m_route_unregistration_success_handler = handler;
			}

			void set_route_unregistration_failure_handler(route_unregistration_failure_handler_type handler)
			{
				m_route_unregistration_failure_handler = handler;
			}

			bool register_route(const route_type& route)
			{
				try
				{
					static_cast<RouteManagerType*>(this)->register_route(route);

					if (m_route_registration_success_handler)
					{
						m_route_registration_success_handler(route);
					}
				}
				catch (boost::system::system_error& ex)
				{
					if (m_route_registration_failure_handler)
					{
						m_route_registration_failure_handler(route, ex);
					}

					return false;
				}

				return true;
			}

			bool unregister_route(const route_type& route)
			{
				try
				{
					static_cast<RouteManagerType*>(this)->unregister_route(route);

					if (m_route_unregistration_success_handler)
					{
						m_route_unregistration_success_handler(route);
					}
				}
				catch (boost::system::system_error& ex)
				{
					if (m_route_unregistration_failure_handler)
					{
						m_route_unregistration_failure_handler(route, ex);
					}

					return false;
				}

				return true;
			}

			entry_type get_route_entry(const route_type& route)
			{
				entry_type entry = m_entry_table[route].lock();

				if (!entry)
				{
					entry = boost::shared_ptr<entry_type_impl>(new entry_type_impl(*this, route));

					m_entry_table[route] = entry;
				}

				return entry;
			}

		protected:

			typedef std::map<route_type, boost::weak_ptr<entry_type_impl>> entry_table_type;

		private:

			boost::asio::io_service& m_io_service;
			entry_table_type m_entry_table;
			route_registration_success_handler_type m_route_registration_success_handler;
			route_registration_failure_handler_type m_route_registration_failure_handler;
			route_unregistration_success_handler_type m_route_unregistration_success_handler;
			route_unregistration_failure_handler_type m_route_unregistration_failure_handler;
	};
}

#endif /* ASIOTAP_BASE_ROUTE_MANAGER_HPP */

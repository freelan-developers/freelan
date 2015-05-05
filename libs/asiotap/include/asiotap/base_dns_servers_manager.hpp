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
 * \file base_dns_servers_manager.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The base DNS servers manager class.
 */

#pragma once

#include <map>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>

#include "types/ip_endpoint.hpp"

namespace asiotap
{
	/**
	 * \brief Handle DNS servers.
	 */
	template <typename DNSServersManagerType>
	class base_dns_servers_manager
	{
		public:
			struct dns_server_type
			{
				std::string interface_name;
				ip_address dns_server_address;

				friend bool operator<(const dns_server_type& lhs, const dns_server_type& rhs) {
					if (lhs.interface_name == rhs.interface_name) {
						return (lhs.dns_server_address < rhs.dns_server_address);
					} else {
						return (lhs.interface_name < rhs.interface_name);
					}
				}

				friend bool operator==(const dns_server_type& lhs, const dns_server_type& rhs) {
					return ((lhs.interface_name == rhs.interface_name) && (lhs.dns_server_address == rhs.dns_server_address));
				}

				friend std::ostream& operator<<(std::ostream& os, const dns_server_type& value) {
					return os << value.interface_name << " - " << value.dns_server_address;
				}
			};

			class entry_type_impl
			{
				public:

					~entry_type_impl()
					{
						if (m_success)
						{
							m_dns_servers_manager.unregister_dns_server(m_dns_server);
						}
					}

					entry_type_impl(const entry_type_impl&) = delete;
					entry_type_impl& operator=(const entry_type_impl&) = delete;

					entry_type_impl(entry_type_impl&&) = delete;
					entry_type_impl& operator=(entry_type_impl&&) = delete;

					const dns_server_type& dns_server() const
					{
						return m_dns_server;
					}

				private:

					entry_type_impl(base_dns_servers_manager& dns_server_manager, const dns_server_type& _dns_server) :
						m_dns_servers_manager(dns_server_manager),
						m_dns_server(_dns_server),
						m_success(m_dns_servers_manager.register_dns_server(m_dns_server))
					{
					}

					base_dns_servers_manager& m_dns_servers_manager;
					dns_server_type m_dns_server;
					bool m_success;

					friend class base_dns_servers_manager<DNSServersManagerType>;
			};

			/**
			 * \brief The entry implementation type.
			 */
			typedef boost::shared_ptr<entry_type_impl> entry_type;

			/**
			 * \brief The add handler type.
			 */
			typedef boost::function<bool (const dns_server_type&)> dns_server_add_handler_type;

			/**
			 * \brief The remove handler type.
			 */
			typedef boost::function<bool (const dns_server_type&)> dns_server_remove_handler_type;

			/**
			 * \brief The registration success handler type.
			 */
			typedef boost::function<void(const dns_server_type&)> dns_server_registration_success_handler_type;

			/**
			* \brief The registration failure handler type.
			*/
			typedef boost::function<void(const dns_server_type&, const boost::system::system_error&)> dns_server_registration_failure_handler_type;

			/**
			* \brief The unregistration success handler type.
			*/
			typedef boost::function<void(const dns_server_type&)> dns_server_unregistration_success_handler_type;

			/**
			 * \brief The unregistration failure handler type.
			 */
			typedef boost::function<void(const dns_server_type&, const boost::system::system_error&)> dns_server_unregistration_failure_handler_type;

			explicit base_dns_servers_manager(boost::asio::io_service& io_service_) :
				m_io_service(io_service_),
				m_dns_server_add_handler(),
				m_dns_server_remove_handler(),
				m_dns_server_registration_success_handler(),
				m_dns_server_registration_failure_handler(),
				m_dns_server_unregistration_success_handler(),
				m_dns_server_unregistration_failure_handler()
			{
			}

			base_dns_servers_manager(const base_dns_servers_manager&) = delete;
			base_dns_servers_manager& operator=(const base_dns_servers_manager&) = delete;

			base_dns_servers_manager(base_dns_servers_manager&&) = delete;
			base_dns_servers_manager& operator=(base_dns_servers_manager&&) = delete;

			boost::asio::io_service& io_service()
			{
				return m_io_service;
			}

			void set_dns_server_add_handler(dns_server_add_handler_type handler)
			{
				m_dns_server_add_handler = handler;
			}

			void set_dns_server_remove_handler(dns_server_remove_handler_type handler)
			{
				m_dns_server_remove_handler = handler;
			}

			void set_dns_server_registration_success_handler(dns_server_registration_success_handler_type handler)
			{
				m_dns_server_registration_success_handler = handler;
			}

			void set_dns_server_registration_failure_handler(dns_server_registration_failure_handler_type handler)
			{
				m_dns_server_registration_failure_handler = handler;
			}

			void set_dns_server_unregistration_success_handler(dns_server_unregistration_success_handler_type handler)
			{
				m_dns_server_unregistration_success_handler = handler;
			}

			void set_dns_server_unregistration_failure_handler(dns_server_unregistration_failure_handler_type handler)
			{
				m_dns_server_unregistration_failure_handler = handler;
			}

			bool register_dns_server(const dns_server_type& dns_server)
			{
				try
				{
					bool result = false;

					if (m_dns_server_add_handler) {
						result = m_dns_server_add_handler(dns_server);
					}

					if (!result) {
						static_cast<DNSServersManagerType*>(this)->register_dns_server(dns_server);
					}

					if (m_dns_server_registration_success_handler)
					{
						m_dns_server_registration_success_handler(dns_server);
					}
				}
				catch (boost::system::system_error& ex)
				{
					if (m_dns_server_registration_failure_handler)
					{
						m_dns_server_registration_failure_handler(dns_server, ex);
					}

					return false;
				}

				return true;
			}

			bool unregister_dns_server(const dns_server_type& dns_server)
			{
				try
				{
					bool result = false;

					if (m_dns_server_remove_handler) {
						result = m_dns_server_remove_handler(dns_server);
					}

					if (!result) {
						static_cast<DNSServersManagerType*>(this)->unregister_dns_server(dns_server);
					}

					if (m_dns_server_unregistration_success_handler)
					{
						m_dns_server_unregistration_success_handler(dns_server);
					}
				}
				catch (boost::system::system_error& ex)
				{
					if (m_dns_server_unregistration_failure_handler)
					{
						m_dns_server_unregistration_failure_handler(dns_server, ex);
					}

					return false;
				}

				return true;
			}

			entry_type get_dns_server_entry(const dns_server_type& dns_server)
			{
				entry_type entry = m_entry_table[dns_server].lock();

				if (!entry)
				{
					entry = boost::shared_ptr<entry_type_impl>(new entry_type_impl(*this, dns_server));

					m_entry_table[dns_server] = entry;
				}

				return entry;
			}

		protected:

			typedef std::map<dns_server_type, boost::weak_ptr<entry_type_impl>> entry_table_type;

		private:

			boost::asio::io_service& m_io_service;
			entry_table_type m_entry_table;
			dns_server_add_handler_type m_dns_server_add_handler;
			dns_server_remove_handler_type m_dns_server_remove_handler;
			dns_server_registration_success_handler_type m_dns_server_registration_success_handler;
			dns_server_registration_failure_handler_type m_dns_server_registration_failure_handler;
			dns_server_unregistration_success_handler_type m_dns_server_unregistration_success_handler;
			dns_server_unregistration_failure_handler_type m_dns_server_unregistration_failure_handler;
	};
}

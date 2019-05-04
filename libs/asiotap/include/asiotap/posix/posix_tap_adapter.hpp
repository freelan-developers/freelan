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
 * \file posix_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The POSIX tap adapter class.
 */

#ifndef ASIOTAP_POSIX_TAP_ADAPTER_HPP
#define ASIOTAP_POSIX_TAP_ADAPTER_HPP

#include "../base_tap_adapter.hpp"

#include "posix_route_manager.hpp"
#include "posix_dns_servers_manager.hpp"

#include <map>
#include <string>

namespace asiotap
{
	class posix_tap_adapter : public base_tap_adapter<boost::asio::posix::stream_descriptor>
	{
		public:

			/**
			 * \brief Enumerate the tap adapters available on the system.
			 * \param _layer The layer of the adapters to list.
			 * \return A map of the tap adapter identifiers with their names.
			 */
			static std::map<std::string, std::string> enumerate(tap_adapter_layer _layer);

			/**
			 * \brief Create a new tap adapter.
			 * \param _io_service The io_service to attach to.
			 * \param _layer The layer of the tap adapter.
			 */
			posix_tap_adapter(boost::asio::io_service& _io_service, tap_adapter_layer _layer) :
				base_tap_adapter(_io_service, _layer),
				m_route_manager(_io_service)
			{}

			/**
			 * \brief Destroy the tap adapter.
			 */
			~posix_tap_adapter()
			{
				if (is_open())
				{
					boost::system::error_code ec;

					// This is required on OSX.
					destroy_device(ec);

					// We do nothing with the error code as errors can happen legitimately.
				}
			}

			posix_tap_adapter(const posix_tap_adapter&) = delete;
			posix_tap_adapter& operator=(const posix_tap_adapter&) = delete;

			/**
			 * \brief Get the associated network manager.
			 * \return The associated network manager.
			 */
			posix_route_manager& network_manager()
			{
				return m_route_manager;
			}

			/**
			 * \brief Open the first available tap adapter.
			 * \param ec The error code.
			 */
			void open(boost::system::error_code& ec);

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter to open.
			 * \param ec The error code.
			 */
			void open(const std::string& name, boost::system::error_code& ec);

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter to open. If name is empty, then the first available tap adapter is opened.
			 */
			void open(const std::string& name = "");

			/**
			 * \brief Close the associated descriptor.
			 */
			void close()
			{
				boost::system::error_code ec;

				destroy_device(ec);

				// We do nothing with the error code as errors can happen legitimately.

				base_tap_adapter::close();
			}

			/**
			 * \brief Close the associated descriptor.
			 * \param ec The error code.
			 */
			boost::system::error_code close(boost::system::error_code& ec)
			{
				destroy_device(ec);

				base_tap_adapter::close(ec);

				return ec;
			}

			/**
			 * \brief Set the tap adapter connected state.
			 * \param connected The connected state.
			 */
			void set_connected_state(bool connected);

			/**
			 * \brief Get the IP addresses of the adapter.
			 * \return The IP addresses.
			 */
			ip_network_address_list get_ip_addresses();

			/**
			 * \brief Configure the tap adapter.
			 * \param configuration The IP configuration.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void configure(const configuration_type& configuration);

			/**
			 * \brief Build a route associated to this tap adapter.
			 * \param route The route.
			 * \return The route.
			 */
			posix_routing_table_entry get_route(const ip_route& route)
			{
				return { name(), route, 0 };
			}

			/**
			 * \brief Build a DNS server entry associated to this tap adapter.
			 * \param dns_server The DNS server IP address.
			 * \return The DNS server entry.
			 */
			posix_dns_servers_manager::dns_server_type get_dns_server(const ip_address& dns_server)
			{
				return { name(), dns_server };
			}

		private:

			void update_mtu_from_device();
			void set_device_mtu(size_t _mtu);
			void set_ip_address_v4(const ipv4_network_address& network_address);
			void set_ip_address_v6(const ipv6_network_address& network_address);
			void set_remote_ip_address_v4(const ipv4_network_address& network_address, const boost::asio::ip::address_v4& remote_address);

			void destroy_device();
			void destroy_device(boost::system::error_code& ec);

			posix_route_manager m_route_manager;

			/**
			 * \brief If we use an existing tun/tap adapter.
			 */
			bool m_existing_tap;
	};
}

#endif /* ASIOTAP_POSIX_TAP_ADAPTER_HPP */

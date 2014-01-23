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
 * \file windows_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windaps tap adapter class.
 */

#ifndef ASIOTAP_WINDOWS_TAP_ADAPTER_HPP
#define ASIOTAP_WINDOWS_TAP_ADAPTER_HPP

#include "../base_tap_adapter.hpp"

#include <map>
#include <string>

namespace asiotap
{
	class windows_tap_adapter : public base_tap_adapter<boost::asio::windows::stream_handle, windows_tap_adapter>
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
			windows_tap_adapter(boost::asio::io_service& _io_service, tap_adapter_layer _layer) :
				base_tap_adapter(_io_service, _layer),
				m_display_name(),
				m_interface_index()
			{}

			windows_tap_adapter(const windows_tap_adapter&) = delete;
			windows_tap_adapter& operator=(const windows_tap_adapter&) = delete;

			/**
			 * \brief Get the device display name.
			 * \return The device display name.
			 */
			const std::string& display_name() const
			{
				return m_display_name;
			}

			/**
			 * \brief Open the first available tap adapter.
			 * \param mtu The MTU to set on the tap adapter.
			 * \param ec The error code.
			 */
			void open(size_t mtu, boost::system::error_code& ec);

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter to open.
			 * \param mtu The MTU to set on the tap adapter.
			 * \param ec The error code.
			 */
			void open(const std::string& name, size_t mtu, boost::system::error_code& ec);

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter to open. If name is empty, then the first available tap adapter is opened.
			 * \param mtu The MTU to set on the tap adapter.
			 */
			void open(const std::string& name = "", size_t mtu = 0);

			/**
			 * \brief Set the tap adapter connected state.
			 * \param connected The connected state.
			 */
			void set_connected_state(bool connected);

			/**
			 * \brief Get the IP addresses associated to this tap adapter.
			 * \return A list of IP addresses.
			 */
			std::vector<ip_address_prefix_length> get_ip_addresses();

			/**
			 * \brief Add an IPv4 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void remove_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Add an IPv6 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void add_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void remove_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

			/**
			 * \brief Set the point-to-point address on the tap adaper.
			 * \param local The local address.
			 * \param remote The remote address.
			 * \warning If a serious error occurs, an exception will be thrown.
			 * \warning Performing this operation on a device that is not in TUN mode has undefined behavior.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			void set_remote_ip_address_v4(const boost::asio::ip::address_v4& local, const boost::asio::ip::address_v4& remote);

		private:

			std::string m_display_name;
			size_t m_interface_index;

			friend ostream& operator<<(std::ostream& os, const windows_tap_adapter& value)
			{
				return os << value.display_name() << " (" << value.layer() << ")";
			}
	};
}

#endif /* ASIOTAP_WINDOWS_TAP_ADAPTER_HPP */

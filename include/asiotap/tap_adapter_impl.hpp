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
 * \file tap_adapter_impl.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The tap adapter implementation class.
 */

#ifndef ASIOTAP_TAP_ADAPTER_IMPL_HPP
#define ASIOTAP_TAP_ADAPTER_IMPL_HPP

#include "os.hpp"

#include <boost/asio.hpp>

#ifdef WINDOWS
#include <windows.h>
#else
#include <aio.h>
#endif

#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <map>

namespace asiotap
{
	class tap_adapter_impl
	{
		public:

			/**
			 * \brief The Ethernet address size.
			 */
			static const size_t ethernet_address_size = 6;

			/**
			 * \brief The Ethernet address type.
			 */
			typedef boost::array<unsigned char, ethernet_address_size> ethernet_address_type;

			/**
			 * \brief Enumerate the available tap adapter on the system.
			 * \return The identifiers and names of the available tap adapters on the system.
			 */
			static std::map<std::string, std::string> enumerate();

			/**
			 * \brief Create a new tap_adapter_impl.
			 */
			tap_adapter_impl();

			/**
			 * \brief Destroy a tap_adapter_impl.
			 */
			~tap_adapter_impl();

			/**
			 * \brief Check if the tap adapter is open.
			 * \return true if the tap adapter is open, false otherwise.
			 */
			bool is_open() const;

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter device. On Windows, a GUID is expected.
			 *
			 * If the tap adapter was already opened, it will be closed first.
			 */
			void open(const std::string& name);

			/**
			 * \brief Close the tap adapter.
			 *
			 * If the tap adapter is already closed, nothing is done.
			 */
			void close();

			/**
			 * \brief Set the connected state of the device.
			 * \param connected If true, set the device's state as up/connected.
			 * \warning The device must be open (see is_open()) or the behavior is undefined.
			 */
			void set_connected_state(bool connected);

			/**
			 * \brief Get the device name, as specified during a call to open().
			 * \return The device name. On Windows, a GUID is returned.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			const std::string& name() const;

			/**
			 * \brief Get the device MTU.
			 * \return The device MTU.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			unsigned int mtu() const;

			/**
			 * \brief Get the Ethernet address.
			 * \return The ethernet address.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			const ethernet_address_type& ethernet_address() const;

			/**
			 * \brief Start a read.
			 * \param buf The buffer to read the data to.
			 * \param buf_len The length of buf.
			 */
			void begin_read(void* buf, size_t buf_len);

			/**
			 * \brief End a read.
			 * \param cnt If the call succeed, cnt is updated to the count of bytes read.
			 * \param timeout The maximum time to wait for the read to end. A special value means "wait forever".
			 * \return true on success, false otherwise.
			 *
			 * If the call fails without throwing, you may retry the call or cancel the write operation using cancel_read().
			 */
			bool end_read(size_t& cnt, const boost::posix_time::time_duration& timeout = boost::posix_time::time_duration(boost::posix_time::not_a_date_time));

			/**
			 * \brief Start a write.
			 * \param buf The buffer to read the data from.
			 * \param buf_len The length of buf.
			 */
			void begin_write(const void* buf, size_t buf_len);

			/**
			 * \brief End a write.
			 * \param cnt If the call succeed, cnt is updated to the count of bytes written.
			 * \param timeout The maximum time to wait for the write to end. A special value means "wait forever".
			 * \return true on success, false otherwise.
			 *
			 * If the call fails without throwing, you may retry the call or cancel the write operation using cancel_write().
			 */
			bool end_write(size_t& cnt, const boost::posix_time::time_duration& timeout = boost::posix_time::time_duration(boost::posix_time::not_a_date_time));

			/**
			 * \brief Cancel any pending read operation.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel_read();

			/**
			 * \brief Cancel any pending write operation.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel_write();

			/**
			 * \brief Cancel any pending operation on the device.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel();

			/**
			 * \brief Process to a blocking read.
			 * \param buf The buffer to read the data to.
			 * \param buf_len The length of buf.
			 * \return The count of bytes read.
			 */
			size_t read(void* buf, size_t buf_len);

			/**
			 * \brief Process to a blocking write.
			 * \param buf The buffer to read to the data from.
			 * \param buf_len The length of buf.
			 * \return The count of bytes written.
			 */
			size_t write(const void* buf, size_t buf_len);

			/**
			 * \brief Add an IP address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool add_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool remove_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len);

			/**
			 * \brief Add an IPv4 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool remove_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Add an IPv6 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool add_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 */
			bool remove_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

		private:

			std::string m_name;
			unsigned int m_mtu;
			ethernet_address_type m_ethernet_address;
#ifdef WINDOWS
			HANDLE m_handle;
			std::string m_display_name;
			DWORD m_interface_index;
			OVERLAPPED m_read_overlapped;
			OVERLAPPED m_write_overlapped;
#else
			int m_device;
			struct aiocb m_read_aio;
			struct aiocb m_write_aio;
#endif
	};

	inline tap_adapter_impl::~tap_adapter_impl()
	{
		close();
	}

	inline const std::string& tap_adapter_impl::name() const
	{
		return m_name;
	}

	inline unsigned int tap_adapter_impl::mtu() const
	{
		return m_mtu;
	}

	inline const tap_adapter_impl::ethernet_address_type& tap_adapter_impl::ethernet_address() const
	{
		return m_ethernet_address;
	}

	inline void tap_adapter_impl::cancel()
	{
		cancel_read();
		cancel_write();
	}

	inline bool tap_adapter_impl::add_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
	{
		if (address.is_v4())
		{
			return add_ip_address_v4(address.to_v4(), prefix_len);
		}
		else if (address.is_v6())
		{
			return add_ip_address_v6(address.to_v6(), prefix_len);
		}
		else
		{
			throw std::logic_error("The specified address must be an IPv4 or an IPv6 address.");
		}
	}

	inline bool tap_adapter_impl::remove_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
	{
		if (address.is_v4())
		{
			return remove_ip_address_v4(address.to_v4(), prefix_len);
		}
		else if (address.is_v6())
		{
			return remove_ip_address_v6(address.to_v6(), prefix_len);
		}
		else
		{
			throw std::logic_error("The specified address must be an IPv4 or an IPv6 address.");
		}
	}
}

#endif /* ASIOTAP_TAP_ADAPTER_IMPL_HPP */

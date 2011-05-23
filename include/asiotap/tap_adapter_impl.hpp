/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
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
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The tap adapter implementation class.
 */

#ifndef ASIOTAP_TAP_ADAPTER_IMPL_HPP
#define ASIOTAP_TAP_ADAPTER_IMPL_HPP

#include "os.hpp"

#ifdef WINDOWS
#include <windows.h>
#else
#endif

#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

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
			 * \param connected If true, set the device's state as connected.
			 * \warning The device must be open (see is_open()) or the behavior is undefined.
			 *
			 * This function does nothing on *NIX.
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
			 * \param timeout The maximum time to wait for the read to end. A special value means "wait forever".
			 * \return The count of bytes read, or 0 if the read failed.
			 */
			size_t end_read(const boost::posix_time::time_duration& timeout);

		private:

			std::string m_name;
			unsigned int m_mtu;
			ethernet_address_type m_ethernet_address;
#ifdef WINDOWS
			HANDLE m_handle;
			std::string m_display_name;
			DWORD m_interface_index;
			OVERLAPPED m_read_overlapped;
#else
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
}

#endif /* ASIOTAP_TAP_ADAPTER_IMPL_HPP */

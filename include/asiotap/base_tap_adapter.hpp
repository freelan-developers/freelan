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
 * \file base_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The base tap adapter class.
 */

#ifndef ASIOTAP_BASE_TAP_ADAPTER_HPP
#define ASIOTAP_BASE_TAP_ADAPTER_HPP

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>
#include <iomanip>

#include "osi/ethernet_address.hpp"
#include "os.hpp"

namespace asiotap
{
	/**
	 * \brief The tap adapter layers.
	 */
	enum class tap_adapter_layer
	{
		ethernet,
		ip

		friend ostream& operator<<(std::ostream& os, const tap_adapter_layer& value)
		{
			switch (value)
			{
				case ethernet:
					return os << "ethernet";
				case ip:
					return os << "ip";
			}
		}
	};

	/**
	 * \brief An IP address and prefix length type.
	 */
	struct ip_address_prefix_length
	{
		boost::asio::ip::address ip_address;
		unsigned int prefix_length;

		friend bool operator<(const ip_address_prefix_length& lhs, const ip_address_prefix_length& rhs)
		{
			if (lhs.ip_address < rhs.ip_address)
			{
				return true;
			}
			else if (lhs.ip_address == rhs.ip_address)
			{
				return (lhs.prefix_length < rhs.prefix_length);
			}
			else
			{
				return false;
			}
		}

		friend bool operator==(const ip_address_prefix_length& lhs, const ip_address_prefix_length& rhs)
		{
			return ((lhs.ip_address == rhs.ip_address) && (lhs.prefix_length == rhs.prefix_length));
		}

		friend std::ostream& operator<<(std::ostream& os, const ip_address_prefix_length& v)
		{
			if (v.ip_address.is_v6())
			{
				return os << "[" << v.ip_address << "]:" << v.prefix_length;
			}
			else
			{
				return os << v.ip_address << ":" << v.prefix_length;
			}
		}
	};

	template <typename DescriptorType, typename DerivedType>
	class base_tap_adapter
	{
		public:

			/**
			 * \brief The descriptor type.
			 */
			typedef DescriptorType descriptor_type;

			/**
			 * \brief Read some data from the tap adapter.
			 * \param buffers The buffers into which the data will be read.
			 * \param handler The handler to be called when the read operation completes.
			 */
			template <typename MutableBufferSequence, typename ReadHandler>
			void async_read(const MutableBufferSequence& buffers, ReadHandler handler)
			{
				m_descriptor.async_read_some(buffers, handler);
			}

			/**
			 * \brief Write some data to the tap adapter.
			 * \param buffers One or more buffers to be written to the tap adapter.
			 * \param handler The handler to be called when the write operation completes.
			 */
			template <typename ConstBufferSequence, typename WriteHandler>
			void async_write(const ConstBufferSequence& buffers, WriteHandler handler)
			{
				m_descriptor.async_write_some(buffers, handler);
			}

			/**
			 * \brief Read some data from the tap adapter.
			 * \param buffers The buffers into which the data will be read.
			 * \return The number of bytes read.
			 */
			template <typename MutableBufferSequence>
			size_t read(const MutableBufferSequence& buffers)
			{
				return m_descriptor.read_some(buffers);
			}

			/**
			 * \brief Read some data from the tap adapter.
			 * \param buffers The buffers into which the data will be read.
			 * \param ec The error code.
			 * \return The number of bytes read.
			 */
			template <typename MutableBufferSequence>
			size_t read(const MutableBufferSequence& buffers, boost::system::error_code& ec)
			{
				return m_descriptor.read_some(buffers, ec);
			}

			/**
			 * \brief Write some data to the tap adapter.
			 * \param buffers One or more buffers to be written to the tap adapter.
			 * \return The number of bytes written.
			 */
			template <typename ConstBufferSequence>
			size_t write(const ConstBufferSequence& buffers)
			{
				return m_descriptor.write_some(buffers);
			}

			/**
			 * \brief Write some data to the tap adapter.
			 * \param buffers One or more buffers to be written to the tap adapter.
			 * \param ec The error code.
			 * \return The number of bytes written.
			 */
			template <typename ConstBufferSequence>
			size_t write(const ConstBufferSequence& buffers, boost::system::error_code& ec)
			{
				return m_descriptor.write_some(buffers, ec);
			}

			/**
			 * \brief Cancel all pending asynchronous operations associated with the tap adapter.
			 */
			void cancel()
			{
				m_descriptor.cancel();
			}

			/**
			 * \brief Cancel all pending asynchronous operations associated with the tap adapter.
			 * \param ec The error code.
			 */
			void cancel(boost::system::error_code& ec)
			{
				m_descriptor.cancel(ec);
			}

			/**
			 * \brief Get the associated io_service instance.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service()
			{
				return m_descriptor.get_io_service();
			}

			/**
			 * \brief Get the layer of the tap adapter.
			 * \return The layer.
			 */
			tap_adapter_layer layer() const
			{
				return m_layer;
			}

			/**
			 * \brief Get the device name.
			 * \return The device name.
			 */
			const std::string& name() const
			{
				return m_name;
			}

			/**
			 * \brief Get the device MTU.
			 * \return The device MTU.
			 */
			size_t mtu() const
			{
				return m_mtu;
			}

			/**
			 * \brief Get the device ethernet address.
			 * \return The device ethernet address.
			 */
			const osi::ethernet_address& ethernet_address() const
			{
				return m_ethernet_address;
			}

			/**
			 * \brief Get the tap adapter current state.
			 * \return true if the tap adapter is open.
			 */
			bool is_open() const
			{
				return m_descriptor.is_open();
			}

			/**
			 * \brief Close the associated descriptor.
			 */
			void close()
			{
				m_descriptor.close();
			}

			/**
			 * \brief Close the associated descriptor.
			 * \param ec The error code.
			 */
			boost::system::error_code close(boost::system::error_code& ec)
			{
				return m_descriptor.close(ec);
			}

			/**
			 * \brief Add an IP address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void add_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
			{
				if (address.is_v4())
				{
					return static_cast<DerivedType*>(this)->add_ip_address_v4(address.to_v4(), prefix_len);
				}
				else if (address.is_v6())
				{
					return static_cast<DerivedType*>(this)->add_ip_address_v6(address.to_v6(), prefix_len);
				}
				else
				{
					throw boost::system::system_error(make_error_code(asiotap_error::invalid_type));
				}
			}

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \warning If a serious error occurs, an exception will be thrown.
			 */
			void remove_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
			{
				if (address.is_v4())
				{
					return static_cast<DerivedType*>(this)->remove_ip_address_v4(address.to_v4(), prefix_len);
				}
				else if (address.is_v6())
				{
					return static_cast<DerivedType*>(this)->remove_ip_address_v6(address.to_v6(), prefix_len);
				}
				else
				{
					throw boost::system::system_error(make_error_code(asiotap_error::invalid_type));
				}
			}

		protected:

			base_tap_adapter(boost::asio::io_service& _io_service, tap_adapter_layer _layer) :
				m_descriptor(_io_service),
				m_layer(_layer),
				m_name(),
				m_mtu(),
				m_ethernet_address()
			{}

			descriptor_type& descriptor()
			{
				return m_descriptor;
			}

			const descriptor_type& descriptor() const
			{
				return m_descriptor;
			}

			std::string& name()
			{
				return m_name;
			}

			size_t& mtu()
			{
				return m_mtu;
			}

			osi::ethernet_address& ethernet_address()
			{
				return m_ethernet_address;
			}

		private:

			descriptor_type m_descriptor;
			tap_adapter_layer m_layer;
			std::string m_name;
			size_t m_mtu;
			osi::ethernet_address m_ethernet_address;

			friend ostream& operator<<(std::ostream& os, const base_tap_adapter& value)
			{
				return os << value.name() << " (" << value.layer() << ")";
			}
	};
}

#endif /* ASIOTAP_BASE_TAP_ADAPTER_HPP */

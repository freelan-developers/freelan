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
 * \file arp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ARP helper class.
 */

#ifndef ASIOTAP_OSI_ARP_HELPER_HPP
#define ASIOTAP_OSI_ARP_HELPER_HPP

#include "helper.hpp"
#include "arp_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base arp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, arp_frame> : public _base_helper<HelperTag, arp_frame>
		{
			public:

				/**
				 * \brief Get the hardware type.
				 * \return The hardware type.
				 */
				uint16_t hardware_type() const;

				/**
				 * \brief Get the protocol type.
				 * \return The protocol type.
				 */
				uint16_t protocol_type() const;

				/**
				 * \brief Get the hardware address length.
				 * \return The hardware address length.
				 */
				size_t hardware_address_length() const;

				/**
				 * \brief Get the logical address length.
				 * \return The logical address length.
				 */
				size_t logical_address_length() const;

				/**
				 * \brief Get the operation.
				 * \return The operation.
				 */
				uint16_t operation() const;

				/**
				 * \brief Get the sender hardware address.
				 * \return The sender hardware address.
				 */
				typename _base_helper_impl::buffer_type sender_hardware_address() const
				{
					return boost::asio::buffer(this->frame().sender_hardware_address, sizeof(this->frame().sender_hardware_address));
				}

				/**
				 * \brief Get the sender logical address.
				 * \return The sender logical address.
				 */
				boost::asio::ip::address_v4 sender_logical_address() const;

				/**
				 * \brief Get the target hardware address.
				 * \return The target hardware address.
				 */
				typename _base_helper_impl::buffer_type target_hardware_address() const
				{
					return boost::asio::buffer(this->frame().target_hardware_address, sizeof(this->frame().target_hardware_address));
				}

				/**
				 * \brief Get the target logical address.
				 * \return The target logical address.
				 */
				boost::asio::ip::address_v4 target_logical_address() const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(arp_frame);
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf);
		};

		/**
		 * \brief The mutable arp helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, arp_frame> : public _base_helper_impl<mutable_helper_tag, arp_frame>
		{
			public:

				/**
				 * \brief Set the hardware type.
				 * \param hardware_type The hardware type.
				 */
				void set_hardware_type(uint16_t hardware_type) const;

				/**
				 * \brief Set the protocol type.
				 * \param protocol_type The protocol type.
				 */
				void set_protocol_type(uint16_t protocol_type) const;

				/**
				 * \brief Set the hardware address length.
				 * \param hardware_address_length The hardware address length.
				 */
				void set_hardware_address_length(size_t hardware_address_length) const;

				/**
				 * \brief Set the logical address length.
				 * \param logical_address_length The logical address length.
				 */
				void set_logical_address_length(size_t logical_address_length) const;

				/**
				 * \brief Set the operation.
				 * \param operation The operation.
				 */
				void set_operation(uint16_t operation) const;

				/**
				 * \brief Set the sender logical address.
				 * \param sender_logical_address The sender logical address.
				 */
				void set_sender_logical_address(boost::asio::ip::address_v4 sender_logical_address) const;

				/**
				 * \brief Set the target logical address.
				 * \param target_logical_address The target logical address.
				 */
				void set_target_logical_address(boost::asio::ip::address_v4 target_logical_address) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, arp_frame>::hardware_type() const
		{
			return ntohs(this->frame().hardware_type);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, arp_frame>::protocol_type() const
		{
			return ntohs(this->frame().protocol_type);
		}

		template <class HelperTag>
		inline size_t _base_helper_impl<HelperTag, arp_frame>::hardware_address_length() const
		{
			return this->frame().hardware_address_length;
		}

		template <class HelperTag>
		inline size_t _base_helper_impl<HelperTag, arp_frame>::logical_address_length() const
		{
			return this->frame().logical_address_length;
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, arp_frame>::operation() const
		{
			return ntohs(this->frame().operation);
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, arp_frame>::sender_logical_address() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().sender_logical_address.s_addr));
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, arp_frame>::target_logical_address() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().target_logical_address.s_addr));
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, arp_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, arp_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_hardware_type(uint16_t _hardware_type) const
		{
			this->frame().hardware_type = htons(_hardware_type);
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_protocol_type(uint16_t _protocol_type) const
		{
			this->frame().protocol_type = htons(_protocol_type);
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_hardware_address_length(size_t _hardware_address_length) const
		{
			this->frame().hardware_address_length = static_cast<uint8_t>(_hardware_address_length);
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_logical_address_length(size_t _logical_address_length) const
		{
			this->frame().logical_address_length = static_cast<uint8_t>(_logical_address_length);
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_operation(uint16_t _operation) const
		{
			this->frame().operation = htons(_operation);
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_sender_logical_address(boost::asio::ip::address_v4 _sender_logical_address) const
		{
			this->frame().sender_logical_address.s_addr = htonl(_sender_logical_address.to_ulong());
		}

		inline void _helper_impl<mutable_helper_tag, arp_frame>::set_target_logical_address(boost::asio::ip::address_v4 _target_logical_address) const
		{
			this->frame().target_logical_address.s_addr = htonl(_target_logical_address.to_ulong());
		}

		inline _helper_impl<mutable_helper_tag, arp_frame>::_helper_impl(_helper_impl<mutable_helper_tag, arp_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, arp_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_ARP_HELPER_HPP */


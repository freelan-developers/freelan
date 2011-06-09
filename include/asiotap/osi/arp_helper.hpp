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
		 * \brief The const arp helper class.
		 */
		typedef const_helper<arp_frame> const_arp_helper;

		/**
		 * \brief The mutable arp helper class.
		 */
		typedef mutable_helper<arp_frame> mutable_arp_helper;

		/**
		 * \brief The const arp helper implementation class.
		 */
		template <>
		class _const_helper_impl<arp_frame> : public _base_const_helper<arp_frame>
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
				boost::asio::const_buffer sender_hardware_address() const;

				/**
				 * \brief Get the sender logical address.
				 * \return The sender logical address.
				 */
				boost::asio::ip::address_v4 sender_logical_address() const;

				/**
				 * \brief Get the target hardware address.
				 * \return The target hardware address.
				 */
				boost::asio::const_buffer target_hardware_address() const;

				/**
				 * \brief Get the target logical address.
				 * \return The target logical address.
				 */
				boost::asio::ip::address_v4 target_logical_address() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_const_helper_impl(const arp_frame& frame);
		};
		
		/**
		 * \brief The mutable arp helper implementation class.
		 */
		template <>
		class _mutable_helper_impl<arp_frame> : public _base_mutable_helper<arp_frame>
		{
			public:

				/**
				 * \brief Get the hardware type.
				 * \return The hardware type.
				 */
				uint16_t hardware_type() const;

				/**
				 * \brief Set the hardware type.
				 * \param hardware_type The hardware type.
				 */
				void set_hardware_type(uint16_t hardware_type) const;

				/**
				 * \brief Get the protocol type.
				 * \return The protocol type.
				 */
				uint16_t protocol_type() const;

				/**
				 * \brief Set the protocol type.
				 * \param protocol_type The protocol type.
				 */
				void set_protocol_type(uint16_t protocol_type) const;

				/**
				 * \brief Get the hardware address length.
				 * \return The hardware address length.
				 */
				size_t hardware_address_length() const;

				/**
				 * \brief Set the hardware address length.
				 * \param hardware_address_length The hardware address length.
				 */
				void set_hardware_address_length(size_t hardware_address_length) const;

				/**
				 * \brief Get the logical address length.
				 * \return The logical address length.
				 */
				size_t logical_address_length() const;

				/**
				 * \brief Set the logical address length.
				 * \param logical_address_length The logical address length.
				 */
				void set_logical_address_length(size_t logical_address_length) const;

				/**
				 * \brief Get the operation.
				 * \return The operation.
				 */
				uint16_t operation() const;

				/**
				 * \brief Set the operation.
				 * \param operation The operation.
				 */
				void operation(uint16_t operation) const;

				/**
				 * \brief Get the sender hardware address.
				 * \return The sender hardware address.
				 */
				boost::asio::mutable_buffer sender_hardware_address() const;

				/**
				 * \brief Get the sender logical address.
				 * \return The sender logical address.
				 */
				boost::asio::ip::address_v4 sender_logical_address() const;

				/**
				 * \brief Set the sender logical address.
				 * \param sender_logical_address The sender logical address.
				 */
				void set_sender_logical_address(boost::asio::ip::address_v4 sender_logical_address) const;

				/**
				 * \brief Get the target hardware address.
				 * \return The target hardware address.
				 */
				boost::asio::mutable_buffer target_hardware_address() const;

				/**
				 * \brief Get the target logical address.
				 * \return The target logical address.
				 */
				boost::asio::ip::address_v4 target_logical_address() const;

				/**
				 * \brief Set the target logical address.
				 * \param target_logical_address The target logical address.
				 */
				void set_target_logical_address(boost::asio::ip::address_v4 target_logical_address) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_mutable_helper_impl(arp_frame& frame);
		};
		
		inline uint16_t _const_helper_impl<arp_frame>::hardware_type() const
		{
			return ntohs(frame().hardware_type);
		}

		inline uint16_t _const_helper_impl<arp_frame>::protocol_type() const
		{
			return ntohs(frame().protocol_type);
		}

		inline size_t _const_helper_impl<arp_frame>::hardware_address_length() const
		{
			return frame().hardware_address_length;
		}

		inline size_t _const_helper_impl<arp_frame>::logical_address_length() const
		{
			return frame().logical_address_length;
		}

		inline uint16_t _const_helper_impl<arp_frame>::operation() const
		{
			return ntohs(frame().operation);
		}

		inline boost::asio::const_buffer _const_helper_impl<arp_frame>::sender_hardware_address() const
		{
			return boost::asio::buffer(frame().sender_hardware_address, sizeof(frame().sender_hardware_address));
		}

		inline boost::asio::ip::address_v4 _const_helper_impl<arp_frame>::sender_logical_address() const
		{
			return boost::asio::ip::address_v4(frame().sender_logical_address.s_addr);
		}

		inline boost::asio::const_buffer _const_helper_impl<arp_frame>::target_hardware_address() const
		{
			return boost::asio::buffer(frame().target_hardware_address, sizeof(frame().target_hardware_address));
		}

		inline boost::asio::ip::address_v4 _const_helper_impl<arp_frame>::target_logical_address() const
		{
			return boost::asio::ip::address_v4(frame().target_logical_address.s_addr);
		}

		inline _const_helper_impl<arp_frame>::_const_helper_impl(const arp_frame& _frame) :
			_base_const_helper<arp_frame>(_frame)
		{
		}

		inline uint16_t _mutable_helper_impl<arp_frame>::hardware_type() const
		{
			return ntohs(frame().hardware_type);
		}

		inline void _mutable_helper_impl<arp_frame>::set_hardware_type(uint16_t _hardware_type) const
		{
			frame().hardware_type = htons(_hardware_type);
		}

		inline uint16_t _mutable_helper_impl<arp_frame>::protocol_type() const
		{
			return ntohs(frame().protocol_type);
		}

		inline void _mutable_helper_impl<arp_frame>::set_protocol_type(uint16_t _protocol_type) const
		{
			frame().protocol_type = htons(_protocol_type);
		}

		inline size_t _mutable_helper_impl<arp_frame>::hardware_address_length() const
		{
			return frame().hardware_address_length;
		}

		inline void _mutable_helper_impl<arp_frame>::set_hardware_address_length(size_t _hardware_address_length) const
		{
			frame().hardware_address_length = static_cast<uint8_t>(_hardware_address_length);
		}

		inline size_t _mutable_helper_impl<arp_frame>::logical_address_length() const
		{
			return frame().logical_address_length;
		}

		inline void _mutable_helper_impl<arp_frame>::set_logical_address_length(size_t _logical_address_length) const
		{
			frame().logical_address_length = static_cast<uint8_t>(_logical_address_length);
		}

		inline uint16_t _mutable_helper_impl<arp_frame>::operation() const
		{
			return ntohs(frame().operation);
		}

		inline void _mutable_helper_impl<arp_frame>::operation(uint16_t _operation) const
		{
			frame().operation = htons(_operation);
		}

		inline boost::asio::mutable_buffer _mutable_helper_impl<arp_frame>::sender_hardware_address() const
		{
			return boost::asio::buffer(frame().sender_hardware_address, sizeof(frame().sender_hardware_address));
		}

		inline boost::asio::ip::address_v4 _mutable_helper_impl<arp_frame>::sender_logical_address() const
		{
			return boost::asio::ip::address_v4(frame().sender_logical_address.s_addr);
		}

		inline void _mutable_helper_impl<arp_frame>::set_sender_logical_address(boost::asio::ip::address_v4 _sender_logical_address) const
		{
			frame().sender_logical_address.s_addr = _sender_logical_address.to_ulong();
		}

		inline boost::asio::mutable_buffer _mutable_helper_impl<arp_frame>::target_hardware_address() const
		{
			return boost::asio::buffer(frame().target_hardware_address, sizeof(frame().target_hardware_address));
		}

		inline boost::asio::ip::address_v4 _mutable_helper_impl<arp_frame>::target_logical_address() const
		{
			return boost::asio::ip::address_v4(frame().target_logical_address.s_addr);
		}

		inline void _mutable_helper_impl<arp_frame>::set_target_logical_address(boost::asio::ip::address_v4 _target_logical_address) const
		{
			frame().target_logical_address.s_addr = _target_logical_address.to_ulong();
		}

		inline _mutable_helper_impl<arp_frame>::_mutable_helper_impl(arp_frame& _frame) :
			_base_mutable_helper<arp_frame>(_frame)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_ARP_HELPER_HPP */


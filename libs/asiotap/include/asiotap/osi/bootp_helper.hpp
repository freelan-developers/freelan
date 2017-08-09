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
 * \file bootp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A BOOTP helper class.
 */

#ifndef ASIOTAP_OSI_BOOTP_HELPER_HPP
#define ASIOTAP_OSI_BOOTP_HELPER_HPP

#include "helper.hpp"
#include "bootp_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base bootp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, bootp_frame> : public _base_helper<HelperTag, bootp_frame>
		{
			public:

				/**
				 * \brief Get the operation.
				 * \return The operation.
				 */
				uint8_t operation() const;

				/**
				 * \brief Get the hardware type.
				 * \return The hardware type.
				 */
				uint8_t hardware_type() const;

				/**
				 * \brief Get the hardware length.
				 * \return The hardware length.
				 */
				size_t hardware_length() const;

				/**
				 * \brief Get the hops count.
				 * \return The hops count.
				 */
				uint8_t hops() const;

				/**
				 * \brief Get the X identifier.
				 * \return The X identifier.
				 */
				uint32_t xid() const;

				/**
				 * \brief Get the seconds elapsed.
				 * \return The seconds elapsed.
				 */
				uint16_t seconds() const;

				/**
				 * \brief Get the flags.
				 * \return The flags.
				 */
				uint16_t flags() const;

				/**
				 * \brief Get the client address.
				 * \return The client address.
				 */
				boost::asio::ip::address_v4 ciaddr() const;

				/**
				 * \brief Get the requested address.
				 * \return The requested address.
				 */
				boost::asio::ip::address_v4 yiaddr() const;

				/**
				 * \brief Get the server address.
				 * \return The server address.
				 */
				boost::asio::ip::address_v4 siaddr() const;

				/**
				 * \brief Get the gateway address.
				 * \return The gateway address.
				 */
				boost::asio::ip::address_v4 giaddr() const;

				/**
				 * \brief Get the hardware address.
				 * \return The hardware address.
				 */
				typename _base_helper_impl::buffer_type chaddr() const
				{
					return boost::asio::buffer(this->frame().chaddr, sizeof(this->frame().chaddr));
				}

				/**
				 * \brief Get the server name.
				 * \return The server name.
				 */
				typename _base_helper_impl::buffer_type sname() const
				{
					return boost::asio::buffer(this->frame().sname, sizeof(this->frame().sname));
				}

				/**
				 * \brief Get the filename.
				 * \return The filename.
				 */
				typename _base_helper_impl::buffer_type file() const
				{
					return boost::asio::buffer(this->frame().file, sizeof(this->frame().file));
				}

				/**
				 * \brief Get the options.
				 * \return The options.
				 */
				typename _base_helper_impl::buffer_type options() const
				{
					return payload();
				}

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(typename _base_helper_impl<HelperTag, bootp_frame>::frame_type);
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf);
		};

		/**
		 * \brief The mutable udp helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, bootp_frame> : public _base_helper_impl<mutable_helper_tag, bootp_frame>
		{
			public:

				/**
				 * \brief Set the operation.
				 * \param operation The operation.
				 */
				void set_operation(uint8_t operation) const;

				/**
				 * \brief Set the hardware type.
				 * \param hardware_type The hardware type.
				 */
				void set_hardware_type(uint8_t hardware_type) const;

				/**
				 * \brief Set the hardware length.
				 * \param hardware_length The hardware length.
				 */
				void set_hardware_length(size_t hardware_length) const;

				/**
				 * \brief Set the hops count.
				 * \param hops The hops count.
				 */
				void set_hops(uint8_t hops) const;

				/**
				 * \brief Set the X identifier.
				 * \param xid The X identifier.
				 */
				void set_xid(uint32_t xid) const;

				/**
				 * \brief Set the seconds elapsed.
				 * \param seconds The seconds elapsed.
				 */
				void set_seconds(uint16_t seconds) const;

				/**
				 * \brief Set the flags.
				 * \param flags The flags.
				 */
				void set_flags(uint16_t flags) const;

				/**
				 * \brief Set the client address.
				 * \param ciaddr The client address.
				 */
				void set_ciaddr(boost::asio::ip::address_v4 ciaddr) const;

				/**
				 * \brief Set the requested address.
				 * \param yiaddr The requested address.
				 */
				void set_yiaddr(boost::asio::ip::address_v4 yiaddr) const;

				/**
				 * \brief Set the server address.
				 * \param siaddr The server address.
				 */
				void set_siaddr(boost::asio::ip::address_v4 siaddr) const;

				/**
				 * \brief Set the gateway address.
				 * \param giaddr The gateway address.
				 */
				void set_giaddr(boost::asio::ip::address_v4 giaddr) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint8_t _base_helper_impl<HelperTag, bootp_frame>::operation() const
		{
			return this->frame().operation;
		}

		template <class HelperTag>
		inline uint8_t _base_helper_impl<HelperTag, bootp_frame>::hardware_type() const
		{
			return this->frame().hardware_type;
		}

		template <class HelperTag>
		inline size_t _base_helper_impl<HelperTag, bootp_frame>::hardware_length() const
		{
			return this->frame().hardware_length;
		}

		template <class HelperTag>
		inline uint8_t _base_helper_impl<HelperTag, bootp_frame>::hops() const
		{
			return this->frame().hops;
		}

		template <class HelperTag>
		inline uint32_t _base_helper_impl<HelperTag, bootp_frame>::xid() const
		{
			return ntohl(this->frame().xid);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, bootp_frame>::seconds() const
		{
			return ntohs(this->frame().seconds);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, bootp_frame>::flags() const
		{
			return this->frame().flags;
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, bootp_frame>::ciaddr() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().ciaddr.s_addr));
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, bootp_frame>::yiaddr() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().yiaddr.s_addr));
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, bootp_frame>::siaddr() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().siaddr.s_addr));
		}

		template <class HelperTag>
		inline boost::asio::ip::address_v4 _base_helper_impl<HelperTag, bootp_frame>::giaddr() const
		{
			return boost::asio::ip::address_v4(ntohl(this->frame().giaddr.s_addr));
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, bootp_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, bootp_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_operation(uint8_t _operation) const
		{
			this->frame().operation = _operation;
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_hardware_type(uint8_t _hardware_type) const
		{
			this->frame().hardware_type = _hardware_type;
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_hardware_length(size_t _hardware_length) const
		{
			this->frame().hardware_length = static_cast<uint8_t>(_hardware_length);
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_hops(uint8_t _hops) const
		{
			this->frame().hops = _hops;
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_xid(uint32_t _xid) const
		{
			this->frame().xid = htonl(_xid);
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_seconds(uint16_t _seconds) const
		{
			this->frame().seconds = htons(_seconds);
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_flags(uint16_t _flags) const
		{
			this->frame().flags = _flags;
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_ciaddr(boost::asio::ip::address_v4 _ciaddr) const
		{
			this->frame().ciaddr.s_addr = htonl(_ciaddr.to_ulong());
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_yiaddr(boost::asio::ip::address_v4 _yiaddr) const
		{
			this->frame().yiaddr.s_addr = htonl(_yiaddr.to_ulong());
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_siaddr(boost::asio::ip::address_v4 _siaddr) const
		{
			this->frame().siaddr.s_addr = htonl(_siaddr.to_ulong());
		}

		inline void _helper_impl<mutable_helper_tag, bootp_frame>::set_giaddr(boost::asio::ip::address_v4 _giaddr) const
		{
			this->frame().giaddr.s_addr = htonl(_giaddr.to_ulong());
		}

		inline _helper_impl<mutable_helper_tag, bootp_frame>::_helper_impl(_helper_impl<mutable_helper_tag, bootp_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, bootp_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_BOOTP_HELPER_HPP */


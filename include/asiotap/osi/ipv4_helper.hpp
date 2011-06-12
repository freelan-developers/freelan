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
 * \file ipv4_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IPv4 helper class.
 */

#ifndef ASIOTAP_OSI_IPV4_HELPER_HPP
#define ASIOTAP_OSI_IPV4_HELPER_HPP

#include "helper.hpp"
#include "ipv4_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The const ipv4 helper class.
		 */
		typedef const_helper<ipv4_frame> const_ipv4_helper;

		/**
		 * \brief The mutable ipv4 helper class.
		 */
		typedef mutable_helper<ipv4_frame> mutable_ipv4_helper;

		/**
		 * \brief The const ipv4 helper implementation class.
		 */
		template <>
		class _const_helper_impl<ipv4_frame> : public _base_const_helper<ipv4_frame>
		{
			public:

				/**
				 * \brief Get the version.
				 * \return The version.
				 */
				uint8_t version() const;

				/**
				 * \brief Get the Internet Header Length, in words.
				 * \return The Internet Header Length, in words.
				 */
				uint8_t ihl() const;

				/**
				 * \brief Get the Internet Header Length, in bytes.
				 * \return The Internet Header Length, in bytes.
				 */
				size_t header_length() const;

				/**
				 * \brief Get the Type Of Service.
				 * \return The Type Of Service.
				 */
				uint8_t tos() const;

				/**
				 * \brief Get the total length.
				 * \return The total length.
				 */
				size_t total_length() const;

				/**
				 * \brief Get the payload length.
				 * \return The payload length.
				 */
				size_t payload_length() const;

				/**
				 * \brief Get the identification.
				 * \return The indentification.
				 */
				uint16_t identification() const;

				/**
				 * \brief Get the flags.
				 */
				uint8_t flags() const;

				/**
				 * \brief Get the position fragment.
				 * \return The position fragment.
				 */
				uint16_t position_fragment() const;

				/**
				 * \brief Get the time-to-live.
				 * \return The time-to-live.
				 */
				uint8_t ttl() const;

				/**
				 * \brief Get the protocol.
				 * \return The protocol.
				 */
				uint8_t protocol() const;

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const;

				/**
				 * \brief Get the source address.
				 * \return The source address.
				 */
				boost::asio::ip::address_v4 source() const;

				/**
				 * \brief Get the destination address.
				 * \return The destination address.
				 */
				boost::asio::ip::address_v4 destination() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_const_helper_impl(const ipv4_frame& frame);
		};

		/**
		 * \brief The mutable ipv4 helper implementation class.
		 */
		template <>
		class _mutable_helper_impl<ipv4_frame> : public _base_mutable_helper<ipv4_frame>
		{
			public:

				/**
				 * \brief Get the version.
				 * \return The version.
				 */
				uint8_t version() const;

				/**
				 * \brief Get the Internet Header Length, in words.
				 * \return The Internet Header Length, in words.
				 */
				uint8_t ihl() const;

				/**
				 * \brief Get the Internet Header Length, in bytes.
				 * \return The Internet Header Length, in bytes.
				 */
				size_t header_length() const;

				/**
				 * \brief Get the Type Of Service.
				 * \return The Type Of Service.
				 */
				uint8_t tos() const;

				/**
				 * \brief Get the total length.
				 * \return The total length.
				 */
				size_t total_length() const;

				/**
				 * \brief Get the payload length.
				 * \return The payload length.
				 */
				size_t payload_length() const;

				/**
				 * \brief Get the identification.
				 * \return The indentification.
				 */
				uint16_t identification() const;

				/**
				 * \brief Get the flags.
				 */
				uint8_t flags() const;

				/**
				 * \brief Get the position fragment.
				 * \return The position fragment.
				 */
				uint16_t position_fragment() const;

				/**
				 * \brief Get the time-to-live.
				 * \return The time-to-live.
				 */
				uint8_t ttl() const;

				/**
				 * \brief Get the protocol.
				 * \return The protocol.
				 */
				uint8_t protocol() const;

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const;

				/**
				 * \brief Get the source address.
				 * \return The source address.
				 */
				boost::asio::ip::address_v4 source() const;

				/**
				 * \brief Get the destination address.
				 * \return The destination address.
				 */
				boost::asio::ip::address_v4 destination() const;
			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param frame The frame to refer to.
				 */
				_mutable_helper_impl(ipv4_frame& frame);
		};

		inline uint8_t _const_helper_impl<ipv4_frame>::version() const
		{
			return (frame().version_ihl & 0xF0) >> 4;
		}

		inline uint8_t _const_helper_impl<ipv4_frame>::ihl() const
		{
			return (frame().version_ihl & 0x0F);
		}

		inline size_t _const_helper_impl<ipv4_frame>::header_length() const
		{
			return ihl() * sizeof(uint32_t);
		}

		inline uint8_t _const_helper_impl<ipv4_frame>::tos() const
		{
			return frame().service_type;
		}

		inline size_t _const_helper_impl<ipv4_frame>::total_length() const
		{
			return ntohs(frame().total_length);
		}

		inline size_t _const_helper_impl<ipv4_frame>::payload_length() const
		{
			return total_length() - header_length();
		}

		inline uint16_t _const_helper_impl<ipv4_frame>::identification() const
		{
			return ntohs(frame().identification);
		}

		inline uint8_t _const_helper_impl<ipv4_frame>::flags() const
		{
			return static_cast<uint8_t>((frame().flags_fragment & 0xE000) >> 13);
		}

		inline uint16_t _const_helper_impl<ipv4_frame>::position_fragment() const
		{
			return (frame().flags_fragment & 0x1FFF);
		}

		inline uint8_t _const_helper_impl<ipv4_frame>::ttl() const
		{
			return frame().ttl;
		}

		inline uint8_t _const_helper_impl<ipv4_frame>::protocol() const
		{
			return frame().protocol;
		}

		inline uint16_t _const_helper_impl<ipv4_frame>::checksum() const
		{
			return ntohs(frame().header_checksum);
		}

		inline boost::asio::ip::address_v4 _const_helper_impl<ipv4_frame>::source() const
		{
			return boost::asio::ip::address_v4(frame().source.s_addr);
		}

		inline boost::asio::ip::address_v4 _const_helper_impl<ipv4_frame>::destination() const
		{
			return boost::asio::ip::address_v4(frame().destination.s_addr);
		}

		inline _const_helper_impl<ipv4_frame>::_const_helper_impl(const ipv4_frame& _frame) :
			_base_const_helper<ipv4_frame>(_frame)
		{
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::version() const
		{
			return (frame().version_ihl & 0xF0) >> 4;
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::ihl() const
		{
			return (frame().version_ihl & 0x0F);
		}

		inline size_t _mutable_helper_impl<ipv4_frame>::header_length() const
		{
			return ihl() * sizeof(uint32_t);
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::tos() const
		{
			return frame().service_type;
		}

		inline size_t _mutable_helper_impl<ipv4_frame>::total_length() const
		{
			return ntohs(frame().total_length);
		}

		inline size_t _mutable_helper_impl<ipv4_frame>::payload_length() const
		{
			return total_length() - header_length();
		}

		inline uint16_t _mutable_helper_impl<ipv4_frame>::identification() const
		{
			return ntohs(frame().identification);
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::flags() const
		{
			return static_cast<uint8_t>((frame().flags_fragment & 0xE000) >> 13);
		}

		inline uint16_t _mutable_helper_impl<ipv4_frame>::position_fragment() const
		{
			return (frame().flags_fragment & 0x1FFF);
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::ttl() const
		{
			return frame().ttl;
		}

		inline uint8_t _mutable_helper_impl<ipv4_frame>::protocol() const
		{
			return frame().protocol;
		}

		inline uint16_t _mutable_helper_impl<ipv4_frame>::checksum() const
		{
			return ntohs(frame().header_checksum);
		}

		inline boost::asio::ip::address_v4 _mutable_helper_impl<ipv4_frame>::source() const
		{
			return boost::asio::ip::address_v4(frame().source.s_addr);
		}

		inline boost::asio::ip::address_v4 _mutable_helper_impl<ipv4_frame>::destination() const
		{
			return boost::asio::ip::address_v4(frame().destination.s_addr);
		}

		inline _mutable_helper_impl<ipv4_frame>::_mutable_helper_impl(ipv4_frame& _frame) :
			_base_mutable_helper<ipv4_frame>(_frame)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_IPV4_HELPER_HPP */


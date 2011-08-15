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
 * \file icmp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ICMP helper class.
 */

#ifndef ASIOTAP_OSI_ICMP_HELPER_HPP
#define ASIOTAP_OSI_ICMP_HELPER_HPP

#include "helper.hpp"
#include "icmp_frame.hpp"

#include "ipv4_helper.hpp"
#include "ipv6_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The const icmp helper implementation class.
		 */
		template <>
		class _const_helper_impl<icmp_frame> : public _base_const_helper<icmp_frame>
		{
			public:

				/**
				 * \brief Get the message type.
				 * \return The message type.
				 */
				uint8_t type() const;

				/**
				 * \brief Get the error code.
				 * \return The error code.
				 */
				uint8_t code() const;

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const;

				/**
				 * \brief Get the data.
				 * \return The data.
				 */
				uint32_t data() const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				boost::asio::const_buffer payload() const;

				/**
				 * \brief Compute the checksum.
				 * \return The checksum.
				 */
				uint16_t compute_checksum() const;

				/**
				 * \brief Verify the checksum.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_const_helper_impl(boost::asio::const_buffer buf);
		};

		/**
		 * \brief The mutable icmp helper implementation class.
		 */
		template <>
		class _mutable_helper_impl<icmp_frame> : public _base_mutable_helper<icmp_frame>
		{
			public:

				/**
				 * \brief Get the message type.
				 * \return The message type.
				 */
				uint8_t type() const;

				/**
				 * \brief Set the message type.
				 * \param type The message type.
				 */
				void set_type(uint8_t type) const;

				/**
				 * \brief Get the error code.
				 * \return The error code.
				 */
				uint8_t code() const;

				/**
				 * \brief Set the error code.
				 * \param code The error code.
				 */
				void set_code(uint8_t code) const;

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const;

				/**
				 * \brief Set the checksum.
				 * \param checksum The checksum.
				 */
				void set_checksum(uint16_t checksum) const;

				/**
				 * \brief Get the data.
				 * \return The data.
				 */
				uint32_t data() const;

				/**
				 * \brief Set the data.
				 * \param data The data.
				 */
				void set_data(uint32_t data) const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				boost::asio::mutable_buffer payload() const;

				/**
				 * \brief Compute the checksum.
				 * \return The checksum.
				 */
				uint16_t compute_checksum() const;

				/**
				 * \brief Verify the checksum.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_mutable_helper_impl(boost::asio::mutable_buffer buf);
		};

		inline uint8_t _const_helper_impl<icmp_frame>::type() const
		{
			return frame().type;
		}

		inline uint8_t _const_helper_impl<icmp_frame>::code() const
		{
			return frame().code;
		}

		inline uint16_t _const_helper_impl<icmp_frame>::checksum() const
		{
			return ntohs(frame().checksum);
		}

		inline uint32_t _const_helper_impl<icmp_frame>::data() const
		{
			return ntohl(frame().data);
		}

		inline boost::asio::const_buffer _const_helper_impl<icmp_frame>::payload() const
		{
			return buffer() + sizeof(frame_type);
		}

		inline bool _const_helper_impl<icmp_frame>::verify_checksum() const
		{
			return compute_checksum() == 0x0000;
		}

		inline _const_helper_impl<icmp_frame>::_const_helper_impl(boost::asio::const_buffer buf) :
			_base_const_helper<icmp_frame>(buf)
		{
		}

		inline uint8_t _mutable_helper_impl<icmp_frame>::type() const
		{
			return frame().type;
		}

		inline void _mutable_helper_impl<icmp_frame>::set_type(uint8_t _type) const
		{
			frame().type = _type;
		}

		inline uint8_t _mutable_helper_impl<icmp_frame>::code() const
		{
			return frame().code;
		}

		inline void _mutable_helper_impl<icmp_frame>::set_code(uint8_t _code) const
		{
			frame().code = _code;
		}

		inline uint16_t _mutable_helper_impl<icmp_frame>::checksum() const
		{
			return ntohs(frame().checksum);
		}

		inline void _mutable_helper_impl<icmp_frame>::set_checksum(uint16_t _checksum) const
		{
			frame().checksum = htons(_checksum);
		}

		inline uint32_t _mutable_helper_impl<icmp_frame>::data() const
		{
			return ntohl(frame().data);
		}

		inline void _mutable_helper_impl<icmp_frame>::set_data(uint32_t _data) const
		{
			frame().data = htonl(_data);
		}

		inline boost::asio::mutable_buffer _mutable_helper_impl<icmp_frame>::payload() const
		{
			return buffer() + sizeof(frame_type);
		}

		inline bool _mutable_helper_impl<icmp_frame>::verify_checksum() const
		{
			return compute_checksum() == 0x0000;
		}

		inline _mutable_helper_impl<icmp_frame>::_mutable_helper_impl(boost::asio::mutable_buffer buf) :
			_base_mutable_helper<icmp_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_ICMP_HELPER_HPP */


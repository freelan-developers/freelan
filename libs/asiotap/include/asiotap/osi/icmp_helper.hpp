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
		 * \brief The base icmp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, icmp_frame> : public _base_helper<HelperTag, icmp_frame>
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
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(typename _base_helper_impl<HelperTag, icmp_frame>::frame_type);
				}

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
				_base_helper_impl(typename _base_helper_impl::buffer_type buf);
		};

		/**
		 * \brief The mutable icmp helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, icmp_frame> : public _base_helper_impl<mutable_helper_tag, icmp_frame>
		{
			public:

				/**
				 * \brief Set the message type.
				 * \param type The message type.
				 */
				void set_type(uint8_t type) const;

				/**
				 * \brief Set the error code.
				 * \param code The error code.
				 */
				void set_code(uint8_t code) const;

				/**
				 * \brief Set the checksum.
				 * \param checksum The checksum.
				 */
				void set_checksum(uint16_t checksum) const;

				/**
				 * \brief Set the data.
				 * \param data The data.
				 */
				void set_data(uint32_t data) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint8_t _base_helper_impl<HelperTag, icmp_frame>::type() const
		{
			return this->frame().type;
		}

		template <class HelperTag>
		inline uint8_t _base_helper_impl<HelperTag, icmp_frame>::code() const
		{
			return this->frame().code;
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, icmp_frame>::checksum() const
		{
			return this->frame().checksum;
		}

		template <class HelperTag>
		inline uint32_t _base_helper_impl<HelperTag, icmp_frame>::data() const
		{
			return ntohl(this->frame().data);
		}

		template <class HelperTag>
		inline bool _base_helper_impl<HelperTag, icmp_frame>::verify_checksum() const
		{
			return this->compute_checksum() == 0x0000;
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, icmp_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, icmp_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, icmp_frame>::set_type(uint8_t _type) const
		{
			this->frame().type = _type;
		}

		inline void _helper_impl<mutable_helper_tag, icmp_frame>::set_code(uint8_t _code) const
		{
			this->frame().code = _code;
		}

		inline void _helper_impl<mutable_helper_tag, icmp_frame>::set_checksum(uint16_t _checksum) const
		{
			this->frame().checksum = _checksum;
		}

		inline void _helper_impl<mutable_helper_tag, icmp_frame>::set_data(uint32_t _data) const
		{
			this->frame().data = htonl(_data);
		}

		inline _helper_impl<mutable_helper_tag, icmp_frame>::_helper_impl(_helper_impl<mutable_helper_tag, icmp_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, icmp_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_ICMP_HELPER_HPP */


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
 * \file udp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An UDP helper class.
 */

#ifndef ASIOTAP_OSI_UDP_HELPER_HPP
#define ASIOTAP_OSI_UDP_HELPER_HPP

#include "helper.hpp"
#include "udp_frame.hpp"

#include "ipv4_helper.hpp"
#include "ipv6_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base udp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, udp_frame> : public _base_helper<HelperTag, udp_frame>
		{
			public:

				/**
				 * \brief Get the source port.
				 * \return The source port.
				 */
				uint16_t source() const;

				/**
				 * \brief Get the destination port.
				 * \return The destination port.
				 */
				uint16_t destination() const;

				/**
				 * \brief Get the length.
				 * \return The length.
				 */
				uint16_t length() const;

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(typename _base_helper_impl<HelperTag, udp_frame>::frame_type);
				}

				/**
				 * \brief Compute the checksum.
				 * \param parent_frame The parent frame.
				 * \return The checksum.
				 */
				uint16_t compute_checksum(const_helper<ipv4_frame> parent_frame) const;

				/**
				 * \brief Compute the checksum.
				 * \param parent_frame The parent frame.
				 * \return The checksum.
				 */
				uint16_t compute_checksum(const_helper<ipv6_frame> parent_frame) const;

				/**
				 * \brief Verify the checksum.
				 * \param parent_frame The parent frame.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum(const_helper<ipv4_frame> parent_frame) const;

				/**
				 * \brief Verify the checksum.
				 * \param parent_frame The parent frame.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum(const_helper<ipv6_frame> parent_frame) const;

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
		class _helper_impl<mutable_helper_tag, udp_frame> : public _base_helper_impl<mutable_helper_tag, udp_frame>
		{
			public:

				/**
				 * \brief Set the source port.
				 * \param source The source port.
				 */
				void set_source(uint16_t source) const;

				/**
				 * \brief Set the destination port.
				 * \param destination The destination port.
				 */
				void set_destination(uint16_t destination) const;

				/**
				 * \brief Set the length.
				 * \param length The length.
				 */
				void set_length(uint16_t length) const;

				/**
				 * \brief Set the checksum.
				 * \param checksum The checksum.
				 */
				void set_checksum(uint16_t checksum) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, udp_frame>::source() const
		{
			return ntohs(this->frame().source);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, udp_frame>::destination() const
		{
			return ntohs(this->frame().destination);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, udp_frame>::length() const
		{
			return ntohs(this->frame().length);
		}

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, udp_frame>::checksum() const
		{
			return this->frame().checksum;
		}

		template <class HelperTag>
		inline bool _base_helper_impl<HelperTag, udp_frame>::verify_checksum(const_helper<ipv4_frame> parent_frame) const
		{
			return this->compute_checksum(parent_frame) == 0x0000;
		}

		template <class HelperTag>
		inline bool _base_helper_impl<HelperTag, udp_frame>::verify_checksum(const_helper<ipv6_frame> parent_frame) const
		{
			return this->compute_checksum(parent_frame) == 0x0000;
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, udp_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, udp_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, udp_frame>::set_source(uint16_t _source) const
		{
			this->frame().source = htons(_source);
		}

		inline void _helper_impl<mutable_helper_tag, udp_frame>::set_destination(uint16_t _destination) const
		{
			this->frame().destination = htons(_destination);
		}

		inline void _helper_impl<mutable_helper_tag, udp_frame>::set_length(uint16_t _length) const
		{
			this->frame().length = htons(_length);
		}

		inline void _helper_impl<mutable_helper_tag, udp_frame>::set_checksum(uint16_t _checksum) const
		{
			this->frame().checksum = _checksum;
		}

		inline _helper_impl<mutable_helper_tag, udp_frame>::_helper_impl(_helper_impl<mutable_helper_tag, udp_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, udp_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_UDP_HELPER_HPP */


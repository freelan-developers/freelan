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
 * \file ethernet_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Ethernet helper class.
 */

#ifndef ASIOTAP_OSI_ETHERNET_HELPER_HPP
#define ASIOTAP_OSI_ETHERNET_HELPER_HPP

#include "helper.hpp"
#include "ethernet_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base Ethernet helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, ethernet_frame> : public _base_helper<HelperTag, ethernet_frame>
		{
			public:

				/**
				 * \brief Get the target.
				 * \return The target.
				 */
				typename _base_helper_impl::buffer_type target() const
				{
					return boost::asio::buffer(this->frame().target, sizeof(this->frame().target));
				}

				/**
				 * \brief Get the sender.
				 * \return The sender.
				 */
				typename _base_helper_impl::buffer_type sender() const
				{
					return boost::asio::buffer(this->frame().sender, sizeof(this->frame().sender));
				}

				/**
				 * \brief Get the protocol.
				 * \return The protocol.
				 */
				uint16_t protocol() const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(ethernet_frame);
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf);
		};

		/**
		 * \brief The mutable Ethernet helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, ethernet_frame> : public _base_helper_impl<mutable_helper_tag, ethernet_frame>
		{
			public:

				/**
				 * \brief Set the protocol.
				 * \param protocol The protocol.
				 */
				void set_protocol(uint16_t protocol) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint16_t _base_helper_impl<HelperTag, ethernet_frame>::protocol() const
		{
			return ntohs(this->frame().protocol);
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, ethernet_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, ethernet_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, ethernet_frame>::set_protocol(uint16_t _protocol) const
		{
			this->frame().protocol = htons(_protocol);
		}

		inline _helper_impl<mutable_helper_tag, ethernet_frame>::_helper_impl(_helper_impl<mutable_helper_tag, ethernet_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, ethernet_frame>(buf)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_ETHERNET_HELPER_HPP */


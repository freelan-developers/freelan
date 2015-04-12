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
 * \file tcp_helper.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A TCP helper class.
 */

#include "osi/tcp_helper.hpp"

#include "osi/checksum_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		namespace
		{
			template <typename HelperType>
			struct tcp_pseudo_header;

			template <>
			struct tcp_pseudo_header<const_helper<ipv4_frame> >
			{
				typedef tcp_ipv4_pseudo_header type;
			};

			template <>
			struct tcp_pseudo_header<const_helper<ipv6_frame> >
			{
				typedef tcp_ipv6_pseudo_header type;
			};

			tcp_ipv4_pseudo_header parent_frame_to_pseudo_header(const_helper<ipv4_frame> parent_frame)
			{
				tcp_ipv4_pseudo_header pseudo_header {};

				pseudo_header.ipv4_source = parent_frame.frame().source;
				pseudo_header.ipv4_destination = parent_frame.frame().destination;
				pseudo_header.ipv4_protocol = parent_frame.frame().protocol;
				pseudo_header.tcp_length = htons(parent_frame.payload_length());

				return pseudo_header;
			}

			tcp_ipv6_pseudo_header parent_frame_to_pseudo_header(const_helper<ipv6_frame> parent_frame)
			{
				tcp_ipv6_pseudo_header pseudo_header {};

				pseudo_header.ipv6_source = parent_frame.frame().source;
				pseudo_header.ipv6_destination = parent_frame.frame().destination;
				pseudo_header.upper_layer_length = htonl(parent_frame.payload_length());
				pseudo_header.ipv6_next_header = TCP_PROTOCOL; // Must be this value, not the parent frame next-header as it could be different.

				return pseudo_header;
			}

			template <typename HelperType, typename ParentHelperType>
			uint16_t compute_tcp_checksum(ParentHelperType parent_frame, HelperType tcp_frame)
			{
				const uint16_t* buf = boost::asio::buffer_cast<const uint16_t*>(tcp_frame.buffer());
				size_t buf_len = boost::asio::buffer_size(tcp_frame.buffer());
				const auto pseudo_header = parent_frame_to_pseudo_header(parent_frame);

				checksum_helper chk;
				chk.update(reinterpret_cast<const uint16_t*>(&pseudo_header), sizeof(pseudo_header));
				chk.update(buf, buf_len);

				return chk.compute();
			}
		}

		template <class HelperTag>
		uint16_t _base_helper_impl<HelperTag, tcp_frame>::compute_checksum(const_helper<ipv4_frame> parent_frame) const
		{
			return compute_tcp_checksum(parent_frame, *this);
		}

		template <class HelperTag>
		uint16_t _base_helper_impl<HelperTag, tcp_frame>::compute_checksum(const_helper<ipv6_frame> parent_frame) const
		{
			return compute_tcp_checksum(parent_frame, *this);
		}

		template uint16_t _base_helper_impl<const_helper_tag, tcp_frame>::compute_checksum(const_helper<ipv4_frame>) const;
		template uint16_t _base_helper_impl<const_helper_tag, tcp_frame>::compute_checksum(const_helper<ipv6_frame>) const;
		template uint16_t _base_helper_impl<mutable_helper_tag, tcp_frame>::compute_checksum(const_helper<ipv4_frame>) const;
		template uint16_t _base_helper_impl<mutable_helper_tag, tcp_frame>::compute_checksum(const_helper<ipv6_frame>) const;
	}
}

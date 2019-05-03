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
 * \file icmpv6_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ICMPv6 frame structure.
 */

#pragma once

#include "frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The ICMPv6 header.
		 */
		const uint8_t ICMPV6_HEADER = 0x3A;

		/**
		 * \brief The neighbor solicitation type.
		 */
		const uint8_t ICMPV6_NEIGHBOR_SOLICITATION = 0x87;

		/**
		 * \brief The neighbor advertisement type.
		 */
		const uint8_t ICMPV6_NEIGHBOR_ADVERTISEMENT = 0x88;

		/**
		* \brief The source link-layer address option.
		*/
		const uint8_t ICMPV6_OPTION_SOURCE_LINK_LAYER_ADDRESS = 0x01;

		/**
		* \brief The target link-layer address option.
		*/
		const uint8_t ICMPV6_OPTION_TARGET_LINK_LAYER_ADDRESS = 0x02;

#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief An ICMPv6 frame structure.
		 */
		struct icmpv6_frame
		{
			uint8_t type; /**< Type */
			uint8_t code; /**< Code */
			uint16_t checksum; /**< Checksum */
			uint32_t flags; /**< Flags */
			struct in6_addr target; /**< Target */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif

		/**
		 * \brief An ICMPv6-IPv6 pseudo-header structure.
		 */
		struct icmpv6_ipv6_pseudo_header
		{
			struct in6_addr ipv6_source; /**< Source IPv6 address */
			struct in6_addr ipv6_destination; /**< Source IPv6 address */
			uint32_t upper_layer_length; /**< The upper-layer length */
			uint16_t zero; /**< 16 bits reserved field (must be zero) */
			uint8_t zero2; /**< 8 bits reserved field (must be zero) */
			uint8_t ipv6_next_header; /**< The IPv6 next header */
		} ;
	}
}

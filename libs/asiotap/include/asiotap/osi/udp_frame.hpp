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
 * \file udp_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An UDP frame structure.
 */

#ifndef ASIOTAP_OSI_UDP_FRAME_HPP
#define ASIOTAP_OSI_UDP_FRAME_HPP

#include "frame.hpp"

namespace asiotap
{
	namespace osi
	{
#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief The UDP protocol.
		 */
		const uint8_t UDP_PROTOCOL = 0x11;

		/**
		 * \brief An UDP frame structure.
		 */
		struct udp_frame
		{
			uint16_t source; /**< Source port */
			uint16_t destination; /**< Destination port */
			uint16_t length; /**< The header and data length */
			uint16_t checksum; /**< The checksum */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif

		/**
		 * \brief An UDP-IPv4 pseudo-header structure.
		 */
		struct udp_ipv4_pseudo_header
		{
			struct in_addr ipv4_source; /**< Source IPv4 address */
			struct in_addr ipv4_destination; /**< Source IPv4 address */
			uint8_t reserved; /**< 8 bits reserved field (must be zero) */
			uint8_t ipv4_protocol; /**< The IPv4 protocol */
			uint16_t udp_length; /**< The UDP length */
		};

		/**
		 * \brief An UDP-IPv6 pseudo-header structure.
		 */
		struct udp_ipv6_pseudo_header
		{
			struct in6_addr ipv6_source; /**< Source IPv6 address */
			struct in6_addr ipv6_destination; /**< Source IPv6 address */
			uint16_t udp_length; /**< The UDP length */
			uint16_t reserved; /**< 8 bits reserved field (must be zero) */
			uint8_t reserved2; /**< 8 bits reserved field (must be zero) */
			uint8_t ipv6_next_header; /**< The IPv6 next header */
		};
	}
}

#endif /* ASIOTAP_OSI_UDP_FRAME_HPP */


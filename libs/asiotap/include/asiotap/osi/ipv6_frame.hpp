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
 * \file ipv6_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IPv6 frame structure.
 */

#ifndef ASIOTAP_OSI_IPV6_FRAME_HPP
#define ASIOTAP_OSI_IPV6_FRAME_HPP

#include "frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The IP protocol.
		 */
		const uint16_t IPV6_PROTOCOL = 0x86DD;

		/**
		 * \brief The IP protocol version 6.
		 */
		const uint8_t IP_PROTOCOL_VERSION_6 = 0x06;

#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief An IPv6 frame structure.
		 */
		struct ipv6_frame
		{
			uint32_t version_class_label; /**< Version, class and flow label */
			uint16_t payload_length; /**< Payload length */
			uint8_t next_header; /**< Next header number */
			uint8_t hop_limit; /**< Hop limit value */
			struct in6_addr source; /**< Source address */
			struct in6_addr destination; /**< Destination address */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif
	}
}

#endif /* ASIOTAP_OSI_IPV6_FRAME_HPP */


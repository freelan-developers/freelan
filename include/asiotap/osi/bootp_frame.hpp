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
 * \file bootp_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A BOOTP frame structure.
 */

#ifndef ASIOTAP_OSI_BOOTP_FRAME_HPP
#define ASIOTAP_OSI_BOOTP_FRAME_HPP

#include "frame.hpp"

namespace asiotap
{
	namespace osi
	{
#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief The BOOTP protocol.
		 */
		const uint16_t BOOTP_PROTOCOL = 67;

		/**
		 * \brief The BOOTP BOOTREQUEST operation code.
		 */
		const uint8_t BOOTP_BOOTREQUEST = 0x01;

		/**
		 * \brief The BOOTP BOOTREPLY operation code.
		 */
		const uint8_t BOOTP_BOOTREPLY = 0x02;

		/**
		 * \brief The BOOTP ethernet hardware type.
		 */
		const uint8_t BOOTP_HARDWARE_TYPE_ETHERNET = 0x01;

		/**
		 * \brief A BOOTP frame structure.
		 */
		struct bootp_frame
		{
			uint8_t operation; /**< The operation. */
			uint8_t hardware_type; /**< The hardware type. */
			uint8_t hardware_length; /**< The hardware length. */
			uint8_t hops; /**< The hops. */
			uint32_t xid; /**< The X identifier. */
			uint16_t seconds; /**< The time elapsed since the request, in seconds. */
			uint16_t flags; /**< The flags. */
			in_addr ciaddr; /**< The client address. */
			in_addr yiaddr; /**< The future client address. */
			in_addr siaddr; /**< The next server address. */
			in_addr giaddr; /**< The gateway address. */
			uint8_t chaddr[16]; /**< The hardware address. */
			uint8_t sname[64]; /**< The server name. */
			uint8_t file[128]; /**< The boot filename. */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif
	}
}

#endif /* ASIOTAP_OSI_BOOTP_FRAME_HPP */


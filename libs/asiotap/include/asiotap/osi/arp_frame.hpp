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
 * \file arp_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ARP frame structure.
 */

#ifndef ASIOTAP_OSI_ARP_FRAME_HPP
#define ASIOTAP_OSI_ARP_FRAME_HPP

#include "frame.hpp"
#include "ethernet_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The ARP protocol number.
		 */
		const uint16_t ARP_PROTOCOL = 0x0806;

		/**
		 * \brief The Ethernet hardware type.
		 */
		const uint16_t ETHERNET_HARDWARE_TYPE = 0x0001;

		/**
		 * \brief The IP protocol type.
		 */
		const uint16_t IP_PROTOCOL_TYPE = 0x0800;

		/**
		 * \brief An ARP request operation.
		 */
		const uint16_t ARP_REQUEST_OPERATION = 0x0001;

		/**
		 * \brief An ARP reply operation.
		 */
		const uint16_t ARP_REPLY_OPERATION = 0x0002;

#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief An arp frame structure.
		 */
		struct arp_frame
		{
			uint16_t hardware_type; /**< Link-layer protocol. */
			uint16_t protocol_type; /**< Layer 3 protocol. */
			uint8_t hardware_address_length; /**< Length of link-layer address (6 in case of ethernet). */
			uint8_t logical_address_length; /**< Length of layer 3 address (4 in case of IPv4). */
			uint16_t operation; /**< Operation of the sender (1 for requesting a resolution, 2 for replying to a request). */
			uint8_t sender_hardware_address[ETHERNET_ADDRESS_SIZE]; /**< Link-layer address of sender. */
			struct in_addr sender_logical_address; /**< Layer 3 address (i.e. IPv4) of sender. */
			uint8_t target_hardware_address[ETHERNET_ADDRESS_SIZE]; /**< Link-layer address of target. */
			struct in_addr target_logical_address; /**< Layer 3 address (i.e. IPv4) of target. */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif
	}
}

#endif /* ASIOTAP_OSI_ARP_FRAME_HPP */


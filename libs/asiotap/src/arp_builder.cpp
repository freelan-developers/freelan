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
 * \file arp_builder.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ARP frame builder class.
 */

#include "osi/arp_builder.hpp"

#include "osi/arp_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		size_t builder<arp_frame>::write(
		    uint16_t operation,
		    boost::asio::const_buffer sender_hardware_address,
		    boost::asio::ip::address_v4 sender_logical_address,
		    boost::asio::const_buffer target_hardware_address,
		    boost::asio::ip::address_v4 target_logical_address
		) const
		{
			assert(boost::asio::buffer_size(sender_hardware_address) == ETHERNET_ADDRESS_SIZE);
			assert(boost::asio::buffer_size(target_hardware_address) == ETHERNET_ADDRESS_SIZE);

			helper_type helper = get_helper();

			helper.set_hardware_type(ETHERNET_HARDWARE_TYPE);
			helper.set_protocol_type(IP_PROTOCOL_TYPE);
			helper.set_hardware_address_length(ETHERNET_ADDRESS_SIZE);
			helper.set_logical_address_length(sizeof(in_addr));
			helper.set_operation(operation);
			memcpy(boost::asio::buffer_cast<uint8_t*>(helper.sender_hardware_address()), boost::asio::buffer_cast<const uint8_t*>(sender_hardware_address), ETHERNET_ADDRESS_SIZE);
			helper.set_sender_logical_address(sender_logical_address);
			memcpy(boost::asio::buffer_cast<uint8_t*>(helper.target_hardware_address()), boost::asio::buffer_cast<const uint8_t*>(target_hardware_address), ETHERNET_ADDRESS_SIZE);
			helper.set_target_logical_address(target_logical_address);

			return sizeof(frame_type) + boost::asio::buffer_size(payload());
		}
	}
}

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
 * \file icmpv6_proxy.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ICMPv6 proxy class.
 */

#include "osi/icmpv6_proxy.hpp"

#include "osi/ipv6_helper.hpp"
#include "osi/icmpv6_helper.hpp"

#include "osi/ipv6_builder.hpp"
#include "osi/icmpv6_builder.hpp"

namespace asiotap
{
	namespace osi
	{
		boost::optional<boost::asio::const_buffer> proxy<icmpv6_frame>::process_frame(const_helper<ipv6_frame> ipv6_helper, const_helper<icmpv6_frame> icmpv6_helper, boost::asio::mutable_buffer response_buffer) const
		{
			if (icmpv6_helper.type() == ICMPV6_NEIGHBOR_SOLICITATION)
			{
				ethernet_address_type eth_addr;

				bool should_answer = false;

				if (m_neighbor_solicitation_callback)
				{
					should_answer = m_neighbor_solicitation_callback(icmpv6_helper.target(), eth_addr);
				}

				if (should_answer)
				{
					// We hardcode the structure for the ICMPv6 option because it just works.
					size_t payload_size = 8;
					uint8_t* const target_link_layer_address_option_buffer = boost::asio::buffer_cast<uint8_t*>(response_buffer + (boost::asio::buffer_size(response_buffer) - payload_size));

					target_link_layer_address_option_buffer[0] = ICMPV6_OPTION_TARGET_LINK_LAYER_ADDRESS; // The option type.
					target_link_layer_address_option_buffer[1] = 0x01; // The size, in multiples of 8 bytes.
					::memcpy(&target_link_layer_address_option_buffer[2], &eth_addr.data()[0], eth_addr.data().size()); // The ethernet address.

					builder<icmpv6_frame> icmpv6_builder(response_buffer, payload_size);

					payload_size = icmpv6_builder.write(
					    ICMPV6_NEIGHBOR_ADVERTISEMENT,
						0,
						false,
						true,
						true,
						icmpv6_helper.target()
					);

					builder<ipv6_frame> ipv6_builder(response_buffer, payload_size);

					payload_size = ipv6_builder.write(
						ipv6_helper._class(),
						ipv6_helper.label(),
						ICMPV6_HEADER,
						0xFF,
						icmpv6_helper.target(),
						ipv6_helper.source()
					);

					icmpv6_builder.update_checksum(ipv6_builder.get_helper());

					return boost::make_optional<boost::asio::const_buffer>(response_buffer + (boost::asio::buffer_size(response_buffer) - payload_size));
				}
			}

			return boost::optional<boost::asio::const_buffer>();
		}
	}
}

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
 * \file arp_proxy.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ARP proxy class.
 */

#include "osi/arp_proxy.hpp"

#include "osi/arp_helper.hpp"
#include "osi/ethernet_builder.hpp"
#include "osi/arp_builder.hpp"

namespace asiotap
{
	namespace osi
	{
		void arp_proxy::on_arp_request(const_helper<ethernet_frame> ethernet_helper, const_helper<arp_frame> arp_helper)
		{
			(void) ethernet_helper;

			entry_map_type::const_iterator entry_it = m_entry_map.find(arp_helper.target_logical_address());

			if (entry_it != m_entry_map.end())
			{
				size_t payload_size;

				builder<arp_frame> arp_builder(m_buffer);
			
				payload_size = arp_builder.write(
						ARP_REPLY_OPERATION,
						arp_helper.sender_hardware_address(),
						arp_helper.sender_logical_address(),
						boost::asio::buffer(entry_it->second),
						entry_it->first
						);

				builder<ethernet_frame> ethernet_builder(m_buffer, payload_size);

				payload_size = ethernet_builder.write(
						ethernet_helper.sender(),
						ethernet_helper.target(),
						ethernet_helper.protocol()
						);

				m_on_reply(payload_size);
			}
		}
	}
}

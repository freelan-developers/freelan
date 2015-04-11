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
 * \file ipv6_builder.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IPv6 frame builder class.
 */

#include "osi/ipv6_builder.hpp"

#include "osi/ipv6_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		size_t builder<ipv6_frame>::write(
			uint8_t _class,
			uint32_t label,
			uint8_t next_header,
			uint8_t hop_limit,
			boost::asio::ip::address_v6 source,
			boost::asio::ip::address_v6 destination
		) const
		{
			helper_type helper = get_helper();

			helper.set_version(IP_PROTOCOL_VERSION_6);
			helper.set_class(_class);
			helper.set_label(label);
			helper.set_payload_length(boost::asio::buffer_size(payload()));
			helper.set_next_header(next_header);
			helper.set_hop_limit(hop_limit);
			helper.set_source(source);
			helper.set_destination(destination);

			return helper.header_length() + helper.payload_length();
		}
	}
}

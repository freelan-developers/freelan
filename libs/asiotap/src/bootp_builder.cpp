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
 * \file bootp_builder.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An bootp frame builder class.
 */

#include "osi/bootp_builder.hpp"

#include "osi/bootp_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		namespace
		{
			void boost_asio_buffer_memcpy(boost::asio::mutable_buffer destination, boost::asio::const_buffer source)
			{
				const size_t destination_size = boost::asio::buffer_size(destination);
				uint8_t* const destination_data = boost::asio::buffer_cast<uint8_t*>(destination);
				const size_t source_size = boost::asio::buffer_size(source);
				const uint8_t* const source_data = boost::asio::buffer_cast<const uint8_t*>(source);

				const size_t copy_cnt = source_size > destination_size ? destination_size : source_size;
				const size_t left_cnt = destination_size - copy_cnt;

				memcpy(destination_data, source_data, copy_cnt);
				memset(destination_data + copy_cnt, 0x00, left_cnt);
			}
		}

		size_t builder<bootp_frame>::write(
		    uint8_t operation,
		    uint8_t hardware_type,
		    size_t hardware_length,
		    uint8_t hops,
		    uint32_t xid,
		    uint16_t seconds,
		    uint16_t flags,
		    boost::asio::ip::address_v4 ciaddr,
		    boost::asio::ip::address_v4 yiaddr,
		    boost::asio::ip::address_v4 siaddr,
		    boost::asio::ip::address_v4 giaddr,
		    boost::asio::const_buffer chaddr,
		    boost::asio::const_buffer sname,
		    boost::asio::const_buffer file
		) const
		{
			helper_type helper = get_helper();

			helper.set_operation(operation);
			helper.set_hardware_type(hardware_type);
			helper.set_hardware_length(hardware_length);
			helper.set_hops(hops);
			helper.set_xid(xid);
			helper.set_seconds(seconds);
			helper.set_flags(flags);
			helper.set_ciaddr(ciaddr);
			helper.set_yiaddr(yiaddr);
			helper.set_siaddr(siaddr);
			helper.set_giaddr(giaddr);
			boost_asio_buffer_memcpy(helper.chaddr(), chaddr);
			boost_asio_buffer_memcpy(helper.sname(), sname);
			boost_asio_buffer_memcpy(helper.file(), file);

			return sizeof(frame_type) + boost::asio::buffer_size(payload());
		}
	}
}

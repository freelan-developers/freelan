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
 * \file bootp_builder.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An BOOTP frame builder class.
 */

#ifndef ASIOTAP_OSI_BOOTP_BUILDER_HPP
#define ASIOTAP_OSI_BOOTP_BUILDER_HPP

#include "builder.hpp"
#include "bootp_frame.hpp"

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An bootp frame builder class.
		 */
		template <>
		class builder<bootp_frame> : public _base_builder<bootp_frame>
		{
			public:

				/**
				 * \brief Create a builder.
				 * \param buf The buffer to use.
				 * \param payload_size The size of the payload.
				 */
				builder(boost::asio::mutable_buffer buf, size_t payload_size);

				/**
				 * \brief Write the frame.
				 * \param operation The operation.
				 * \param hardware_type The hardware type.
				 * \param hardware_length The hardware length.
				 * \param hops The number of hops.
				 * \param xid The x identifier.
				 * \param seconds The number of seconds since the beginning of the request.
				 * \param flags The flags.
				 * \param ciaddr The client address.
				 * \param yiaddr The future client address.
				 * \param siaddr The server address.
				 * \param giaddr The gateway address.
				 * \param chaddr The hardware address.
				 * \param sname The server name.
				 * \param file The filename to use for the boot sequence.
				 * \return The total size of the written frame, including its payload.
				 */
				size_t write(
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
				) const;
		};

		inline builder<bootp_frame>::builder(boost::asio::mutable_buffer buf, size_t payload_size) :
			_base_builder<bootp_frame>(buf, payload_size)
		{
		}
	}
}

#endif /* ASIOTAP_BOOTP_BUILDER_HPP */


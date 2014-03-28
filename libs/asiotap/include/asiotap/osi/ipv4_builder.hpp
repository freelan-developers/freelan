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
 * \file ipv4_builder.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IPV4 frame builder class.
 */

#ifndef ASIOTAP_OSI_IPV4_BUILDER_HPP
#define ASIOTAP_OSI_IPV4_BUILDER_HPP

#include "builder.hpp"
#include "ipv4_frame.hpp"

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An ipv4 frame builder class.
		 */
		template <>
		class builder<ipv4_frame> : public _base_builder<ipv4_frame>
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
				 * \param tos The type of service.
				 * \param identification The identification.
				 * \param flags The IP flags.
				 * \param position_fragment The position fragment.
				 * \param ttl The time to live.
				 * \param protocol The underlying protocol.
				 * \param source The source address.
				 * \param destination The destination address.
				 * \return The total size of the written frame, including its payload.
				 */
				size_t write(
				    uint8_t tos,
				    uint16_t identification,
				    uint8_t flags,
				    uint16_t position_fragment,
				    uint8_t ttl,
				    uint8_t protocol,
				    boost::asio::ip::address_v4 source,
				    boost::asio::ip::address_v4 destination
				) const;
		};

		inline builder<ipv4_frame>::builder(boost::asio::mutable_buffer buf, size_t payload_size) :
			_base_builder<ipv4_frame>(buf, payload_size)
		{
		}
	}
}

#endif /* ASIOTAP_IPV4_BUILDER_HPP */


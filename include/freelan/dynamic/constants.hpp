/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file constants.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The constants.
 */

#ifndef FREELAN_CONSTANTS_HPP
#define FREELAN_CONSTANTS_HPP

#include <cryptoplus/cipher/cipher_algorithm.hpp>
#include <cryptoplus/hash/message_digest_algorithm.hpp>

#include <stdint.h>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>

namespace freelan
{
	/**
	 * \brief The challenge type.
	 */
	typedef boost::array<uint8_t, 32> hash_type;

	/**
	 * \brief The current protocol version.
	 */
	const unsigned char CURRENT_PROTOCOL_VERSION = 1;

	/**
	 * \brief The different message types.
	 */
	enum message_type
	{
		MESSAGE_TYPE_CONTACT_REQUEST = 0x00,
		MESSAGE_TYPE_CONTACT = 0x01
	};

	/**
	 * \brief The endpoint family type.
	 */
	enum endpoint_family_type
	{
		ENDPOINT_FAMILY_IPV4 = 4,
		ENDPOINT_FAMILY_IPV6 = 6
	};
}

#endif /* FREELAN_CONSTANTS_HPP */

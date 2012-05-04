/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file contact_request_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A contact request message class.
 */

#include "contact_request_message.hpp"

#include <cryptoplus/cipher/cipher_context.hpp>
#include <cryptoplus/hash/hmac.hpp>
#include <cryptoplus/random/random.hpp>
#include <cassert>
#include <stdexcept>

namespace fscp
{
	std::vector<hash_type> contact_request_message::get_hash_list(session_number_type session_number, const void* enc_key, size_t enc_key_len) const
	{
		const std::vector<uint8_t> buf = get_cleartext<uint8_t>(session_number, enc_key, enc_key_len);

		const cryptoplus::hash::message_digest_algorithm certificate_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM);

		const size_t hash_size = certificate_digest_algorithm.result_size();

		if ((buf.size() / hash_size) * hash_size != buf.size())
		{
			throw std::runtime_error("Invalid message structure");
		}

		std::vector<hash_type> result;

		for (std::vector<uint8_t>::const_iterator it = buf.begin(); it != buf.end(); it += hash_size)
		{
			hash_type hash;

			std::copy(it, it + hash_size, hash.begin());

			result.push_back(hash);
		}

		return result;
	}
}

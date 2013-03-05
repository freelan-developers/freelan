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
 * \file constants.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The constants.
 */

#include "constants.hpp"

#include <cassert>

#include <cryptoplus/hash/message_digest_context.hpp>

namespace fscp
{
	const std::string cipher_algorithm_type::aes256_cbc_string("aes256-cbc");
	const std::string message_digest_algorithm_type::hmac_sha256_string("hmac-sha256");
	const std::string message_digest_algorithm_type::hmac_sha256_128_string("hmac-sha256-128");
	const std::string message_digest_algorithm_type::hmac_sha1_string("hmac-sha1");
	const std::string message_digest_algorithm_type::hmac_sha1_96_string("hmac-sha1-96");
	const std::string message_digest_algorithm_type::none_string("none");

	channel_number_type to_channel_number(message_type type)
	{
		assert(is_data_message_type(type));

		return static_cast<channel_number_type>(static_cast<uint8_t>(type) & 0x0F);
	}

	message_type to_data_message_type(channel_number_type channel_number)
	{
		assert(channel_number >= CHANNEL_NUMBER_0);
		assert(channel_number <= CHANNEL_NUMBER_15);

		return static_cast<message_type>(static_cast<uint8_t>(MESSAGE_TYPE_DATA_0) + static_cast<uint8_t>(channel_number));
	}

	void get_certificate_hash(void* buf, size_t buflen, cryptoplus::x509::certificate cert)
	{
		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM));
		mdctx.update(cert.write_der());
		mdctx.finalize(buf, buflen);
	}

	hash_type get_certificate_hash(cryptoplus::x509::certificate cert)
	{
		hash_type result;

		get_certificate_hash(&result[0], result.size(), cert);

		return result;
	}
}

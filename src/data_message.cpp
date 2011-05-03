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
 * \file data_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A data message class.
 */

#include "data_message.hpp"

#include <cryptoplus/cipher/cipher_context.hpp>
#include <cryptoplus/hash/hmac.hpp>
#include <cryptoplus/random/random.hpp>
#include <cassert>
#include <stdexcept>

namespace fscp
{
	size_t data_message::write(void* buf, size_t buf_len, sequence_number_type _sequence_number, const void* _cleartext, size_t cleartext_len, const void* sig_key, size_t sig_key_len, const void* enc_key, size_t enc_key_len)
	{
		assert(sig_key);
		assert(enc_key);

		const cryptoplus::cipher::cipher_algorithm cipher_algorithm(CIPHER_ALGORITHM);
		const cryptoplus::hash::message_digest_algorithm message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM);

		const size_t hmac_size = message_digest_algorithm.result_size();

		if (buf_len < HEADER_LENGTH + cipher_algorithm.iv_length() + cleartext_len + cipher_algorithm.block_size() + hmac_size)
		{
			throw std::runtime_error("buf_len");
		}

		uint8_t* const payload = static_cast<uint8_t*>(buf) + HEADER_LENGTH;
		const size_t payload_len = buf_len - HEADER_LENGTH;
		uint8_t* const iv = payload + sizeof(sequence_number_type) + sizeof(uint16_t);
		const size_t iv_len = cipher_algorithm.iv_length();
		uint8_t* const ciphertext = iv + iv_len;
		const size_t ciphertext_len = payload_len - sizeof(sequence_number_type) - sizeof(uint16_t) - iv_len;
		cryptoplus::random::get_random_bytes(iv, iv_len);

		cryptoplus::cipher::cipher_context cipher_context;
		cipher_context.initialize(cryptoplus::cipher::cipher_algorithm(CIPHER_ALGORITHM), cryptoplus::cipher::cipher_context::encrypt, enc_key, enc_key_len, iv, iv_len);
		size_t cnt = cipher_context.update(ciphertext, ciphertext_len, _cleartext, cleartext_len);
		cnt += cipher_context.finalize(ciphertext + cnt, ciphertext_len - cnt);

		buffer_tools::set<sequence_number_type>(payload, 0, htons(_sequence_number));
		buffer_tools::set<uint16_t>(payload, sizeof(sequence_number_type), htons(static_cast<uint16_t>(cleartext_len)));

		// The HMAC is cut in half
		const size_t length = sizeof(sequence_number_type) + sizeof(uint16_t) + cnt + hmac_size / 2;

		uint8_t* hmac = ciphertext + cnt;
		const size_t hmac_len = hmac_size;

		cryptoplus::hash::hmac(
		   	hmac,
		    hmac_len,
		    sig_key,
		    sig_key_len,
		    payload,
		    length - hmac_size / 2,
		    message_digest_algorithm
		);

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, MESSAGE_TYPE_DATA, length) + length;
	}

	data_message::data_message(const void* buf, size_t buf_len) :
		message(buf, buf_len)
	{
		check_format();
	}

	data_message::data_message(const message& _message) :
		message(_message)
	{
		check_format();
	}

	void data_message::check_format() const
	{
		if (length() < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("bad message length");
		}

		if (length() != MIN_BODY_LENGTH + initialization_vector_size() + ciphertext_size() + hmac_size())
		{
			throw std::runtime_error("bad message length");
		}
	}

	void data_message::check_signature(void* tmp, size_t tmp_len, const void* sig_key, size_t sig_key_len) const
	{
		assert(sig_key);

		size_t hmac_len = cryptoplus::hash::hmac(
				tmp,
				tmp_len,
				sig_key,
				sig_key_len,
				payload(),
				sizeof(sequence_number_type) + sizeof(uint16_t) + initialization_vector_size() + ciphertext_size(),
				cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM)
				);

		hmac_len /= 2;

		if ((hmac_len != hmac_size()) || (std::memcmp(hmac(), tmp, hmac_len) != 0))
		{
			throw std::runtime_error("hmac mismatch");
		}
	}

	size_t data_message::get_cleartext(void* buf, size_t buf_len, const void* enc_key, size_t enc_key_len) const
	{
		assert(enc_key);

		if (buf)
		{
			cryptoplus::cipher::cipher_context cipher_context;
			cipher_context.initialize(cryptoplus::cipher::cipher_algorithm(CIPHER_ALGORITHM), cryptoplus::cipher::cipher_context::decrypt, enc_key, enc_key_len, initialization_vector(), initialization_vector_size());
			size_t cnt = cipher_context.update(buf, buf_len, ciphertext(), ciphertext_size());
			cnt += cipher_context.finalize(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt);

			return cnt;
		}
		else
		{
			return ciphertext_size();
		}
	}
}

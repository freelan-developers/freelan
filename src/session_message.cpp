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
 * \file session_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session message class.
 */

#include "session_message.hpp"

#include "constants.hpp"

#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/pkey/pkey.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cassert>
#include <stdexcept>

namespace fscp
{
	session_message::session_message(const message& _message, size_t pkey_size) :
		message(_message),
		m_pkey_size(pkey_size)
	{
		check_format();
	}

	void session_message::check_format() const
	{
		if (length() < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("bad message length");
		}

		if (length() < MIN_BODY_LENGTH + ciphertext_size())
		{
			throw std::runtime_error("bad message length");
		}

		if (length() != MIN_BODY_LENGTH + ciphertext_size() + ciphertext_signature_size())
		{
			throw std::runtime_error("bad message length");
		}
	}

	void session_message::check_signature(cryptoplus::pkey::pkey key) const
	{
		assert(key);
		assert(key.get_rsa_key());

		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM));
		mdctx.update(ciphertext(), ciphertext_size());

		const cryptoplus::buffer digest = mdctx.finalize();
		const cryptoplus::buffer padded_buf = key.get_rsa_key().public_decrypt(ciphertext_signature(), ciphertext_signature_size(), RSA_NO_PADDING);

		key.get_rsa_key().verify_PKCS1_PSS(digest, padded_buf, cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM), -1);
	}

	size_t session_message::get_cleartext(void* buf, size_t buf_len, cryptoplus::pkey::pkey key) const
	{
		assert(key);
		assert(key.size() == m_pkey_size);

		if (buf)
		{
			size_t result = 0;

			for (unsigned int ciphertext_index = 0; ciphertext_index < ciphertext_count(); ++ciphertext_index)
			{
				result += key.get_rsa_key().private_decrypt(static_cast<char*>(buf) + result, buf_len - result, ciphertext() + ciphertext_index * m_pkey_size, m_pkey_size, RSA_PKCS1_OAEP_PADDING);
			}

			return result;
		}
		else
		{
			return key.get_rsa_key().size() * ciphertext_count();
		}
	}

	size_t session_message::_write(void* buf, size_t buf_len, const void* ciphertext, size_t ciphertext_len, unsigned int ciphertext_cnt, const void* ciphertext_signature, size_t ciphertext_signature_len, message_type type)
	{
		const size_t payload_len = MIN_BODY_LENGTH + ciphertext_len + ciphertext_signature_len;

		if (buf_len < HEADER_LENGTH + payload_len)
		{
			throw std::runtime_error("buf_len");
		}

		buffer_tools::set<uint16_t>(buf, HEADER_LENGTH, htons(static_cast<uint16_t>(ciphertext_cnt)));
		std::memcpy(static_cast<uint8_t*>(buf) + HEADER_LENGTH + sizeof(uint16_t), ciphertext, ciphertext_len);
		buffer_tools::set<uint16_t>(buf, HEADER_LENGTH + sizeof(uint16_t) + ciphertext_len, htons(static_cast<uint16_t>(ciphertext_signature_len)));
		std::memcpy(static_cast<uint8_t*>(buf) + HEADER_LENGTH + 2 * sizeof(uint16_t) + ciphertext_len, ciphertext_signature, ciphertext_signature_len);

		message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, type, payload_len);

		return HEADER_LENGTH + payload_len;
	}

	size_t session_message::_write(void* buf, size_t buf_len, const void* cleartext, size_t cleartext_len, cryptoplus::pkey::pkey enc_key, cryptoplus::pkey::pkey sig_key, message_type type)
	{
		const size_t max_cleartext_len = enc_key.size() - cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM).result_size() * 2 - 2;
		const unsigned int packet_count = (cleartext_len + max_cleartext_len - 1) / max_cleartext_len;

		if (packet_count >= (1 << 16))
		{
			throw std::runtime_error("Too many ciphertexts");
		}

		std::vector<uint8_t> ciphertext(packet_count * enc_key.size());

		for (unsigned int packet_index = 0; packet_index < packet_count; ++packet_index)
		{
			enc_key.get_rsa_key().public_encrypt(&ciphertext[0 + packet_index * enc_key.size()], enc_key.size(), static_cast<const char*>(cleartext) + packet_index * max_cleartext_len, max_cleartext_len, RSA_PKCS1_OAEP_PADDING);
		}

		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM));
		mdctx.update(&ciphertext[0], ciphertext.size());
		const cryptoplus::buffer digest = mdctx.finalize();

		cryptoplus::buffer padded_buf(sig_key.get_rsa_key().size());
		sig_key.get_rsa_key().padding_add_PKCS1_PSS(cryptoplus::buffer_cast<uint8_t>(padded_buf), cryptoplus::buffer_size(padded_buf), cryptoplus::buffer_cast<uint8_t>(digest), cryptoplus::buffer_size(digest), cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM), -1);

		const cryptoplus::buffer ciphertext_signature = sig_key.get_rsa_key().private_encrypt(padded_buf, RSA_NO_PADDING);

		return _write(buf, buf_len, &ciphertext[0], ciphertext.size(), packet_count, cryptoplus::buffer_cast<uint8_t>(ciphertext_signature), cryptoplus::buffer_size(ciphertext_signature), type);
	}

}

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

#include <cassert>
#include <stdexcept>

#include <cryptoplus/hash/message_digest_context.hpp>

namespace fscp
{
	size_t session_message::write(void* buf, size_t buf_len, session_number_type _session_number, const host_identifier_type& _host_identifier, elliptic_curve_type ec, key_derivation_algorithm_type kd, cipher_algorithm_type calg, const void* pub_key, size_t pub_key_len, const void* _salt, size_t salt_len, const void* nonce_prefix, size_t nonce_prefix_len, cryptoplus::pkey::pkey sig_key)
	{
		using cryptoplus::buffer_cast;
		using cryptoplus::buffer_size;

		const size_t unsigned_payload_size = MIN_BODY_LENGTH + pub_key_len + nonce_prefix_len + salt_len;
		const size_t signed_payload_size = unsigned_payload_size + sig_key.get_rsa_key().size();

		if (buf_len < HEADER_LENGTH + signed_payload_size)
		{
			throw std::runtime_error("buf_len");
		}

		uint8_t* const payload = static_cast<uint8_t*>(buf) + HEADER_LENGTH;

		buffer_tools::set<session_number_type>(payload, 0, htonl(_session_number));
		std::copy(_host_identifier.begin(), _host_identifier.end(), payload + sizeof(_session_number));
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size, ec.value());
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t), kd.value());
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 2, calg.value());
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3, 0x00);
		buffer_tools::set<uint16_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1, htons(static_cast<uint16_t>(pub_key_len)));
		std::memcpy(static_cast<uint8_t*>(payload) + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t), pub_key, pub_key_len);
		buffer_tools::set<uint16_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + pub_key_len, htons(static_cast<uint16_t>(salt_len)));
		std::memcpy(static_cast<uint8_t*>(payload) + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + pub_key_len + sizeof(uint16_t), _salt, salt_len);
		buffer_tools::set<uint16_t>(payload, sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + pub_key_len + sizeof(uint16_t) + salt_len, htons(static_cast<uint16_t>(nonce_prefix_len)));
		std::memcpy(static_cast<uint8_t*>(payload) + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + pub_key_len + sizeof(uint16_t) + salt_len + sizeof(uint16_t), nonce_prefix, nonce_prefix_len);

		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM));
		mdctx.update(static_cast<const uint8_t*>(payload), unsigned_payload_size);
		const cryptoplus::buffer digest = mdctx.finalize();

		cryptoplus::buffer padded_buf(sig_key.get_rsa_key().size());
		sig_key.get_rsa_key().padding_add_PKCS1_PSS(buffer_cast<uint8_t*>(padded_buf), buffer_size(padded_buf), cryptoplus::buffer_cast<const uint8_t*>(digest), cryptoplus::buffer_size(digest), cryptoplus::hash::message_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM), -1);

		const size_t signature_size = sig_key.get_rsa_key().private_encrypt(payload + unsigned_payload_size + sizeof(uint16_t), sig_key.get_rsa_key().size(), buffer_cast<const uint8_t*>(padded_buf), buffer_size(padded_buf), RSA_NO_PADDING);
		buffer_tools::set<uint16_t>(payload, unsigned_payload_size, htons(static_cast<uint16_t>(signature_size)));

		const size_t length = unsigned_payload_size + sizeof(uint16_t) + signature_size;

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, MESSAGE_TYPE_SESSION, length) + length;
	}

	session_message::session_message(const message& _message) :
		message(_message)
	{
		if (length() < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + public_key_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + public_key_size() + salt_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + public_key_size() + salt_size() + nonce_prefix_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + public_key_size() + salt_size() + nonce_prefix_size() + header_signature_size())
		{
			throw std::runtime_error("buf_len");
		}
	}

	void session_message::check_signature(cryptoplus::pkey::pkey key) const
	{
		assert(key);
		assert(key.get_rsa_key());

		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM));
		mdctx.update(payload(), header_size());

		const cryptoplus::buffer digest = mdctx.finalize();
		const cryptoplus::buffer padded_buf = key.get_rsa_key().public_decrypt(header_signature(), header_signature_size(), RSA_NO_PADDING);

		key.get_rsa_key().verify_PKCS1_PSS(digest, padded_buf, cryptoplus::hash::message_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM), -1);
	}
}

/*
 * libfscp - A C++ library to establish peer-to-peer virtual private networks.
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
#include <cryptoplus/hash/hmac_context.hpp>

namespace fscp
{
	namespace
	{
		void configure_context(EVP_PKEY_CTX* evp_ctx)
		{
			// Set RSASSA_PSS with a digest size salt length.
			EVP_PKEY_CTX_set_rsa_padding(evp_ctx, RSA_PKCS1_PSS_PADDING);
			EVP_PKEY_CTX_set_rsa_pss_saltlen(evp_ctx, -1);
		}
	}

	size_t session_message::write(void* buf, size_t buf_len, session_number_type _session_number, const host_identifier_type& _host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len, cryptoplus::pkey::pkey sig_key)
	{
		uint8_t* const payload = static_cast<uint8_t*>(buf) + HEADER_LENGTH;
		const size_t unsigned_payload_size = write_unsigned(payload, buf_len - HEADER_LENGTH, _session_number, _host_identifier, cs, ec, pub_key, pub_key_len);

		cryptoplus::hash::message_digest_context mdctx;
		EVP_PKEY_CTX* evp_ctx = nullptr;

		mdctx.digest_sign_initialize(get_default_digest_algorithm(), sig_key, &evp_ctx);
		configure_context(evp_ctx);
		mdctx.digest_sign_update(static_cast<const uint8_t*>(payload), unsigned_payload_size);

		const size_t signature_size = mdctx.digest_sign_finalize(nullptr, 0);
		const size_t signed_payload_size = unsigned_payload_size + sizeof(uint16_t) + signature_size;

		if (buf_len < HEADER_LENGTH + signed_payload_size)
		{
			throw std::runtime_error("buf_len");
		}

		mdctx.digest_sign_finalize(payload + unsigned_payload_size + sizeof(uint16_t), signature_size);
		buffer_tools::set<uint16_t>(payload, unsigned_payload_size, htons(static_cast<uint16_t>(signature_size)));

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, MESSAGE_TYPE_SESSION, signed_payload_size) + signed_payload_size;
	}

	size_t session_message::write(void* buf, size_t buf_len, session_number_type _session_number, const host_identifier_type& _host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len, const void* pre_shared_key, size_t pre_shared_key_len)
	{
		const auto mdalg = get_default_digest_algorithm();
		uint8_t* const payload = static_cast<uint8_t*>(buf) + HEADER_LENGTH;
		const size_t unsigned_payload_size = write_unsigned(payload, buf_len - HEADER_LENGTH, _session_number, _host_identifier, cs, ec, pub_key, pub_key_len);

		if (buf_len < HEADER_LENGTH + unsigned_payload_size + mdalg.result_size())
		{
			throw std::runtime_error("buf_len");
		}

		cryptoplus::hash::hmac_context hmctx;
		hmctx.initialize(pre_shared_key, pre_shared_key_len, &mdalg);
		hmctx.update(static_cast<const uint8_t*>(payload), unsigned_payload_size);
		hmctx.finalize(payload + unsigned_payload_size + sizeof(uint16_t), mdalg.result_size());
		buffer_tools::set<uint16_t>(payload, unsigned_payload_size, htons(static_cast<uint16_t>(mdalg.result_size())));

		const size_t signed_payload_size = unsigned_payload_size + sizeof(uint16_t) + mdalg.result_size();

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, MESSAGE_TYPE_SESSION, signed_payload_size) + signed_payload_size;
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

		if (length() < MIN_BODY_LENGTH + public_key_size() + header_signature_size())
		{
			throw std::runtime_error("buf_len");
		}
	}

	bool session_message::check_signature(cryptoplus::pkey::pkey key) const
	{
		assert(key);
		assert(key.get_rsa_key());

		cryptoplus::hash::message_digest_context mdctx;
		EVP_PKEY_CTX* evp_ctx = nullptr;

		mdctx.digest_verify_initialize(get_default_digest_algorithm(), key, &evp_ctx);
		configure_context(evp_ctx);
		mdctx.digest_verify_update(payload(), header_size());

		return mdctx.digest_verify_finalize(header_signature(), header_signature_size());
	}

	bool session_message::check_signature(const void* pre_shared_key, size_t pre_shared_key_len) const
	{
		const auto mdalg = get_default_digest_algorithm();

		cryptoplus::hash::hmac_context hmctx;
		hmctx.initialize(pre_shared_key, pre_shared_key_len, &mdalg);
		hmctx.update(payload(), header_size());
		const auto verified_signature = hmctx.finalize();
		const cryptoplus::buffer signature(header_signature(), header_signature_size());

		return (signature == verified_signature);
	}

	size_t session_message::write_unsigned(uint8_t* payload, size_t payload_len, session_number_type _session_number, const host_identifier_type& _host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len)
	{
		using cryptoplus::buffer_cast;
		using cryptoplus::buffer_size;

		const size_t unsigned_payload_size = MIN_BODY_LENGTH + pub_key_len;

		if (payload_len < unsigned_payload_size)
		{
			throw std::runtime_error("payload_len");
		}

		buffer_tools::set<session_number_type>(payload, 0, htonl(_session_number));
		std::copy(_host_identifier.data.begin(), _host_identifier.data.end(), payload + sizeof(_session_number));
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::data_type::static_size, cs.value());
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t), ec.value());
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 2, 0x00);
		buffer_tools::set<uint8_t>(payload, sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 3, 0x00);
		buffer_tools::set<uint16_t>(payload, sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 4, htons(static_cast<uint16_t>(pub_key_len)));
		std::memcpy(static_cast<uint8_t*>(payload)+sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 4 + sizeof(uint16_t), pub_key, pub_key_len);

		return unsigned_payload_size;
	}
}

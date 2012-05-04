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
	size_t data_message::write(void* buf, size_t buf_len, channel_number_type channel_number, session_number_type _session_number, sequence_number_type _sequence_number, const void* _cleartext, size_t cleartext_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		return raw_write(buf, buf_len, _session_number, _sequence_number, _cleartext, cleartext_len, seal_key, seal_key_len, enc_key, enc_key_len, to_data_message_type(channel_number));
	}

	size_t data_message::write_keep_alive(void* buf, size_t buf_len, session_number_type _session_number, sequence_number_type _sequence_number, size_t random_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		std::vector<unsigned char> random = cryptoplus::random::get_random_bytes<unsigned char>(random_len);

		return raw_write(buf, buf_len, _session_number, _sequence_number, &random[0], random.size(), seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_KEEP_ALIVE);
	}

	size_t data_message::write_contact_request(void* buf, size_t buf_len, session_number_type session_number, sequence_number_type _sequence_number, const contact_map_type& contact_map, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		std::vector<uint8_t> cleartext;
		cleartext.resize(contact_map.size() * 49);

		std::vector<uint8_t>::iterator ptr = cleartext.begin();

		for (contact_map_type::const_iterator it = contact_map.begin(); it != contact_map.end(); ++it)
		{
			// We copy the hash
			ptr = std::copy(it->first.begin(), it->first.end(), ptr);

			if (it->second.address().is_v4())
			{
				*(ptr++) = static_cast<uint8_t>(ENDPOINT_TYPE_IPV4);

				boost::asio::ip::address_v4::bytes_type bytes = it->second.address().to_v4().to_bytes();

				ptr = std::copy(bytes.begin(), bytes.end(), ptr);

				*(ptr + sizeof(uint16_t)) = htons(it->second.port()); ptr += sizeof(uint16_t);
			}
			else if (it->second.address().is_v6())
			{
				*(ptr++) = static_cast<uint8_t>(ENDPOINT_TYPE_IPV6);

				boost::asio::ip::address_v6::bytes_type bytes = it->second.address().to_v6().to_bytes();

				ptr = std::copy(bytes.begin(), bytes.end(), ptr);

				*(ptr + sizeof(uint16_t)) = htons(it->second.port()); ptr += sizeof(uint16_t);
			}
		}

		cleartext.resize(std::distance(cleartext.begin(), ptr));

		return raw_write(buf, buf_len, session_number, _sequence_number, &cleartext[0], cleartext.size(), seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_CONTACT);
	}

	std::vector<hash_type> data_message::parse_hash_list(void* buf, size_t buflen)
	{
		const cryptoplus::hash::message_digest_algorithm certificate_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM);

		const size_t hash_size = certificate_digest_algorithm.result_size();

		if ((buflen / hash_size) * hash_size != buflen)
		{
			throw std::runtime_error("Invalid message structure");
		}

		std::vector<hash_type> result;

		for (const uint8_t* ptr = static_cast<const uint8_t*>(buf); ptr < static_cast<const uint8_t*>(buf) + buflen; ptr += hash_size)
		{
			hash_type hash;

			std::copy(ptr, ptr + hash_size, hash.begin());

			result.push_back(hash);
		}

		return result;
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

		if (length() != MIN_BODY_LENGTH + ciphertext_size() + hmac_size())
		{
			throw std::runtime_error("bad message length");
		}
	}

	void data_message::check_seal(void* tmp, size_t tmp_len, const void* seal_key, size_t seal_key_len) const
	{
		assert(seal_key);

		size_t hmac_len = cryptoplus::hash::hmac(
		                      tmp,
		                      tmp_len,
		                      seal_key,
		                      seal_key_len,
		                      payload(),
		                      sizeof(sequence_number_type) + sizeof(uint16_t) + ciphertext_size(),
		                      cryptoplus::hash::message_digest_algorithm(MESSAGE_DIGEST_ALGORITHM)
		                  );

		hmac_len /= 2;

		if ((hmac_len != hmac_size()) || (std::memcmp(hmac(), tmp, hmac_len) != 0))
		{
			throw std::runtime_error("hmac mismatch");
		}
	}

	size_t data_message::get_cleartext(void* buf, size_t buf_len, session_number_type session_number, const void* enc_key, size_t enc_key_len) const
	{
		assert(enc_key);

		if (buf)
		{
			cryptoplus::cipher::cipher_algorithm cipher_algorithm(CIPHER_ALGORITHM);

			const std::vector<uint8_t> iv = compute_initialization_vector<uint8_t>(session_number, sequence_number(), enc_key, enc_key_len);

			cryptoplus::cipher::cipher_context cipher_context;
			cipher_context.initialize(cipher_algorithm, cryptoplus::cipher::cipher_context::decrypt, enc_key, enc_key_len, &iv[0], iv.size());
			cipher_context.set_padding(false);
			size_t cnt = cipher_context.update(buf, buf_len, ciphertext(), ciphertext_size());
			cnt += cipher_context.finalize(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt);

			cnt = cipher_context.verify_iso_10126_padding(buf, cnt);

			return cnt;
		}
		else
		{
			return ciphertext_size();
		}
	}

	size_t data_message::compute_initialization_vector(void* buf, size_t buf_len, session_number_type session_number, sequence_number_type sequence_number, const void* enc_key, size_t enc_key_len)
	{
		cryptoplus::cipher::cipher_algorithm cipher_algorithm(CIPHER_ALGORITHM);

		if (buf)
		{
			static const unsigned char null_iv[16] = {};
			static const char must_be_zero_padding[10] = {};
			session_number = htonl(session_number);
			sequence_number = htons(sequence_number);

			cryptoplus::cipher::cipher_algorithm iv_cipher_algorithm(IV_CIPHER_ALGORITHM);

			cryptoplus::cipher::cipher_context cipher_context;
			cipher_context.initialize(iv_cipher_algorithm, cryptoplus::cipher::cipher_context::encrypt, enc_key, enc_key_len, null_iv, sizeof(null_iv));
			cipher_context.set_padding(false);
			size_t cnt = cipher_context.update(buf, buf_len, &session_number, sizeof(session_number));
			cnt += cipher_context.update(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt, &sequence_number, sizeof(sequence_number));
			cnt += cipher_context.update(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt, must_be_zero_padding, sizeof(must_be_zero_padding));
			cnt += cipher_context.finalize(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt);

			return cnt;
		}
		else
		{
			return cipher_algorithm.iv_length() * 2;
		}
	}

	size_t data_message::raw_write(void* buf, size_t buf_len, session_number_type _session_number, sequence_number_type _sequence_number, const void* _cleartext, size_t cleartext_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len, message_type type)
	{
		assert(seal_key);
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
		uint8_t* const ciphertext = payload + sizeof(sequence_number_type) + sizeof(uint16_t);
		const size_t ciphertext_len = payload_len - sizeof(sequence_number_type) - sizeof(uint16_t);

		const std::vector<uint8_t> iv = compute_initialization_vector<uint8_t>(_session_number, _sequence_number, enc_key, enc_key_len);

		cryptoplus::cipher::cipher_context cipher_context;
		cipher_context.initialize(cryptoplus::cipher::cipher_algorithm(CIPHER_ALGORITHM), cryptoplus::cipher::cipher_context::encrypt, enc_key, enc_key_len, &iv[0], iv.size());
		cipher_context.set_padding(false);

		std::vector<uint8_t> cleartext = cipher_context.get_iso_10126_padded_buffer<uint8_t>(_cleartext, cleartext_len);

		size_t cnt = cipher_context.update(ciphertext, ciphertext_len, &cleartext[0], cleartext.size());
		cnt += cipher_context.finalize(ciphertext + cnt, ciphertext_len - cnt);

		buffer_tools::set<sequence_number_type>(payload, 0, htons(_sequence_number));
		buffer_tools::set<uint16_t>(payload, sizeof(sequence_number_type), htons(static_cast<uint16_t>(cnt / cipher_algorithm.block_size())));

		// The HMAC is cut in half
		const size_t length = sizeof(sequence_number_type) + sizeof(uint16_t) + cnt + hmac_size / 2;

		uint8_t* hmac = ciphertext + cnt;
		const size_t hmac_len = hmac_size;

		cryptoplus::hash::hmac(
		    hmac,
		    hmac_len,
		    seal_key,
		    seal_key_len,
		    payload,
		    length - hmac_size / 2,
		    message_digest_algorithm
		);

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, type, length) + length;
	}
}

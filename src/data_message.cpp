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
	size_t data_message::write(void* buf, size_t buf_len, channel_number_type channel_number, sequence_number_type _sequence_number, data_message::calg_t cipher_algorithm, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, const void* _cleartext, size_t cleartext_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		return raw_write(buf, buf_len, _sequence_number, cipher_algorithm, message_digest_algorithm, message_digest_algorithm_hmac_size, _cleartext, cleartext_len, seal_key, seal_key_len, enc_key, enc_key_len, to_data_message_type(channel_number));
	}

	size_t data_message::write_keep_alive(void* buf, size_t buf_len, sequence_number_type _sequence_number, data_message::calg_t cipher_algorithm, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, size_t random_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		const cryptoplus::buffer random = cryptoplus::random::get_random_bytes(random_len);

		return raw_write(buf, buf_len, _sequence_number, cipher_algorithm, message_digest_algorithm, message_digest_algorithm_hmac_size, cryptoplus::buffer_cast<uint8_t>(random), cryptoplus::buffer_size(random), seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_KEEP_ALIVE);
	}

	size_t data_message::write_contact_request(void* buf, size_t buf_len, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, const hash_list_type& hash_list, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		return raw_write(buf, buf_len, sequence_number, cipher_algorithm, message_digest_algorithm, message_digest_algorithm_hmac_size, reinterpret_cast<const char*>(&hash_list[0]), hash_list.size() * hash_type::static_size, seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_CONTACT_REQUEST);
	}

	size_t data_message::write_contact(void* buf, size_t buf_len, sequence_number_type _sequence_number, data_message::calg_t cipher_algorithm, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, const contact_map_type& contact_map, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
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

				*(reinterpret_cast<uint16_t*>(&*ptr)) = htons(it->second.port());

				ptr += sizeof(uint16_t);
			}
			else if (it->second.address().is_v6())
			{
				*(ptr++) = static_cast<uint8_t>(ENDPOINT_TYPE_IPV6);

				boost::asio::ip::address_v6::bytes_type bytes = it->second.address().to_v6().to_bytes();

				ptr = std::copy(bytes.begin(), bytes.end(), ptr);

				*(reinterpret_cast<uint16_t*>(&*ptr)) = htons(it->second.port());

				ptr += sizeof(uint16_t);
			}
		}

		cleartext.resize(std::distance(cleartext.begin(), ptr));

		return raw_write(buf, buf_len, _sequence_number, cipher_algorithm, message_digest_algorithm, message_digest_algorithm_hmac_size, &cleartext[0], cleartext.size(), seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_CONTACT);
	}

	std::vector<hash_type> data_message::parse_hash_list(void* buf, size_t buflen)
	{
		if ((buflen / hash_type::static_size) * hash_type::static_size != buflen)
		{
			throw std::runtime_error("Invalid message structure");
		}

		std::vector<hash_type> result;

		for (const uint8_t* ptr = static_cast<const uint8_t*>(buf); ptr < static_cast<const uint8_t*>(buf) + buflen; ptr += hash_type::static_size)
		{
			hash_type hash;

			std::copy(ptr, ptr + hash_type::static_size, hash.begin());

			result.push_back(hash);
		}

		return result;
	}

	contact_map_type data_message::parse_contact_map(void* buf, size_t buflen)
	{
		contact_map_type result;

		for (const uint8_t* ptr = static_cast<const uint8_t*>(buf); ptr < static_cast<const uint8_t*>(buf) + buflen;)
		{
			hash_type hash;

			if (static_cast<const uint8_t*>(buf) + buflen - ptr < static_cast<ptrdiff_t>(hash_type::static_size) + 1)
			{
				throw std::runtime_error("Invalid message structure");
			}

			std::copy(ptr, ptr + hash_type::static_size, hash.begin());

			ptr += hash_type::static_size;

			switch (static_cast<endpoint_type_type>(*ptr))
			{
				case ENDPOINT_TYPE_IPV4:
					{
						++ptr;

						boost::asio::ip::address_v4::bytes_type bytes;

						if (static_cast<const uint8_t*>(buf) + buflen - ptr < static_cast<ptrdiff_t>(bytes.size()))
						{
							throw std::runtime_error("Invalid message structure");
						}

						std::copy(ptr, ptr + bytes.size(), bytes.begin());

						ptr += bytes.size();

						result[hash] = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4(bytes), ntohs(*reinterpret_cast<const uint16_t*>(ptr)));

						ptr += sizeof(uint16_t);

						break;
					}
				case ENDPOINT_TYPE_IPV6:
					{
						++ptr;

						boost::asio::ip::address_v6::bytes_type bytes;

						if (static_cast<const uint8_t*>(buf) + buflen - ptr < static_cast<ptrdiff_t>(bytes.size()))
						{
							throw std::runtime_error("Invalid message structure");
						}

						std::copy(ptr, ptr + bytes.size(), bytes.begin());

						ptr += bytes.size();

						result[hash] = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v6(bytes), ntohs(*reinterpret_cast<const uint16_t*>(ptr)));

						ptr += sizeof(uint16_t);

						break;
					}
				default:
					{
						throw std::runtime_error("Invalid message structure");
					}
			}
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
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + iv_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + iv_size() + ciphertext_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (length() < MIN_BODY_LENGTH + iv_size() + ciphertext_size() + hmac_size())
		{
			throw std::runtime_error("buf_len");
		}
	}

	void data_message::check_seal(void* tmp, size_t tmp_len, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, const void* seal_key, size_t seal_key_len) const
	{
		assert(seal_key);

		if (message_digest_algorithm)
		{
			if (hmac_size() != message_digest_algorithm_hmac_size)
			{
				throw std::runtime_error("hmac size mismatch");
			}

			size_t hmac_len = cryptoplus::hash::hmac(
					tmp,
					tmp_len,
					seal_key,
					seal_key_len,
					payload(),
					sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size() + sizeof(uint16_t) + ciphertext_size(),
					*message_digest_algorithm
					);

			if ((hmac_len < message_digest_algorithm_hmac_size) || (std::memcmp(hmac(), tmp, message_digest_algorithm_hmac_size) != 0))
			{
				throw std::runtime_error("hmac mismatch");
			}
		}
	}

	size_t data_message::get_cleartext(void* buf, size_t buf_len, data_message::calg_t cipher_algorithm, const void* enc_key, size_t enc_key_len) const
	{
		assert(enc_key);

		if (buf)
		{
			cryptoplus::cipher::cipher_context cipher_context;
			cipher_context.initialize(cipher_algorithm, cryptoplus::cipher::cipher_context::decrypt, enc_key, enc_key_len, iv(), iv_size());
			cipher_context.set_padding(false);
			size_t cnt = cipher_context.update(buf, buf_len, ciphertext(), ciphertext_size());
			cnt += cipher_context.finalize(static_cast<uint8_t*>(buf) + cnt, buf_len - cnt);

			try
			{
				cnt = cipher_context.verify_iso_10126_padding(buf, cnt);
			}
			catch (std::logic_error&)
			{
				throw std::runtime_error("Incorrect padding in the RSA ciphertext");
			}

			return cnt;
		}
		else
		{
			return ciphertext_size();
		}
	}

	size_t data_message::raw_write(void* buf, size_t buf_len, sequence_number_type _sequence_number, data_message::calg_t cipher_algorithm, boost::optional<data_message::mdalg_t> message_digest_algorithm, size_t message_digest_algorithm_hmac_size, const void* _cleartext, size_t cleartext_len, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len, message_type type)
	{
		assert(seal_key);
		assert(enc_key);

		const size_t iv_size = cipher_algorithm.iv_length();
		const size_t raw_hmac_size = message_digest_algorithm ? message_digest_algorithm->result_size() : 0;

		if (buf_len < HEADER_LENGTH + sizeof(sequence_number_type) + sizeof(uint16_t) * 3 + iv_size + raw_hmac_size + (cleartext_len + cipher_algorithm.block_size()))
		{
			throw std::runtime_error("buf_len");
		}

		uint8_t* const payload = static_cast<uint8_t*>(buf) + HEADER_LENGTH;
		uint8_t* const iv = payload + sizeof(sequence_number_type) + sizeof(uint16_t);

		buffer_tools::set<uint16_t>(payload, sizeof(sequence_number_type), htons(static_cast<uint16_t>(iv_size)));
		cryptoplus::random::get_random_bytes(iv, iv_size);

		uint8_t* const ciphertext = payload + sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size + sizeof(uint16_t);

		buffer_tools::set<sequence_number_type>(payload, 0, htonl(_sequence_number));

		cryptoplus::cipher::cipher_context cipher_context;
		cipher_context.initialize(cipher_algorithm, cryptoplus::cipher::cipher_context::encrypt, enc_key, enc_key_len, iv, iv_size);
		cipher_context.set_padding(false);

		const size_t max_ciphertext_len = buf_len - HEADER_LENGTH - sizeof(sequence_number_type) - sizeof(uint16_t) * 3 - iv_size - raw_hmac_size - cipher_algorithm.block_size();

		const cryptoplus::buffer cleartext = cipher_context.get_iso_10126_padded_buffer(_cleartext, cleartext_len);

		size_t ciphertext_len = cipher_context.update(ciphertext, max_ciphertext_len, cleartext);
		ciphertext_len += cipher_context.finalize(ciphertext + ciphertext_len, max_ciphertext_len - ciphertext_len);

		buffer_tools::set<uint16_t>(payload, sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size, htons(static_cast<uint16_t>(ciphertext_len)));

		uint8_t* const hmac = payload + sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size + sizeof(uint16_t) + ciphertext_len + sizeof(uint16_t);
		const size_t max_hmac_len = buf_len - HEADER_LENGTH - sizeof(sequence_number_type) - sizeof(uint16_t) - iv_size - sizeof(uint16_t) - ciphertext_len - sizeof(uint16_t);

		if (message_digest_algorithm)
		{
			const size_t hmac_len = cryptoplus::hash::hmac(
					hmac,
					max_hmac_len,
					seal_key,
					seal_key_len,
					payload,
					sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size + sizeof(uint16_t) + ciphertext_len,
					*message_digest_algorithm
					);

			if (hmac_len < message_digest_algorithm_hmac_size)
			{
				throw std::runtime_error("invalid HMAC length");
			}
		}

		buffer_tools::set<uint16_t>(payload, sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size + sizeof(uint16_t) + ciphertext_len, htons(static_cast<uint16_t>(message_digest_algorithm_hmac_size)));

		const size_t length = sizeof(sequence_number_type) + sizeof(uint16_t) + iv_size + sizeof(uint16_t) + ciphertext_len + sizeof(uint16_t) + message_digest_algorithm_hmac_size;

		return message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, type, length) + length;
	}
}

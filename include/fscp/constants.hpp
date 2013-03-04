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
 * \file constants.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The constants.
 */

#ifndef FSCP_CONSTANTS_HPP
#define FSCP_CONSTANTS_HPP

#include <boost/asio.hpp>

#include <cryptoplus/cipher/cipher_algorithm.hpp>
#include <cryptoplus/hash/message_digest_algorithm.hpp>
#include <cryptoplus/x509/certificate.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/utility/enable_if.hpp>

#include <stdint.h>
#include <vector>
#include <map>

#include "enumerations.hpp"

namespace fscp
{
	/**
	 * \brief The session number type.
	 */
	typedef uint32_t session_number_type;

	/**
	 * \brief The challenge type.
	 */
	typedef boost::array<uint8_t, 32> challenge_type;

	/**
	 * \brief The hash type.
	 */
	typedef boost::array<uint8_t, 32> hash_type;

	/**
	 * \brief The hash list type.
	 */
	typedef std::vector<hash_type> hash_list_type;

	/**
	 * \brief The contact map type.
	 */
	typedef std::map<hash_type, boost::asio::ip::udp::endpoint> contact_map_type;

	/**
	 * \brief The sequence number type.
	 */
	typedef uint32_t sequence_number_type;

	/**
	 * \brief The current protocol version.
	 */
	const unsigned char CURRENT_PROTOCOL_VERSION = 2;

	/**
	 * \brief The different message types.
	 */
	enum message_type
	{
		MESSAGE_TYPE_HELLO_REQUEST = 0x00,
		MESSAGE_TYPE_HELLO_RESPONSE = 0x01,
		MESSAGE_TYPE_PRESENTATION = 0x02,
		MESSAGE_TYPE_SESSION_REQUEST = 0x03,
		MESSAGE_TYPE_SESSION = 0x04,
		MESSAGE_TYPE_DATA_0 = 0x70,
		MESSAGE_TYPE_DATA_1 = 0x71,
		MESSAGE_TYPE_DATA_2 = 0x72,
		MESSAGE_TYPE_DATA_3 = 0x73,
		MESSAGE_TYPE_DATA_4 = 0x74,
		MESSAGE_TYPE_DATA_5 = 0x75,
		MESSAGE_TYPE_DATA_6 = 0x76,
		MESSAGE_TYPE_DATA_7 = 0x77,
		MESSAGE_TYPE_DATA_8 = 0x78,
		MESSAGE_TYPE_DATA_9 = 0x79,
		MESSAGE_TYPE_DATA_10 = 0x7A,
		MESSAGE_TYPE_DATA_11 = 0x7B,
		MESSAGE_TYPE_DATA_12 = 0x7C,
		MESSAGE_TYPE_DATA_13 = 0x7D,
		MESSAGE_TYPE_DATA_14 = 0x7E,
		MESSAGE_TYPE_DATA_15 = 0x7F,
		MESSAGE_TYPE_CONTACT_REQUEST = 0xFD,
		MESSAGE_TYPE_CONTACT = 0xFE,
		MESSAGE_TYPE_KEEP_ALIVE = 0xFF
	};

	/**
	 * \brief The different channel numbers.
	 */
	enum channel_number_type
	{
		CHANNEL_NUMBER_0 = 0,
		CHANNEL_NUMBER_1 = 1,
		CHANNEL_NUMBER_2 = 2,
		CHANNEL_NUMBER_3 = 3,
		CHANNEL_NUMBER_4 = 4,
		CHANNEL_NUMBER_5 = 5,
		CHANNEL_NUMBER_6 = 6,
		CHANNEL_NUMBER_7 = 7,
		CHANNEL_NUMBER_8 = 8,
		CHANNEL_NUMBER_9 = 9,
		CHANNEL_NUMBER_10 = 10,
		CHANNEL_NUMBER_11 = 11,
		CHANNEL_NUMBER_12 = 12,
		CHANNEL_NUMBER_13 = 13,
		CHANNEL_NUMBER_14 = 14,
		CHANNEL_NUMBER_15 = 15
	};

	/**
	 * \brief The endpoint type type.
	 */
	enum endpoint_type_type
	{
		ENDPOINT_TYPE_IPV4 = 0x04,
		ENDPOINT_TYPE_IPV6 = 0x06
	};

	/**
	 * \brief The cipher algorithm type.
	 */
	DECLARE_VALUE_TYPE(cipher_algorithm_type);
	DECLARE_VALUE(cipher_algorithm_type, 0x00, CIPHER_ALGORITHM_UNSUPPORTED, "Unsupported cipher algorithm");
	DECLARE_VALUE(cipher_algorithm_type, 0x01, CIPHER_ALGORITHM_AES256_CBC, "aes256-cbc");
	
	/**
	 * \brief The cipher algorithm list type.
	 */
	typedef std::vector<cipher_algorithm_type> cipher_algorithm_list_type;

	/**
	 * \brief Get the default cipher capabilities.
	 * \return The default cipher capabilities.
	 */
	inline cipher_algorithm_list_type get_default_cipher_capabilities()
	{
		cipher_algorithm_list_type result;

		result.push_back(CIPHER_ALGORITHM_AES256_CBC);

		return result;
	}

	/**
	 * \brief The message digest algorithm type.
	 */
	DECLARE_VALUE_TYPE(message_digest_algorithm_type);
	DECLARE_VALUE(message_digest_algorithm_type, 0x00, MESSAGE_DIGEST_ALGORITHM_UNSUPPORTED, "Unsupported message digest algorithm");
	DECLARE_VALUE(message_digest_algorithm_type, 0x01, MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256, "sha256");
	DECLARE_VALUE(message_digest_algorithm_type, 0x02, MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256_128, "sha256-128");
	DECLARE_VALUE(message_digest_algorithm_type, 0x03, MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1, "sha1");
	DECLARE_VALUE(message_digest_algorithm_type, 0x04, MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1_96, "sha1-96");
	DECLARE_VALUE(message_digest_algorithm_type, 0x05, MESSAGE_DIGEST_ALGORITHM_NONE, "disabled");

	/**
	 * \brief The message digest algorithm list type.
	 */
	typedef std::vector<message_digest_algorithm_type> message_digest_algorithm_list_type;

	/**
	 * \brief Get the default message digest capabilities.
	 * \return The default message digest capabilities.
	 */
	inline message_digest_algorithm_list_type get_default_message_digest_capabilities()
	{
		message_digest_algorithm_list_type result;

		result.push_back(MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1_96);
		result.push_back(MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1);
		result.push_back(MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256_128);
		result.push_back(MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256);

		return result;
	}

	/**
	 * \brief Get the cipher algorithm associated with the specified integral value.
	 * \param cipher_algorithm The cipher algorithm.
	 * \return The associated cipher algorithm.
	 *
	 * If cipher_algorithm is not supported, a std::runtime_error is thrown.
	 */
	inline cryptoplus::cipher::cipher_algorithm to_cipher_algorithm(cipher_algorithm_type cipher_algorithm)
	{
		switch (cipher_algorithm)
		{
			case CIPHER_ALGORITHM_UNSUPPORTED:
				throw std::runtime_error("Unsupported cipher algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(cipher_algorithm)));
			case CIPHER_ALGORITHM_AES256_CBC:
				return cryptoplus::cipher::cipher_algorithm(NID_aes_256_cbc);
		}

		throw std::runtime_error("Unknown cipher algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(cipher_algorithm)));
	}

	/**
	 * \brief Get the message digest algorithm associated with the specified integral value.
	 * \param message_digest_algorithm The message digest algorithm.
	 * \return The associated message digest algorithm.
	 *
	 * If message_digest_algorithm is not supported, a std::runtime_error is thrown.
	 *
	 * If message_digest_algorithm is MESSAGE_DIGEST_ALGORITHM_NONE, a std::runtime_error is thrown.
	 */
	inline boost::optional<cryptoplus::hash::message_digest_algorithm> to_message_digest_algorithm(message_digest_algorithm_type message_digest_algorithm)
	{
		switch (message_digest_algorithm)
		{
			case MESSAGE_DIGEST_ALGORITHM_UNSUPPORTED:
				throw std::runtime_error("Unsupported message digest algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(message_digest_algorithm)));
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256:
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256_128:
				return cryptoplus::hash::message_digest_algorithm(NID_sha256);
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1:
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1_96:
				return cryptoplus::hash::message_digest_algorithm(NID_sha1);
			case MESSAGE_DIGEST_ALGORITHM_NONE:
				return boost::none;
		}

		throw std::runtime_error("Unknown message digest algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(message_digest_algorithm)));
	}

	/**
	 * \brief Get the size of the HMAC for the specified message digest algorithm.
	 * \param message_digest_algorithm The message digest algorithm.
	 * \return The size of the HMAC for the specified message digest algorithm.
	 *
	 * If message_digest_algorithm is not supported, a std::runtime_error is thrown.
	 */
	inline size_t get_message_digest_algorithm_hmac_size(message_digest_algorithm_type message_digest_algorithm)
	{
		switch (message_digest_algorithm)
		{
			case MESSAGE_DIGEST_ALGORITHM_UNSUPPORTED:
				throw std::runtime_error("Unsupported message digest algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(message_digest_algorithm)));
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256:
				return 32;
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA256_128:
				return 16;
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1:
				return 20;
			case MESSAGE_DIGEST_ALGORITHM_HMAC_SHA1_96:
				return 12;
			case MESSAGE_DIGEST_ALGORITHM_NONE:
				return 0;
		}

		throw std::runtime_error("Unknown message digest algorithm value: " + boost::lexical_cast<std::string>(static_cast<int>(message_digest_algorithm)));
	}

	/**
	 * \brief The certificate digest algorithm.
	 */
	const int CERTIFICATE_DIGEST_ALGORITHM = NID_sha256;

	/**
	 * \brief The session keep-alive period.
	 */
	const boost::posix_time::time_duration SESSION_KEEP_ALIVE_PERIOD = boost::posix_time::seconds(10);

	/**
	 * \brief The session timeout.
	 */
	const boost::posix_time::time_duration SESSION_TIMEOUT = SESSION_KEEP_ALIVE_PERIOD * 3;

	/**
	 * \brief Check if a message type is a DATA type message.
	 * \param type The message type.
	 * \return true if the message type is one from MESSAGE_TYPE_DATA_0 to MESSAGE_TYPE_DATA_15.
	 */
	inline bool is_data_message_type(message_type type)
	{
		return (type >= MESSAGE_TYPE_DATA_0) && (type <= MESSAGE_TYPE_DATA_15);
	}

	/**
	 * \brief Convert a DATA message type to a channel number.
	 * \param type The message type. Must be one from MESSAGE_TYPE_DATA_0 to MESSAGE_TYPE_DATA_15.
	 * \return The channel number.
	 */
	channel_number_type to_channel_number(message_type type);

	/**
	 * \brief Convert a channel number to a DATA message type.
	 * \param channel_number The channel number.
	 * \return The DATA message type.
	 */
	message_type to_data_message_type(channel_number_type channel_number);

	/**
	 * \brief Gives a hash for a certificate.
	 * \param buf The output buffer.
	 * \param buflen The output buffer length.
	 * \param cert The certificate.
	 */
	void get_certificate_hash(void* buf, size_t buflen, cryptoplus::x509::certificate cert);

	/**
	 * \brief Gives a hash for a certificate.
	 * \param cert The certificate.
	 */
	hash_type get_certificate_hash(cryptoplus::x509::certificate cert);
}

#endif /* FSCP_CONSTANTS_HPP */

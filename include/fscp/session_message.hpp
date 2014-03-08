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
 * \file session_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session message class.
 */

#ifndef FSCP_SESSION_MESSAGE_HPP
#define FSCP_SESSION_MESSAGE_HPP

#include <boost/asio.hpp>

#include "message.hpp"
#include "buffer_tools.hpp"
#include "constants.hpp"

#include <stdint.h>
#include <cstring>

#include <cryptoplus/pkey/pkey.hpp>

namespace fscp
{
	/**
	 * \brief A session message class.
	 */
	class session_message : public message
	{
		public:

			/**
			 * \brief Write a session message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param calg The cipher algorithm type.
			 * \param pub_key The public key.
			 * \param pub_key_len The public key length.
			 * \param salt The salt.
			 * \param salt_len The salt length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \param sig_key The private key to use to sign the ciphertext.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const host_identifier_type& host_identifier, elliptic_curve_type ec, key_derivation_algorithm_type kd, cipher_algorithm_type calg, const void* pub_key, size_t pub_key_len, const void* salt, size_t salt_len, const void* nonce_prefix, size_t nonce_prefix_len, cryptoplus::pkey::pkey sig_key);

			/**
			 * \brief Create a session_message from a message.
			 * \param message The message.
			 */
			session_message(const message& message);

			/**
			 * \brief Get the session number.
			 * \return The session number.
			 */
			session_number_type session_number() const;

			/**
			 * \brief Get the host identifier.
			 * \return The host identifier.
			 */
			host_identifier_type host_identifier() const;

			/**
			 * \brief Get the elliptic curve.
			 * \return The elliptic curve.
			 */
			elliptic_curve_type elliptic_curve() const;

			/**
			 * \brief Get the key derivation algorithm.
			 * \return The key derivation algorithm.
			 */
			key_derivation_algorithm_type key_derivation_algorithm() const;

			/**
			 * \brief Get the cipher algorithm.
			 * \return The cipher algorithm.
			 */
			cipher_algorithm_type cipher_algorithm() const;

			/**
			 * \brief Get the public key.
			 * \return The public key.
			 */
			const uint8_t* public_key() const;

			/**
			 * \brief Get the public key size.
			 * \return The public key size.
			 */
			size_t public_key_size() const;

			/**
			 * \brief Get the salt.
			 * \return The salt.
			 */
			const uint8_t* salt() const;

			/**
			 * \brief Get the salt size.
			 * \return The salt size.
			 */
			size_t salt_size() const;

			/**
			 * \brief Get the nonce prefix.
			 * \return The nonce prefix.
			 */
			const uint8_t* nonce_prefix() const;

			/**
			 * \brief Get the nonce prefix size.
			 * \return The nonce prefix size.
			 */
			size_t nonce_prefix_size() const;

			/**
			 * \brief Get the header size, without the signature.
			 * \return The header size, without the signature.
			 */
			size_t header_size() const;

			/**
			 * \brief Get the header signature.
			 * \return The header signature.
			 */
			const uint8_t* header_signature() const;

			/**
			 * \brief Get the header signature size.
			 * \return The header signature size.
			 */
			size_t header_signature_size() const;

			/**
			 * \brief Check if the signature matches with a given public key.
			 * \param key The public key to use.
			 * \warning If the check fails, an exception is thrown.
			 */
			void check_signature(cryptoplus::pkey::pkey key) const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t);
	};

	inline session_number_type session_message::session_number() const
	{
		return ntohl(buffer_tools::get<session_number_type>(payload(), 0));
	}

	inline host_identifier_type session_message::host_identifier() const
	{
		host_identifier_type result;

		std::copy(payload() + sizeof(session_number_type), payload() + sizeof(session_number_type) + result.size(), result.begin());

		return result;
	}

	inline elliptic_curve_type session_message::elliptic_curve() const
	{
		return buffer_tools::get<uint8_t>(payload(), sizeof(session_number_type) + host_identifier_type::static_size);
	}

	inline key_derivation_algorithm_type session_message::key_derivation_algorithm() const
	{
		return buffer_tools::get<uint8_t>(payload(), sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t));
	}

	inline cipher_algorithm_type session_message::cipher_algorithm() const
	{
		return buffer_tools::get<uint8_t>(payload(), sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 2);
	}

	inline const uint8_t* session_message::public_key() const
	{
		return payload() + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t);
	}

	inline size_t session_message::public_key_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1));
	}

	inline const uint8_t* session_message::salt() const
	{
		return payload() + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + public_key_size() + sizeof(uint16_t);
	}

	inline size_t session_message::salt_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + public_key_size()));
	}

	inline const uint8_t* session_message::nonce_prefix() const
	{
		return payload() + sizeof(session_number_type) + host_identifier_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + public_key_size() + sizeof(uint16_t) + salt_size() + sizeof(uint16_t);
	}

	inline size_t session_message::nonce_prefix_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + public_key_size() + sizeof(uint16_t) + salt_size()));
	}

	inline size_t session_message::header_size() const
	{
		return sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 3 + 1 + sizeof(uint16_t) + public_key_size() + sizeof(uint16_t) + salt_size() + sizeof(uint16_t) + nonce_prefix_size();
	}

	inline const uint8_t* session_message::header_signature() const
	{
		return payload() + header_size() + sizeof(uint16_t);
	}

	inline size_t session_message::header_signature_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), header_size()));
	}
}

#endif /* FSCP_SESSION_MESSAGE_HPP */

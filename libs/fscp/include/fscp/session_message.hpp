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
			 * \param cs The cipher suite.
			 * \param ec The elliptic curve.
			 * \param pub_key The public key.
			 * \param pub_key_len The public key length.
			 * \param sig_key The private key to use to sign the ciphertext.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const host_identifier_type& host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len, cryptoplus::pkey::pkey sig_key);

			/**
			 * \brief Write a session message to a buffer using a pre-shared key.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param cs The cipher suite.
			 * \param ec The elliptic curve.
			 * \param pub_key The public key.
			 * \param pub_key_len The public key length.
			 * \param pre_shared_key The pre-shared key used to sign the session message.
			 * \param pre_shared_key_len The pre-shared key length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const host_identifier_type& host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len, const void* pre_shared_key, size_t pre_shared_key_len);

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
			 * \brief Get the cipher suite.
			 * \return The cipher suite.
			 */
			cipher_suite_type cipher_suite() const;

			/**
			 * \brief Get the elliptic curve.
			 * \return The elliptic curve.
			 */
			elliptic_curve_type elliptic_curve() const;

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
			 * \return If the check fails, false is returned.
			 */
			bool check_signature(cryptoplus::pkey::pkey key) const;

			/**
			* \brief Check if the signature matches with a given public key.
			* \param pre_shared_key The pre-shared key used to sign the session message.
			* \param pre_shared_key_len The pre-shared key length.
			* \return If the check fails, false is returned.
			*/
			bool check_signature(const void* pre_shared_key, size_t pre_shared_key_len) const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 4 + sizeof(uint16_t);

			/**
			 * \brief Write an unsigned session message to a buffer.
			 * \param payload The buffer to write to.
			 * \param payload_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param cs The cipher suite.
			 * \param ec The elliptic curve.
			 * \param pub_key The public key.
			 * \param pub_key_len The public key length.
			 * \return The count of bytes written.
			 */
			static size_t write_unsigned(uint8_t* payload, size_t payload_len, session_number_type session_number, const host_identifier_type& host_identifier, cipher_suite_type cs, elliptic_curve_type ec, const void* pub_key, size_t pub_key_len);
	};

	inline session_number_type session_message::session_number() const
	{
		return ntohl(buffer_tools::get<session_number_type>(payload(), 0));
	}

	inline host_identifier_type session_message::host_identifier() const
	{
		host_identifier_type result;

		std::copy(payload() + sizeof(session_number_type), payload() + sizeof(session_number_type) + result.data.size(), result.data.begin());

		return result;
	}

	inline cipher_suite_type session_message::cipher_suite() const
	{
		return buffer_tools::get<uint8_t>(payload(), sizeof(session_number_type) + host_identifier_type::data_type::static_size);
	}

	inline elliptic_curve_type session_message::elliptic_curve() const
	{
		return buffer_tools::get<uint8_t>(payload(), sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t));
	}

	inline const uint8_t* session_message::public_key() const
	{
		return payload() + sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 2 + 2 + sizeof(uint16_t);
	}

	inline size_t session_message::public_key_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 2 + 2));
	}

	inline size_t session_message::header_size() const
	{
		return sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint8_t) * 2 + 2 + sizeof(uint16_t) + public_key_size();
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

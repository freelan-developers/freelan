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
 * \file session_request_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session request message class.
 */

#ifndef FSCP_SESSION_REQUEST_MESSAGE_HPP
#define FSCP_SESSION_REQUEST_MESSAGE_HPP

#include "message.hpp"

#include "buffer_tools.hpp"
#include "constants.hpp"

#include <stdint.h>
#include <cstring>

#include <boost/asio.hpp>

namespace fscp
{
	/**
	 * \brief A session request message class.
	 */
	class session_request_message : public message
	{
		public:

			/**
			 * \brief Write a session request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param cs_cap The cipher suite capabilities.
			 * \param ec_cap The elliptic curve capabilities.
			 * \param sig_key The private key to use to sign the ciphertext.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const host_identifier_type& host_identifier, const cipher_suite_list_type& cs_cap, const elliptic_curve_list_type& ec_cap, cryptoplus::pkey::pkey sig_key);

			/**
			 * \brief Write a session request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param cs_cap The cipher suite capabilities.
			 * \param ec_cap The elliptic curve capabilities.
			 * \param pre_shared_key The pre-shared key used to sign the session message.
			 * \param pre_shared_key_len The pre-shared key length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const host_identifier_type& host_identifier, const cipher_suite_list_type& cs_cap, const elliptic_curve_list_type& ec_cap, const void* pre_shared_key, size_t pre_shared_key_len);

			/**
			 * \brief Create a session_request_message from a message.
			 * \param message The message.
			 */
			session_request_message(const message& message);

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
			 * \brief Get the cipher suite capabilities.
			 * \return The cipher suite capabilities.
			 */
			cipher_suite_list_type cipher_suite_capabilities() const;

			/**
			 * \brief Get the elliptic curve capabilities.
			 * \return The elliptic curve capabilities.
			 */
			elliptic_curve_list_type elliptic_curve_capabilities() const;

			/**
			 * \brief Get the cipher suite capabilities size.
			 * \return The cipher suite capabilities size.
			 */
			size_t cipher_suite_capabilities_size() const;

			/**
			 * \brief Get the elliptic curve capabilities size.
			 * \return The elliptic curve capabilities size.
			 */
			size_t elliptic_curve_capabilities_size() const;

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
			static const size_t MIN_BODY_LENGTH = sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint16_t) * 2;

			/**
			 * \brief Write an unsigned session request message to a buffer.
			 * \param payload The buffer to write to.
			 * \param payload_len The length of buf.
			 * \param session_number The session number.
			 * \param host_identifier The host identifier.
			 * \param cs_cap The cipher suite capabilities.
			 * \param ec_cap The elliptic curve capabilities.
			 * \return The count of bytes written.
			 */
			static size_t write_unsigned(uint8_t* payload, size_t payload_len, session_number_type session_number, const host_identifier_type& host_identifier, const cipher_suite_list_type& cs_cap, const elliptic_curve_list_type& ec_cap);
	};

	inline session_number_type session_request_message::session_number() const
	{
		return ntohl(buffer_tools::get<session_number_type>(payload(), 0));
	}

	inline host_identifier_type session_request_message::host_identifier() const
	{
		host_identifier_type result;

		std::copy(payload() + sizeof(session_number_type), payload() + sizeof(session_number_type) + result.data.size(), result.data.begin());

		return result;
	}

	inline size_t session_request_message::cipher_suite_capabilities_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + host_identifier_type::data_type::static_size));
	}

	inline size_t session_request_message::elliptic_curve_capabilities_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint16_t) + cipher_suite_capabilities_size()));
	}

	inline size_t session_request_message::header_size() const
	{
		return sizeof(session_number_type) + host_identifier_type::data_type::static_size + sizeof(uint16_t) + cipher_suite_capabilities_size() + sizeof(uint16_t) + elliptic_curve_capabilities_size();
	}

	inline const uint8_t* session_request_message::header_signature() const
	{
		return payload() + header_size() + sizeof(uint16_t);
	}

	inline size_t session_request_message::header_signature_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), header_size()));
	}
}

#endif /* FSCP_SESSION_REQUEST_MESSAGE_HPP */

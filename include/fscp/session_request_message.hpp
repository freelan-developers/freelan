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
 * \file session_request_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session request message class.
 */

#ifndef FSCP_SESSION_REQUEST_MESSAGE_HPP
#define FSCP_SESSION_REQUEST_MESSAGE_HPP

#include "session_message.hpp"

namespace fscp
{
	/**
	 * \brief A session request message class.
	 */
	class session_request_message : private session_message
	{
		public:

			/**
			 * \brief Write a session request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param cleartext The cleartext.
			 * \param cleartext_len The cleartext length.
			 * \param enc_key The public key to use to cipher the cleartext.
			 * \param sig_key The private key to use to sign the ciphertext.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, const void* cleartext, size_t cleartext_len, cryptoplus::pkey::pkey enc_key, cryptoplus::pkey::pkey sig_key);

			/**
			 * \brief Create a session_request_message from a message.
			 * \param message The message.
			 * \param pkey_size The private key size.
			 */
			session_request_message(const message& message, size_t pkey_size);

			using session_message::ciphertext;
			using session_message::ciphertext_size;
			using session_message::ciphertext_signature;
			using session_message::ciphertext_signature_size;
			using session_message::check_signature;
			using session_message::get_cleartext;
	};

	inline size_t session_request_message::write(void* buf, size_t buf_len, const void* cleartext, size_t cleartext_len, cryptoplus::pkey::pkey enc_key, cryptoplus::pkey::pkey sig_key)
	{
		return _write(buf, buf_len, cleartext, cleartext_len, enc_key, sig_key, MESSAGE_TYPE_SESSION_REQUEST);
	}

}

#endif /* FSCP_SESSION_REQUEST_MESSAGE_HPP */

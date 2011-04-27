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

#include "message.hpp"

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
			 * \param ciphertext The ciphertext.
			 * \param ciphertext_len The ciphertext length.
			 * \param ciphertext_signature The ciphertext signature.
			 * \param ciphertext_signature_len The ciphertext signature length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, const void* ciphertext, size_t ciphertext_len, const void* ciphertext_signature, size_t ciphertext_signature_len);

			/**
			 * \brief Write a session message to a buffer.
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
			 * \brief Create a session_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			session_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a session_message from a message.
			 * \param message The message.
			 */
			session_message(const message& message);

			/**
			 * \brief Get the ciphertext.
			 * \return The ciphertext.
			 */
			const uint8_t* ciphertext() const;

			/**
			 * \brief Get the ciphertext size.
			 * \return The ciphertext size.
			 */
			size_t ciphertext_size() const;

			/**
			 * \brief Get the ciphertext signature.
			 * \return The ciphertext signature.
			 */
			const uint8_t* ciphertext_signature() const;

			/**
			 * \brief Get the ciphertext signature size.
			 * \return The ciphertext signature size.
			 */
			size_t ciphertext_signature_size() const;

			/**
			 * \brief Check if the signature matches with a given public key.
			 * \param key The public key to use.
			 * \warning If the check fails, an exception is thrown.
			 */
			void check_signature(cryptoplus::pkey::pkey key) const;

			/**
			 * \brief Get the clear text data, using a given private key.
			 * \param buf The buffer that must receive the data. If buf is NULL, the function returns the expected size of buf.
			 * \param buf_len The length of buf.
			 * \param key The private key to use.
			 * \return The count of bytes deciphered.
			 */
			size_t get_cleartext(void* buf, size_t buf_len, cryptoplus::pkey::pkey key) const;

			/**
			 * \brief Get the clear text data, using a given private key.
			 * \param key The private key to use.
			 * \return The clear text data.
			 */
			template <typename T>
			std::vector<T> get_cleartext(cryptoplus::pkey::pkey key) const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = 2 * sizeof(uint16_t);

		private:

			void check_format() const;
	};

	inline const uint8_t* session_message::ciphertext() const
	{
		return payload() + sizeof(uint16_t);
	}

	inline size_t session_message::ciphertext_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), 0));
	}

	inline const uint8_t* session_message::ciphertext_signature() const
	{
		return payload() + 2 * sizeof(uint16_t) + ciphertext_size();
	}

	inline size_t session_message::ciphertext_signature_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(uint16_t) + ciphertext_size()));
	}
	
	template <typename T>
	inline std::vector<T> session_message::get_cleartext(cryptoplus::pkey::pkey key) const
	{
		std::vector<T> result(get_cleartext(NULL, 0, key));

		result.resize(get_cleartext(&result[0], result.size(), key));

		return result;
	}
}

#endif /* FSCP_SESSION_MESSAGE_HPP */

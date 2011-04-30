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
 * \file data_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A data message class.
 */

#ifndef FSCP_DATA_MESSAGE_HPP
#define FSCP_DATA_MESSAGE_HPP

#include "message.hpp"

#include <cryptoplus/pkey/pkey.hpp>

namespace fscp
{
	/**
	 * \brief A data message class.
	 */
	class data_message : public message
	{
		public:

			/**
			 * \brief The sequence number type.
			 */
			typedef uint32_t sequence_number_type;

			/**
			 * \brief Write a data message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sequence_number The sequence number.
			 * \param data The data.
			 * \param data_len The data length.
			 * \param sig_key The signature key.
			 * \param sig_key_len The signature key length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param iv The initialization vector.
			 * \param iv_len The initialization vector length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, sequence_number_type sequence_number, const void* data, size_t data_len, const void* sig_key, size_t sig_key_len, const void* enc_len, size_t enc_key_len);

			/**
			 * \brief Create a data_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			data_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a data_message from a message.
			 * \param message The message.
			 */
			data_message(const message& message);

			/**
			 * \brief Get the sequence number.
			 * \return The sequence number.
			 */
			sequence_number_type sequence_number() const;

			/**
			 * \brief Get the data.
			 * \return The data.
			 */
			const uint8_t* data() const;

			/**
			 * \brief Get the data size.
			 * \return The data size.
			 */
			size_t data_size() const;

			/**
			 * \brief Get the hmac signature.
			 * \return The hmac signature.
			 */
			const uint8_t* hmac() const;

			/**
			 * \brief Get the hmac signature size.
			 * \return The hmac signature size.
			 */
			size_t hmac_size() const;

			/**
			 * \brief Check if the signature matches with a given signature key.
			 * \param sig_key The signature key.
			 * \param sig_key_len The signature key length.
			 * \warning If the check fails, an exception is thrown.
			 */
			void check_signature(const void* sig_key, size_t sig_key_len) const;

			/**
			 * \brief Get the clear text data, using a given encryption key.
			 * \param buf The buffer that must receive the data. If buf is NULL, the function returns the expected size of buf.
			 * \param buf_len The length of buf.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \return The count of bytes deciphered.
			 */
			size_t get_cleartext(void* buf, size_t buf_len, const void* enc_key, size_t enc_key_len) const;

			/**
			 * \brief Get the clear text data, using a given encryption key.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \return The clear text data.
			 */
			template <typename T>
			std::vector<T> get_cleartext(const void* enc_key, size_t enc_key_len) const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(sequence_number_type) + sizeof(uint16_t);

			/**
			 * \brief The HMAC size.
			 */
			static const size_t HMAC_SIZE = 32;

		private:

			void check_format() const;
	};

	inline data_message::sequence_number_type data_message::sequence_number() const
	{
		return buffer_tools::get<sequence_number_type>(payload(), 0);
	}

	inline const uint8_t* data_message::data() const
	{
		return payload() + sizeof(sequence_number_type) + sizeof(uint16_t);
	}

	inline size_t data_message::data_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(sequence_number_type)));
	}

	inline const uint8_t* data_message::hmac() const
	{
		return payload() + sizeof(sequence_number_type) + sizeof(uint16_t) + data_size();
	}

	inline size_t data_message::hmac_size() const
	{
		return HMAC_SIZE;
	}

	template <typename T>
	inline std::vector<T> data_message::get_cleartext(const void* enc_key, size_t enc_key_len) const
	{
		std::vector<T> result(get_cleartext(NULL, 0, enc_key, enc_key_len));

		result.resize(get_cleartext(&result[0], result.size(), enc_key, enc_key_len));

		return result;
	}
}

#endif /* FSCP_DATA_MESSAGE_HPP */

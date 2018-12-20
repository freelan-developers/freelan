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
 * \file data_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A data message class.
 */

#ifndef FSCP_DATA_MESSAGE_HPP
#define FSCP_DATA_MESSAGE_HPP

#include "message.hpp"

#include "constants.hpp"

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
			 * \brief The cipher algorithm type.
			 */
			typedef cryptoplus::cipher::cipher_algorithm calg_t;

			/**
			 * \brief Write a data message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param channel_number The channel number.
			 * \param sequence_number The sequence number.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param cleartext The cleartext data.
			 * \param cleartext_len The data length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, channel_number_type channel_number, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, const void* cleartext, size_t cleartext_len, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len);

			/**
			 * \brief Write a contact-request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sequence_number The sequence number.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param hash_list The hash list.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \return The count of bytes written.
			 */
			static size_t write_contact_request(void* buf, size_t buf_len, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, const hash_list_type& hash_list, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len);

			/**
			 * \brief Write a contact message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sequence_number The sequence number.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param contact_map The contact map.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \return The count of bytes written.
			 */
			static size_t write_contact(void* buf, size_t buf_len, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, const contact_map_type& contact_map, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len);

			/**
			 * \brief Write a keep-alive message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sequence_number The sequence number.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param random_len The length of the random content to send.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \return The count of bytes written.
			 */
			static size_t write_keep_alive(void* buf, size_t buf_len, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, size_t random_len, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len);

			/**
			 * \brief Parse the hash list.
			 * \param buf The buffer to parse.
			 * \param buflen The length of the buffer to parse.
			 * \return The hash list.
			 */
			static hash_list_type parse_hash_list(const void* buf, size_t buflen);

			/**
			 * \brief Parse the contact map.
			 * \param buf The buffer to parse.
			 * \param buflen The length of the buffer to parse.
			 * \return The contact map.
			 */
			static contact_map_type parse_contact_map(const void* buf, size_t buflen);

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
			 * \brief Get the tag.
			 * \return The tag.
			 */
			const uint8_t* tag() const;

			/**
			 * \brief Get the tag size.
			 * \return The tag size.
			 */
			size_t tag_size() const;

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
			 * \brief Get the clear text data, using a given encryption key.
			 * \param buf The buffer that must receive the data. If buf is NULL, the function returns the expected size of buf.
			 * \param buf_len The length of buf.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \return The count of bytes deciphered.
			 */
			size_t get_cleartext(void* buf, size_t buf_len, data_message::calg_t cipher_algorithm, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len) const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(sequence_number_type) + GCM_TAG_LENGTH + sizeof(uint16_t);

			/**
			 * \brief Write a data message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sequence_number The sequence number.
			 * \param cipher_algorithm The cipher algorithm to use.
			 * \param cleartext The cleartext data.
			 * \param cleartext_len The data length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 * \param type The message type.
			 * \return The count of bytes written.
			 */
			static size_t raw_write(void* buf, size_t buf_len, sequence_number_type sequence_number, data_message::calg_t cipher_algorithm, const void* cleartext, size_t cleartext_len, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len, message_type type);

		private:

			void check_format() const;
	};

	inline sequence_number_type data_message::sequence_number() const
	{
		return ntohl(buffer_tools::get<sequence_number_type>(payload(), 0));
	}

	inline const uint8_t* data_message::tag() const
	{
		return payload() + sizeof(sequence_number_type);
	}

	inline size_t data_message::tag_size() const
	{
		return GCM_TAG_LENGTH;
	}

	inline const uint8_t* data_message::ciphertext() const
	{
		return payload() + sizeof(sequence_number_type) + tag_size() + sizeof(uint16_t);
	}

	inline size_t data_message::ciphertext_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(sequence_number_type) + tag_size()));
	}
}

#endif /* FSCP_DATA_MESSAGE_HPP */

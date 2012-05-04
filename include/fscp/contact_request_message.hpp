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
 * \file contact_request_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A contact request message class.
 */

#ifndef FSCP_CONTACT_REQUEST_MESSAGE_HPP
#define FSCP_CONTACT_REQUEST_MESSAGE_HPP

#include "data_message.hpp"

namespace fscp
{
	/**
	 * \brief A contact request message class.
	 */
	class contact_request_message : public data_message
	{
		public:

			/**
			 * \brief Write a keep-alive message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param sequence_number The sequence number.
			 * \param cert_begin An iterator to the first certificate to request.
			 * \param cert_end An iterator past the last certificate to request.
			 * \param seal_key The seal key.
			 * \param seal_key_len The seal key length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \return The count of bytes written.
			 */
			template <typename CertIterator>
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, sequence_number_type sequence_number, CertIterator cert_begin, CertIterator cert_end, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
			{
				const cryptoplus::hash::message_digest_algorithm certificate_digest_algorithm(CERTIFICATE_DIGEST_ALGORITHM);

				const size_t hash_size = certificate_digest_algorithm.result_size();
				
				std::vector<uint8_t> cleartext;
				cleartext.reserve(hash_size * std::distance(cert_begin, cert_end));

				for (CertIterator it = cert_begin; it != cert_end; ++it)
				{
					ptrdiff_t dist = std::distance(cert_begin, it);

					get_certificate_hash(&cleartext[dist * hash_size], cleartext.size() - dist * hash_size, *it);
				}

				return data_message::raw_write(buf, buf_len, session_number, sequence_number, &cleartext[0], cleartext.size(), seal_key, seal_key_len, enc_key, enc_key_len, MESSAGE_TYPE_CONTACT_REQUEST);
			}

			/**
			 * \brief Create a contact request message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			contact_request_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a data_message from a data_message.
			 * \param data_message The message.
			 */
			contact_request_message(const data_message& message);
	};
}

#endif /* FSCP_CONTACT_REQUEST_MESSAGE_HPP */

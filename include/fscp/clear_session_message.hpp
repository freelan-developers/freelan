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
 * \file clear_session_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session message class.
 */

#ifndef FSCP_CLEAR_SESSION_MESSAGE_HPP
#define FSCP_CLEAR_SESSION_MESSAGE_HPP

#include "buffer_tools.hpp"
#include "constants.hpp"

#include <stdint.h>
#include <cstring>

#include <boost/array.hpp>
#include <boost/asio.hpp>

namespace fscp
{
	/**
	 * \brief A clear session message class.
	 */
	class clear_session_message
	{
		public:

			/**
			 * \brief The key length.
			 */
			static const size_t KEY_LENGTH = 32;

			/**
			 * \brief Write a session message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param seal_key The seal key.
			 * \param seal_key_len The seal key length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len);

			/**
			 * \brief Write a session message to a buffer.
			 * \param session_number The session number.
			 * \param seal_key The seal key.
			 * \param seal_key_len The seal key length.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \return The buffer.
			 */
			template <typename T>
			static std::vector<T> write(session_number_type session_number, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len);

			/**
			 * \brief Create a clear_session_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			clear_session_message(const void* buf, size_t buf_len);

			/**
			 * \brief Get the session number.
			 * \return The session number.
			 */
			session_number_type session_number() const;

			/**
			 * \brief Get the seal key.
			 * \return The seal key.
			 */
			const uint8_t* seal_key() const;

			/**
			 * \brief Get the seal key size.
			 * \return The seal key size.
			 */
			size_t seal_key_size() const;

			/**
			 * \brief Get the encryption key.
			 * \return The encryption key.
			 */
			const uint8_t* encryption_key() const;

			/**
			 * \brief Get the encryption key size.
			 * \return The encryption key size.
			 */
			size_t encryption_key_size() const;

		protected:

			/**
			 * \brief The length of the body.
			 */
			static const size_t BODY_LENGTH = sizeof(session_number_type) + 2 * KEY_LENGTH + 2 * sizeof(uint16_t);

			/**
			 * \brief The data.
			 * \return The data buffer.
			 */
			const uint8_t* data() const;

		private:

			const void* m_data;
	};

	template <typename T>
	inline std::vector<T> clear_session_message::write(session_number_type _session_number, const void* seal_key, size_t seal_key_len, const void* enc_key, size_t enc_key_len)
	{
		std::vector<T> result(BODY_LENGTH);

		result.resize(write(&result[0], result.size(), _session_number, seal_key, seal_key_len, enc_key, enc_key_len));

		return result;
	}

	inline session_number_type clear_session_message::session_number() const
	{
		return ntohl(buffer_tools::get<session_number_type>(data(), 0));
	}

	inline const uint8_t* clear_session_message::seal_key() const
	{
		return data() + sizeof(session_number_type) + sizeof(uint16_t);
	}

	inline size_t clear_session_message::seal_key_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(data(), sizeof(session_number_type)));
	}

	inline const uint8_t* clear_session_message::encryption_key() const
	{
		return data() + sizeof(session_number_type) + sizeof(uint16_t) + seal_key_size() + sizeof(uint16_t);
	}

	inline size_t clear_session_message::encryption_key_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(data(), sizeof(session_number_type) + sizeof(uint16_t) + seal_key_size()));
	}

	inline const uint8_t* clear_session_message::data() const
	{
		return static_cast<const uint8_t*>(m_data);
	}
}

#endif /* FSCP_CLEAR_SESSION_MESSAGE_HPP */

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
 * \file clear_session_request_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A session request message class.
 */

#ifndef FSCP_CLEAR_SESSION_REQUEST_MESSAGE_HPP
#define FSCP_CLEAR_SESSION_REQUEST_MESSAGE_HPP

#include "buffer_tools.hpp"
#include "constants.hpp"

#include <stdint.h>
#include <cstring>

#include <boost/asio.hpp>

namespace fscp
{
	/**
	 * \brief A clear session request message class.
	 */
	class clear_session_request_message
	{
		public:

			/**
			 * \brief Write a session request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param session_number The session number.
			 * \param challenge The challenge.
			 * \param cipher_capabilities The cipher algorithm capabilities.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, session_number_type session_number, const challenge_type& challenge, const cipher_algorithm_list_type& cipher_capabilities);

			/**
			 * \brief Write a session request message to a buffer.
			 * \param session_number The session number.
			 * \param challenge The challenge.
			 * \param cipher_capabilities The cipher algorithm capabilities.
			 * \return The buffer.
			 */
			template <typename T>
			static std::vector<T> write(session_number_type session_number, const challenge_type& challenge, const cipher_algorithm_list_type& cipher_capabilities);

			/**
			 * \brief Create a clear_session_request_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			clear_session_request_message(const void* buf, size_t buf_len);

			/**
			 * \brief Get the session number.
			 * \return The session number.
			 */
			session_number_type session_number() const;

			/**
			 * \brief Get the challenge.
			 * \return The challenge.
			 */
			challenge_type challenge() const;

			/**
			 * \brief Get the cipher capabilities.
			 * \return The cipher capabilities.
			 */
			cipher_algorithm_list_type cipher_capabilities() const;

			/**
			 * \brief Get the cipher capabilities size.
			 * \return The cipher capabilities size.
			 */
			size_t cipher_capabilities_size() const;

		protected:

			/**
			 * \brief The min length of the body.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(session_number_type) + challenge_type::static_size + sizeof(uint16_t);

			/**
			 * \brief The data.
			 * \return The data buffer.
			 */
			const uint8_t* data() const;

		private:

			const void* m_data;
	};

	template <typename T>
	inline std::vector<T> clear_session_request_message::write(session_number_type _session_number, const challenge_type& _challenge, const cipher_algorithm_list_type& _cipher_capabilities)
	{
		std::vector<T> result(MIN_BODY_LENGTH + _cipher_capabilities.size());

		result.resize(write(&result[0], result.size(), _session_number, _challenge, _cipher_capabilities));

		return result;
	}

	inline session_number_type clear_session_request_message::session_number() const
	{
		return ntohl(buffer_tools::get<session_number_type>(data(), 0));
	}

	inline challenge_type clear_session_request_message::challenge() const
	{
		challenge_type result;

		std::copy(data() + sizeof(session_number_type), data() + sizeof(session_number_type) + result.size(), result.begin());

		return result;
	}

	inline size_t clear_session_request_message::cipher_capabilities_size() const
	{
		return ntohs(buffer_tools::get<uint16_t>(data(), sizeof(session_number_type) + challenge_type::static_size));
	}

	inline const uint8_t* clear_session_request_message::data() const
	{
		return static_cast<const uint8_t*>(m_data);
	}
}

#endif /* FSCP_CLEAR_SESSION_REQUEST_MESSAGE_HPP */

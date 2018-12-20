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
 * \file hello_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A hello message class.
 */

#ifndef FSCP_HELLO_MESSAGE_HPP
#define FSCP_HELLO_MESSAGE_HPP

#include "message.hpp"

namespace fscp
{
	/**
	 * \brief A hello message class.
	 */
	class hello_message : public message
	{
		public:

			/**
			 * \brief Write a hello request message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param unique_number The unique number to write.
			 * \return The count of bytes written.
			 */
			static size_t write_request(void* buf, size_t buf_len, uint32_t unique_number);

			/**
			 * \brief Write a hello response message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param unique_number The unique number to write.
			 * \return The count of bytes written.
			 */
			static size_t write_response(void* buf, size_t buf_len, uint32_t unique_number);

			/**
			 * \brief Create a hello_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			hello_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a hello_message from a message.
			 * \param message The message.
			 */
			hello_message(const message& message);

			/**
			 * \brief Get the unique number.
			 * \return The unique number.
			 */
			uint32_t unique_number() const;

		protected:

			/**
			 * \brief The length of the body.
			 */
			static const size_t BODY_LENGTH = 4;
	};

	inline uint32_t hello_message::unique_number() const
	{
		return ntohl(buffer_tools::get<uint32_t>(payload(), 0));
	}
}

#endif /* FSCP_HELLO_MESSAGE_HPP */

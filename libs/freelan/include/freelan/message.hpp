/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file message.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The messages exchanged by the peers.
 */

#ifndef FREELAN_MESSAGE_HPP
#define FREELAN_MESSAGE_HPP

#include <boost/asio.hpp>

#include <fscp/buffer_tools.hpp>

namespace freelan
{
	/**
	 * \brief The base class for all messages.
	 *
	 * A message is composed of a message type and arbitrary
	 * data depending on the effective message type.
	 */
	class message
	{
		public:

			/**
			 * \brief The message type.
			 */
			enum message_type
			{
				MT_ROUTES_REQUEST = 0x01,
				MT_ROUTES = 0x02
			};

			/**
			 * \brief Write a message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param type The message type.
			 * \param length The length of the payload.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, message_type type, size_t length);

			/**
			 * \brief Create a message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			message(const void* buf, size_t buf_len);

			/**
			 * \brief Get the type.
			 * \return The type.
			 */
			message_type type() const;

			/**
			 * \brief Get the length.
			 * \return The length.
			 */
			size_t length() const;

			/**
			 * \brief Get the raw data.
			 * \return The message data buffer.
			 */
			const uint8_t* data() const;

			/**
			 * \brief Get the total size of the message.
			 * \return The total size of the message.
			 */
			size_t size() const;

			/**
			 * \brief Get the payload data.
			 * \return The payload data.
			 */
			const uint8_t* payload() const;

		protected:

			/**
			 * \brief The length of the header.
			 */
			static const size_t HEADER_LENGTH = sizeof(uint8_t) +  sizeof(uint16_t);

		private:

			const void* m_data;
	};

	inline message::message_type message::type() const
	{
		return static_cast<message_type>(fscp::buffer_tools::get<uint8_t>(m_data, 0));
	}

	inline size_t message::length() const
	{
		return ntohs(fscp::buffer_tools::get<uint16_t>(m_data, 1));
	}

	inline const uint8_t* message::data() const
	{
		return static_cast<const uint8_t*>(m_data);
	}

	inline size_t message::size() const
	{
		return HEADER_LENGTH + length();
	}

	inline const uint8_t* message::payload() const
	{
		return static_cast<const uint8_t*>(m_data) + HEADER_LENGTH;
	}
}

#endif /* FREELAN_MESSAGE_HPP */

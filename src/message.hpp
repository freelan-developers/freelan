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
 * \file message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A generic message class.
 */

#ifndef FSCP_MESSAGE_HPP
#define FSCP_MESSAGE_HPP

#include "buffer_tools.hpp"
#include "constants.hpp"

#include <boost/asio.hpp>

#include <stdint.h>
#include <cstring>

namespace fscp
{
	/**
	 * \brief A generic message class.
	 */
	class message
	{
		public:

			/**
			 * \brief Create a message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			message(void* buf, size_t buf_len);

			/**
			 * \brief Get the version.
			 * \return The version.
			 */
			unsigned int version() const;

			/**
			 * \brief Set the version.
			 * \param version The version.
			 */
			void set_version(unsigned int version);

			/**
			 * \brief Get the type.
			 * \return The type.
			 */
			message_type type() const;

			/**
			 * \brief Set the type.
			 * \param type The type.
			 */
			void set_type(message_type type);

			/**
			 * \brief Get the length.
			 * \return The length.
			 */
			size_t length() const;

			/**
			 * \brief Set the length.
			 * \param length The length.
			 */
			void set_length(size_t length);

			/**
			 * \brief Get the raw data.
			 * \return The message data buffer.
			 */
			uint8_t* data();

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
			uint8_t* payload();

			/**
			 * \brief Get the payload data.
			 * \return The payload data.
			 */
			const uint8_t* payload() const;

		private:

			static const size_t HEADER_LENGTH = 4;

			void* m_data;
	};

	inline unsigned int message::version() const
	{
		return buffer_tools::get<uint8_t>(m_data, 0);
	}
	
	inline void message::set_version(unsigned int _version)
	{
		buffer_tools::set<uint8_t>(m_data, 0, _version);
	}

	inline message_type message::type() const
	{
		return buffer_tools::get<uint8_t>(m_data, 1);
	}
	
	inline void message::set_type(message_type _type)
	{
		buffer_tools::set<uint8_t>(m_data, 1, _type);
	}

	inline size_t message::length() const
	{
		return ntohs(buffer_tools::get<uint16_t>(m_data, 2));
	}
	
	inline void message::set_length(size_t _length)
	{
		buffer_tools::set<uint16_t>(m_data, 2, htons(static_cast<uint16_t>(_length)));
	}

	inline uint8_t* message::data()
	{
		return static_cast<uint8_t*>(m_data);
	}

	inline const uint8_t* message::data() const
	{
		return static_cast<const uint8_t*>(m_data);
	}

	inline size_t message::size() const
	{
		return HEADER_LENGTH + length();
	}

	inline uint8_t* message::payload()
	{
		return static_cast<uint8_t*>(m_data) + HEADER_LENGTH;
	}

	inline const uint8_t* message::payload() const
	{
		return static_cast<const uint8_t*>(m_data) + HEADER_LENGTH;
	}
}

#endif /* FSCP_MESSAGE_HPP */

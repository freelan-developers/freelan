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

#include <stdint.h>
#include <cassert>
#include <stdexcept>
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
			 * \brief Read a message from a buffer.
			 * \param buf The buffer.
			 * \param buf_len The length of buf. Must be at least sizeof(message) bytes long.
			 * \return A message.
			 */
			static message read(const void* buf, size_t buf_len);

			/**
			 * \brief Create a default empty message.
			 */
			message();

			/**
			 * \brief Create a new message.
			 * \param version The message version.
			 * \param type The message type.
			 * \param length The message length.
			 */
			message(uint8_t version, uint8_t type, uint16_t length);

			/**
			 * \brief Get the version.
			 * \return The version.
			 */
			uint8_t version() const;

			/**
			 * \brief Set the version.
			 * \param version The version.
			 */
			void set_version(uint8_t version);

			/**
			 * \brief Get the type.
			 * \return The type.
			 */
			uint8_t type() const;

			/**
			 * \brief Set the type.
			 * \param type The type.
			 */
			void set_type(uint8_t type);

			/**
			 * \brief Get the length.
			 * \return The length.
			 */
			uint16_t length() const;

			/**
			 * \brief Set the length.
			 * \param length The length.
			 */
			void set_length(uint16_t length);

			/**
			 * \brief Write the message to a buffer.
			 * \param buf The buffer to write the message to.
			 * \param buf_len The length of buf. Must be at least sizeof(message) bytes long.
			 */
			void write(void* buf, size_t buf_len);

		private:

			uint8_t m_version;
			uint8_t m_type;
			uint16_t m_length;
	} __attribute__((__packed__));

	inline message message::read(const void* buf, size_t buf_len)
	{
		message msg;

		assert(buf_len >= sizeof(msg));

		if (buf_len < sizeof(msg))
		{
			throw std::invalid_argument("buf_len");
		}

		std::memcpy(&msg, buf, sizeof(msg));

		return msg;
	}

	inline message::message() :
		m_version(0), m_type(0), m_length(0)
	{
	}

	inline message::message(uint8_t _version, uint8_t _type, uint16_t _length) :
		m_version(_version), m_type(_type), m_length(_length)
	{
	}

	inline uint8_t message::version() const
	{
		return m_version;
	}
	
	inline void message::set_version(uint8_t _version)
	{
		m_version = _version;
	}

	inline uint8_t message::type() const
	{
		return m_type;
	}
	
	inline void message::set_type(uint8_t _type)
	{
		m_type = _type;
	}

	inline uint16_t message::length() const
	{
		return m_length;
	}
	
	inline void message::set_length(uint16_t _length)
	{
		m_length = _length;
	}
	
	inline void message::write(void* buf, size_t buf_len)
	{
		assert(buf_len >= sizeof(*this));

		if (buf_len < sizeof(*this))
		{
			throw std::invalid_argument("buf_len");
		}

		std::memcpy(buf, this, sizeof(*this));
	}
}

#endif /* FSCP_MESSAGE_HPP */

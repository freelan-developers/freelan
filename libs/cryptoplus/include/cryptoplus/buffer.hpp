/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file buffer.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A buffer class.
 */

#ifndef CRYPTOPLUS_BUFFER_HPP
#define CRYPTOPLUS_BUFFER_HPP

#include <vector>
#include <stdint.h>
#include <iostream>
#include <algorithm>

namespace cryptoplus
{
	/**
	 * \brief A buffer type.
	 */
	class buffer
	{
		public:

			/**
			 * \brief The underlying storage type.
			 */
			typedef std::vector<uint8_t> storage_type;

			/**
			 * \brief Create an empty buffer.
			 */
			buffer() {}

			/**
			 * \brief Create a buffer that has the specified size.
			 * \param size The size of the buffer to create.
			 */
			explicit buffer(size_t size) : m_data(size) {}

			/**
			 * \brief Create a buffer from a raw buffer.
			 * \param data The raw buffer.
			 * \param data_len The count of bytes to copy.
			 */
			buffer(const void* data, size_t data_len);

			/**
			 * \brief Create a buffer from a C-string.
			 * \param str The C-string.
			 */
			explicit buffer(const char* str);

			/**
			 * \brief Create a buffer from a string.
			 * \param str The string.
			 */
			explicit buffer(const std::string& str);

			/**
			 * \brief Create a buffer from its storage type.
			 * \param data The storage type.
			 */
			explicit buffer(const storage_type& data);

			/**
			 * \brief Create a buffer from two iterators.
			 * \tparam Iterator The iterator type.
			 * \param begin An iterator to the first element.
			 * \param end An iterator past the last element.
			 */
			template <typename Iterator>
			buffer(Iterator begin, Iterator end) : m_data(begin, end) {}

			/**
			 * \brief Get the underlying data.
			 * \return The underlying data.
			 */
			storage_type& data()
			{
				return m_data;
			}

			/**
			 * \brief Get the underlying data.
			 * \return The underlying data.
			 */
			const storage_type& data() const
			{
				return m_data;
			}

			/**
			 * \brief Get the buffer as a string.
			 * \return The buffer as a string.
			 */
			std::string to_string() const
			{
				return std::string(m_data.begin(), m_data.end());
			}

			/**
			 * \brief Check if the buffer is empty.
			 * \return true if the buffer is empty, false otherwise.
			 */
			bool empty() const
			{
				return m_data.empty();
			}

		private:

			std::vector<uint8_t> m_data;

			friend bool operator!(const buffer& buf)
			{
				return buf.empty();
			}
	};

	/**
	 * \brief Compare two buffers for equality.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two buffers have the exact same content.
	 */
	bool operator==(const buffer& lhs, const buffer& rhs);

	/**
	 * \brief Compare two buffers for non-equality.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two buffers have different contents.
	 */
	bool operator!=(const buffer& lhs, const buffer& rhs);

	/**
	 * \brief Compare two buffers for ordering.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if lhs is "smaller" than rhs.
	 */
	bool operator<(const buffer& lhs, const buffer& rhs);

	/**
	 * \brief Cast a buffer to a pointer.
	 * \tparam T The type of the pointer.
	 * \param buf The buffer to cast.
	 * \return The pointer.
	 */
	template<typename T>
	T buffer_cast(buffer& buf)
	{
		return buf.data().empty() ? nullptr : reinterpret_cast<T>(&buf.data()[0]);
	}

	/**
	 * \brief Cast a buffer to a pointer.
	 * \tparam T The type of the pointer.
	 * \param buf The buffer to cast.
	 * \return The pointer.
	 */
	template<typename T>
	T buffer_cast(const buffer& buf)
	{
		return buf.data().empty() ? nullptr : reinterpret_cast<T>(&buf.data()[0]);
	}

	/**
	 * \brief Get the size of a buffer.
	 * \param buf The buffer to get the size of.
	 * \return The size of buf.
	 */
	size_t buffer_size(const buffer& buf);

	/**
	 * \brief Output a buffer to a stream, as an hexadecimal string.
	 * \param os The output stream.
	 * \param buf The buffer to output.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const buffer& buf);

	/**
	 * \brief Convert a buffer to its hexadecimal representation.
	 * \param buf The buffer to convert.
	 * \return The hexadecimal representation of buf.
	 */
	std::string hex(const buffer& buf);

	inline bool operator==(const buffer& lhs, const buffer& rhs)
	{
		// Optimization-free implementation to prevent timing attacks.
		bool result = true;
		const auto len = std::min(lhs.data().size(), rhs.data().size());

		for (size_t i = 0; i < len; ++i)
		{
			if (lhs.data()[i] != rhs.data()[i])
			{
				result = false;
			}
		}

		if (lhs.data().size() != rhs.data().size())
		{
			result = false;
		}

		return result;
	}

	inline bool operator!=(const buffer& lhs, const buffer& rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator<(const buffer& lhs, const buffer& rhs)
	{
		return lhs.data() < rhs.data();
	}

	inline size_t buffer_size(const buffer& buf)
	{
		return buf.data().size();
	}
}

#endif /* CRYPTOPLUS_BUFFER_HPP */

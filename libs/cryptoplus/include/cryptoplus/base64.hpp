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
 * \file base64.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Base 64 related functions.
 */

#ifndef CRYPTOPLUS_BASE64_HPP
#define CRYPTOPLUS_BASE64_HPP

#include "buffer.hpp"

#include <string>

namespace cryptoplus
{
	/**
	 * \brief base64 encode a given buffer.
	 * \param output The output buffer.
	 * \param outputlen The output buffer length. Specify at least (((inputlen + 2) / 3) * 4) + 3 to get a complete result.
	 * \param input The string to base64 encode.
	 * \param inputlen The length of input.
	 * \return The count of bytes of output that were used.
	 *
	 * If output is too small to hold the result, the data is truncated.
	 */
	size_t base64_encode(void* output, size_t outputlen, const void* input, size_t inputlen);

	/**
	 * \brief base64 encode a given buffer.
	 * \param input The string to base64 encode.
	 * \param inputlen The length of input.
	 * \return The base64 encoded string.
	 */
	std::string base64_encode(const void* input, size_t inputlen);

	/**
	 * \brief base64 encode a given string.
	 * \param buf The buffer to encode.
	 * \return The base64 encoded string.
	 */
	std::string base64_encode(const buffer& buf);

	/**
	 * \brief base64 decode a given string.
	 * \param output The output buffer.
	 * \param outputlen The output buffer length. Specify at least (inputlen * 3) / 4 to get a complete result.
	 * \param input The base64 encoded string to decode.
	 * \param inputlen The length of input.
	 * \return The count of bytes of output that were used.
	 *
	 * If output is too small to hold the result, the data is truncated.
	 */
	size_t base64_decode(void* output, size_t outputlen, const void* input, size_t inputlen);

	/**
	 * \brief base64 decode a given buffer.
	 * \param input The string to base64 decode.
	 * \param inputlen The length of input.
	 * \return The base64 decoded string.
	 */
	buffer base64_decode(const void* input, size_t inputlen);

	/**
	 * \brief base64 decode a given string.
	 * \param str The base64 encoded string to decode.
	 * \return The decoded string.
	 */
	buffer base64_decode(const std::string& str);

	inline std::string base64_encode(const void* input, size_t inputlen)
	{
		std::string result(((inputlen + 2) / 3) * 4, '\0');

		if (!result.empty())
		{
			result.resize(base64_encode(&result[0], result.size(), input, inputlen));
		}

		return result;
	}

	inline std::string base64_encode(const buffer& buf)
	{
		return base64_encode(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
	}

	inline buffer base64_decode(const void* input, size_t inputlen)
	{
		buffer result((inputlen * 3) / 4);

		result.data().resize(base64_decode(buffer_cast<uint8_t*>(result), buffer_size(result), input, inputlen));

		return result;
	}

	inline buffer base64_decode(const std::string& str)
	{
		return base64_decode(str.c_str(), str.size());
	}
}

#endif /* CRYPTOPLUS_BASE64_HPP */


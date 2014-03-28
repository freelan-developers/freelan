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
 * \file message_digest.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Message digest helper functions.
 */

#ifndef CRYPTOPLUS_HASH_MESSAGE_DIGEST_HPP
#define CRYPTOPLUS_HASH_MESSAGE_DIGEST_HPP

#include "../buffer.hpp"
#include "message_digest_algorithm.hpp"

#include <openssl/evp.h>

namespace cryptoplus
{
	namespace hash
	{
		/**
		 * \brief Compute a message digest for the given buffer, using the given digest method.
		 * \param out The output buffer. Must be at least message_digest_size(md) bytes long.
		 * \param out_len The output buffer length.
		 * \param data The buffer.
		 * \param len The buffer length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The count of bytes written to out. Should be equal to the size of the message digest algorithm.
		 */
		size_t message_digest(void* out, size_t out_len, const void* data, size_t len, const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

		/**
		 * \brief Compute a message digest for the given buffer, using the given digest method.
		 * \param data The buffer.
		 * \param len The buffer length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The message digest.
		 */
		buffer message_digest(const void* data, size_t len, const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

		/**
		 * \brief Compute a message digest for the given buffer, using the given digest method.
		 * \param buf The buffer.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The message digest.
		 */
		buffer message_digest(const buffer& buf, const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

		inline buffer message_digest(const void* data, size_t len, const message_digest_algorithm& algorithm, ENGINE* impl)
		{
			buffer result(algorithm.result_size());

			message_digest(buffer_cast<uint8_t*>(result), buffer_size(result), data, len, algorithm, impl);

			return result;
		}

		inline buffer message_digest(const buffer& buf, const message_digest_algorithm& algorithm, ENGINE* impl)
		{
			return message_digest(buffer_cast<const uint8_t*>(buf), buffer_size(buf), algorithm, impl);
		}
	}
}

#endif /* CRYPTOPLUS_HASH_MESSAGE_DIGEST_HPP */


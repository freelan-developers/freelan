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
 * \file pbkdf2.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief PBKDF2 helper functions.
 */

#ifndef CRYPTOPLUS_HASH_PBKDF2_HPP
#define CRYPTOPLUS_HASH_PBKDF2_HPP

#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "message_digest.hpp"
#include "message_digest_algorithm.hpp"

#include <openssl/evp.h>

namespace cryptoplus
{
	namespace hash
	{
		/**
		 * \brief Generate a buffer from a password and a salt, using PBKDF2.
		 * \param password The password to generate a digest from.
		 * \param passwordlen The password size.
		 * \param salt The salt.
		 * \param saltlen The salt len.
		 * \param outbuf The PBKDF2 resulting buffer.
		 * \param outbuflen The resulting buffer length. Optimal value is the size of the digest for the specified hash method. See cryptoplus::hash::message_digest_size().
		 * \param algorithm The message digest algorithm to use.
		 * \param iter The iteration count. Default is 1000. The more iterations you use, the securer (and the slower) the function becomes.
		 * \return The count of bytes written. Should be outbuflen.
		 * \warning This function is slow by design.
		 *
		 * The list of the available message digest algorithms depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
		 *
		 * Some versions of OpenSSL only provide SHA1 as a hash method. In this case, pbkdf2() will throw an std::invalid_argument exception.
		 */
		size_t pbkdf2(const void* password, size_t passwordlen, const void* salt, size_t saltlen, void* outbuf, size_t outbuflen, const message_digest_algorithm& algorithm, unsigned int iter = 1000);

		/**
		 * \brief Generate a buffer from a password and a salt, using PBKDF2.
		 * \param password The password to generate a digest from.
		 * \param passwordlen The password size.
		 * \param salt The salt.
		 * \param saltlen The salt len.
		 * \param algorithm The message digest algorithm to use.
		 * \param iter The iteration count. Default is 1000. The more iterations you use, the securer (and the slower) the function becomes.
		 * \return The generated buffer.
		 * \warning This function is slow by design.
		 *
		 * The list of the available message digest algorithms depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
		 *
		 * Some versions of OpenSSL only provide SHA1 as a hash method. In this case, pbkdf2() will throw an std::invalid_argument exception.
		 */
		buffer pbkdf2(const void* password, size_t passwordlen, const void* salt, size_t saltlen, const message_digest_algorithm& algorithm, unsigned int iter = 1000);

		inline buffer pbkdf2(const void* password, size_t passwordlen, const void* salt, size_t saltlen, const message_digest_algorithm& algorithm, unsigned int iter)
		{
			buffer result(algorithm.result_size());

			pbkdf2(password, passwordlen, salt, saltlen, buffer_cast<uint8_t*>(result), buffer_size(result), algorithm, iter);

			return result;
		}
	}
}

#endif /* CRYPTOPLUS_HASH_PBKDF2_HPP */


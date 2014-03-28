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
 * \file pbkdf2.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief PBKDF2 helper functions.
 */

#include "hash/pbkdf2.hpp"

#include <openssl/opensslv.h>

#include <stdexcept>

#if OPENSSL_VERSION_NUMBER < 0x10000000
namespace
{
	int PKCS5_PBKDF2_HMAC(const char* pass, int passlen, const unsigned char* salt, int saltlen, int iter, const EVP_MD* md, int outlen, unsigned char* out)
	{
		if (md == EVP_sha1())
		{
			return PKCS5_PBKDF2_HMAC_SHA1(pass, passlen, salt, saltlen, iter, outlen, out);
		}

		throw std::invalid_argument("md");
	}
}
#endif

#include <cassert>

namespace cryptoplus
{
	namespace hash
	{
		size_t pbkdf2(const void* password, size_t passwordlen, const void* salt, size_t saltlen, void* outbuf, size_t outbuflen, const message_digest_algorithm& algorithm, unsigned int iter)
		{
			int result = PKCS5_PBKDF2_HMAC(
			                 static_cast<const char*>(password),
			                 static_cast<int>(passwordlen),
			                 static_cast<const unsigned char*>(salt),
			                 static_cast<int>(saltlen),
			                 static_cast<int>(iter),
			                 algorithm.raw(),
			                 static_cast<int>(outbuflen),
			                 static_cast<unsigned char*>(outbuf)
			             );

			return result;
		}
	}
}


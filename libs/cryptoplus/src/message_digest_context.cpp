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
 * \file message_digest_context.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A message digest context class.
 */

#include "hash/message_digest_context.hpp"

#include "pkey/pkey.hpp"

#include <cassert>

namespace cryptoplus
{
	namespace hash
	{
		size_t message_digest_context::finalize(void* md, size_t md_len)
		{
			assert(md);

			unsigned int ilen = static_cast<unsigned int>(md_len);

			throw_error_if_not(EVP_DigestFinal_ex(m_ctx, static_cast<unsigned char*>(md), &ilen) != 0);

			return ilen;
		}

		size_t message_digest_context::sign_finalize(void* sig, size_t sig_len, pkey::pkey& pkey)
		{
			assert(sig);

			unsigned int ilen = static_cast<unsigned int>(sig_len);

			throw_error_if_not(EVP_SignFinal(m_ctx, static_cast<unsigned char*>(sig), &ilen, pkey.raw()) != 0);

			return ilen;
		}

		bool message_digest_context::verify_finalize(const void* sig, size_t sig_len, pkey::pkey& pkey)
		{
			int result = EVP_VerifyFinal(m_ctx, static_cast<const unsigned char*>(sig), static_cast<unsigned int>(sig_len), pkey.raw());

			throw_error_if(result < 0);

			return (result == 1);
		}

		size_t message_digest_context::digest_sign_finalize(void* md, size_t md_len)
		{
			throw_error_if_not(EVP_DigestSignFinal(m_ctx, static_cast<unsigned char*>(md), &md_len) != 0);

			return md_len;
		}

		bool message_digest_context::digest_verify_finalize(const void* sig, size_t sig_len)
		{
			// The const_cast<> below is needed because the prototype in the code is incorrect...
			//
			// The documentation clearly states this should be const.
			// http://www.openssl.org/docs/crypto/EVP_DigestVerifyInit.html

			int result = EVP_DigestVerifyFinal(m_ctx, const_cast<unsigned char*>(static_cast<const unsigned char*>(sig)), static_cast<unsigned int>(sig_len));

			throw_error_if(result < 0);

			return (result == 1);
		}

	}
}


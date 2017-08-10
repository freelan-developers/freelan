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
 * \file hmac_context.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A HMAC context class.
 */

#include "hash/hmac_context.hpp"

#include <cassert>

namespace cryptoplus
{
	namespace hash
	{
		void hmac_context::initialize(const void* key, size_t key_len, const message_digest_algorithm* _algorithm, ENGINE* impl)
		{
#if OPENSSL_VERSION_NUMBER < 0x01000000
			HMAC_Init_ex(m_ctx, key, static_cast<int>(key_len), _algorithm ? _algorithm->raw() : NULL, impl);
#else
			throw_error_if_not(HMAC_Init_ex(m_ctx, key, static_cast<int>(key_len), _algorithm ? _algorithm->raw() : NULL, impl) != 0);
#endif
		}

		size_t hmac_context::finalize(void* md, size_t len)
		{
			assert(md);

			unsigned int ilen = static_cast<unsigned int>(len);

#if OPENSSL_VERSION_NUMBER < 0x01000000
			HMAC_Final(m_ctx, static_cast<unsigned char*>(md), &ilen);
#else
			throw_error_if_not(HMAC_Final(m_ctx, static_cast<unsigned char*>(md), &ilen) != 0);
#endif
			return ilen;
		}
	}
}


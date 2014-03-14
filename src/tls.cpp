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
 * \file tls.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief TLS utility functions.
 */

#include "tls/tls.hpp"

#include "hash/message_digest_context.hpp"
#include "pkey/pkey.hpp"

#include <iomanip>
#include <cassert>

namespace cryptoplus
{
	namespace tls
	{
		size_t p_hash(void* out, size_t out_len, const void* key, size_t key_len, const void* data, size_t data_len, const void* data2, size_t data2_len, const hash::message_digest_algorithm& algorithm, ENGINE* impl)
		{
			using hash::message_digest_context;
			using pkey::pkey;

			assert(out);
			assert(key);

			message_digest_context ctx;
			message_digest_context ctx_a1;
			message_digest_context ctx_init;

			ctx.set_flags(EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);

			pkey mac_key = pkey::from_hmac_key(key, key_len);

			ctx_init.digest_sign_initialize(algorithm, mac_key, NULL, impl);

			// Everything is set up. We can start the computation.

			ctx.copy(ctx_init);

			if (data) {
				ctx.digest_sign_update(data, data_len);
			}

			if (data2) {
				ctx.digest_sign_update(data2, data2_len);
			}

			buffer a1 = ctx.digest_sign_finalize();

			// We got A1 let's compute for real.

			const size_t chunk_size = algorithm.result_size();
			size_t bytes_left = out_len;
			unsigned char* buf = static_cast<unsigned char*>(out);

			while (bytes_left > 0)
			{
				ctx.copy(ctx_init);
				ctx.digest_sign_update(a1);
				ctx_a1.copy(ctx);

				if (data) {
					ctx.digest_sign_update(data, data_len);
				}

				if (data2) {
					ctx.digest_sign_update(data2, data2_len);
				}

				if (bytes_left > chunk_size) {
					const size_t len = ctx.digest_sign_finalize(buf, bytes_left);
					buf += len;
					bytes_left -= len;

					a1 = ctx_a1.digest_sign_finalize();
				} else {
					a1 = ctx.digest_sign_finalize();
					std::copy(a1.data().begin(), a1.data().begin() + bytes_left, buf);
					bytes_left = 0;
				}
			}

			return out_len;
		}
	}
}


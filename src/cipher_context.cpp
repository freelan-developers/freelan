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
 * \file cipher_context.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher context class.
 */

#include "cipher/cipher_context.hpp"

#include "pkey/pkey.hpp"

#include <cassert>

namespace cryptoplus
{
	namespace cipher
	{
		namespace
		{
			typedef int (*update_function)(EVP_CIPHER_CTX*, unsigned char*, int*, const unsigned char*, int);
			typedef int (*finalize_function)(EVP_CIPHER_CTX*, unsigned char*, int*);

			int _EVP_SealUpdate(EVP_CIPHER_CTX* ctx, unsigned char* out, int* outl, const unsigned char* in, int inl)
			{
				return EVP_SealUpdate(ctx, out, outl, in, inl);
			}

			int _EVP_OpenUpdate(EVP_CIPHER_CTX* ctx, unsigned char* out, int* outl, const unsigned char* in, int inl)
			{
				return EVP_OpenUpdate(ctx, out, outl, in, inl);
			}

			void generic_update(cipher_context& ctx, update_function update_func, void* out, size_t& out_len, const void* in, size_t in_len)
			{
				assert(out);
				assert(in);
				assert(out_len >= in_len + ctx.algorithm().block_size());

				int iout_len = static_cast<int>(out_len);

				error::throw_error_if_not(update_func(&ctx.raw(), static_cast<unsigned char*>(out), &iout_len, static_cast<const unsigned char*>(in), static_cast<int>(in_len)));

				out_len = iout_len;
			}

			void generic_finalize(cipher_context& ctx, finalize_function finalize_func, void* out, size_t& out_len)
			{
				assert(out);
				assert(out_len >= ctx.algorithm().block_size());

				int iout_len = static_cast<int>(out_len);

				error::throw_error_if_not(finalize_func(&ctx.raw(), static_cast<unsigned char*>(out), &iout_len));

				out_len = iout_len;
			}
		}

		void cipher_context::initialize(const cipher_algorithm& _algorithm, cipher_context::cipher_direction direction, const void* key, const void* iv, ENGINE* impl)
		{
			assert(key);

			error::throw_error_if_not(EVP_CipherInit_ex(&m_ctx, _algorithm.raw(), impl, static_cast<const unsigned char*>(key), static_cast<const unsigned char*>(iv), static_cast<int>(direction)));
		}

		std::vector<unsigned char> cipher_context::seal_initialize(const cipher_algorithm& _algorithm, void* iv, pkey::pkey pkey)
		{
			return seal_initialize(_algorithm, iv, &pkey, &pkey + sizeof(&pkey))[0];
		}

		void cipher_context::open_initialize(const cipher_algorithm& _algorithm, const void* key, size_t key_len, const void* iv, pkey::pkey pkey)
		{
			assert(key);

			error::throw_error_if_not(EVP_OpenInit(&m_ctx, _algorithm.raw(), static_cast<const unsigned char*>(key), key_len, static_cast<const unsigned char*>(iv), pkey.raw()));
		}

		void cipher_context::update(void* out, size_t& out_len, const void* in, size_t in_len)
		{
			generic_update(*this, EVP_CipherUpdate, out, out_len, in, in_len);
		}

		void cipher_context::seal_update(void* out, size_t& out_len, const void* in, size_t in_len)
		{
			generic_update(*this, _EVP_SealUpdate, out, out_len, in, in_len);
		}

		void cipher_context::open_update(void* out, size_t& out_len, const void* in, size_t in_len)
		{
			generic_update(*this, _EVP_OpenUpdate, out, out_len, in, in_len);
		}

		void cipher_context::finalize(void* out, size_t& out_len)
		{
			generic_finalize(*this, EVP_CipherFinal, out, out_len);
		}

		void cipher_context::seal_finalize(void* out, size_t& out_len)
		{
			generic_finalize(*this, EVP_SealFinal, out, out_len);
		}

		void cipher_context::open_finalize(void* out, size_t& out_len)
		{
			generic_finalize(*this, EVP_OpenFinal, out, out_len);
		}
	}
}


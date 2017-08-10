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
#include "random/random.hpp"

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

			size_t generic_update(cipher_context& ctx, update_function update_func, void* out, size_t out_len, const void* in, size_t in_len)
			{
				assert(out);

				if (in_len <= 0)
				{
					return 0;
				}

				assert(in);
				assert(out_len >= in_len + ctx.algorithm().block_size());

				int iout_len = static_cast<int>(out_len);

				throw_error_if_not(update_func(&ctx.raw(), static_cast<unsigned char*>(out), &iout_len, static_cast<const unsigned char*>(in), static_cast<int>(in_len)) != 0);

				return iout_len;
			}

			size_t generic_finalize(cipher_context& ctx, finalize_function finalize_func, void* out, size_t out_len)
			{
				assert(out);
				assert(out_len >= ctx.algorithm().block_size());

				int iout_len = static_cast<int>(out_len);

				throw_error_if_not(finalize_func(&ctx.raw(), static_cast<unsigned char*>(out), &iout_len) != 0);

				return iout_len;
			}
		}

		void cipher_context::initialize(const cipher_algorithm& _algorithm, cipher_context::cipher_direction direction, const void* key, size_t key_len, const void* iv, ENGINE* impl)
		{
			if (key && _algorithm)
			{
				if (key_len != _algorithm.key_length())
				{
					throw std::runtime_error("key_len");
				}
			}

			// Doing the same test on the IV is wrong because for some algorithms, the IV size is dynamic.

			throw_error_if_not(EVP_CipherInit_ex(m_ctx, _algorithm.raw(), impl, static_cast<const unsigned char*>(key), static_cast<const unsigned char*>(iv), static_cast<int>(direction)) != 0);
		}

		buffer cipher_context::seal_initialize(const cipher_algorithm& _algorithm, void* iv, pkey::pkey pkey)
		{
			return seal_initialize(_algorithm, iv, &pkey, &pkey + sizeof(&pkey))[0];
		}

		void cipher_context::open_initialize(const cipher_algorithm& _algorithm, const void* key, size_t key_len, const void* iv, pkey::pkey pkey)
		{
			if (key && _algorithm)
			{
				if (key_len != _algorithm.key_length())
				{
					throw std::runtime_error("key_len");
				}
			}

			// Doing the same test on the IV is wrong because for some algorithms, the IV size is dynamic.

			throw_error_if_not(EVP_OpenInit(m_ctx, _algorithm.raw(), static_cast<const unsigned char*>(key), static_cast<int>(key_len), static_cast<const unsigned char*>(iv), pkey.raw()) != 0);
		}

		size_t cipher_context::add_iso_10126_padding(void* buf, size_t buf_len, size_t max_buf_len) const
		{
			assert(buf);
			assert(buf_len <= max_buf_len);

			const size_t result_len = get_iso_10126_padding_size(buf_len);

			if (result_len > max_buf_len)
			{
				throw std::logic_error("The resulting buffer is too small");
			}

			const size_t padding_len = result_len - buf_len;

			unsigned char* padding = reinterpret_cast<unsigned char*>(buf) + buf_len;

			random::get_random_bytes(padding, padding_len - 1);
			padding[padding_len - 1] = static_cast<unsigned char>(padding_len);

			return result_len;
		}

		size_t cipher_context::verify_iso_10126_padding(const void* buf, size_t buf_len) const
		{
			assert(buf);

			if (buf_len % algorithm().block_size() != 0)
			{
				throw std::logic_error("buf_len should be a multiple of algorithm().block_size()");
			}

			const unsigned char* cbuf = reinterpret_cast<const unsigned char*>(buf);

			const size_t padding_len = static_cast<size_t>(cbuf[buf_len - 1]);

			if ((padding_len > algorithm().block_size()) || (padding_len == 0))
			{
				throw std::logic_error("Impossible padding length");
			}

			return buf_len - padding_len;
		}

		size_t cipher_context::update(void* out, size_t out_len, const void* in, size_t in_len)
		{
			return generic_update(*this, EVP_CipherUpdate, out, out_len, in, in_len);
		}

		size_t cipher_context::seal_update(void* out, size_t out_len, const void* in, size_t in_len)
		{
			return generic_update(*this, _EVP_SealUpdate, out, out_len, in, in_len);
		}

		size_t cipher_context::open_update(void* out, size_t out_len, const void* in, size_t in_len)
		{
			return generic_update(*this, _EVP_OpenUpdate, out, out_len, in, in_len);
		}

		size_t cipher_context::finalize(void* out, size_t out_len)
		{
			return generic_finalize(*this, EVP_CipherFinal, out, out_len);
		}

		size_t cipher_context::seal_finalize(void* out, size_t out_len)
		{
			return generic_finalize(*this, EVP_SealFinal, out, out_len);
		}

		size_t cipher_context::open_finalize(void* out, size_t out_len)
		{
			return generic_finalize(*this, EVP_OpenFinal, out, out_len);
		}
	}
}


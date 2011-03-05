/*
 * libcryptopen - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptopen.
 *
 * libcryptopen is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptopen is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptopen in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file cipher_context.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher context class.
 */

#include "cipher/cipher_context.hpp"

#include <cassert>

namespace cryptopen
{
	namespace cipher
	{
		void cipher_context::initialize(const cipher_algorithm& _algorithm, cipher_context::cipher_direction direction, const void* key, const void* iv, ENGINE* impl)
		{
			assert(key);

			error::throw_error_if_not(EVP_CipherInit_ex(&m_ctx, _algorithm.raw(), impl, static_cast<const unsigned char*>(key), static_cast<const unsigned char*>(iv), static_cast<int>(direction)));
		}
		
		void cipher_context::update(void* out, size_t& out_len, const void* in, size_t in_len)
		{
			assert(out);
			assert(in);
			assert(out_len >= in_len + algorithm().block_size());

			int iout_len = static_cast<int>(out_len);

			error::throw_error_if_not(EVP_CipherUpdate(&m_ctx, static_cast<unsigned char*>(out), &iout_len, static_cast<const unsigned char*>(in), static_cast<int>(in_len)));

			out_len = iout_len;
		}
		
		void cipher_context::finalize(void* out, size_t& out_len)
		{
			assert(out);
			assert(out_len >= algorithm().block_size());

			int iout_len = static_cast<int>(out_len);

			error::throw_error_if_not(EVP_CipherFinal(&m_ctx, static_cast<unsigned char*>(out), &iout_len));

			out_len = iout_len;
		}
	}
}


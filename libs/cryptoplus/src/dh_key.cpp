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
 * \file dh_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A Diffie-Hellman key class.
 */

#include "pkey/dh_key.hpp"

#include "bio/bio_chain.hpp"

#include <cassert>

namespace cryptoplus
{
	template <>
	pkey::dh_key::deleter_type pointer_wrapper<pkey::dh_key::value_type>::deleter = DH_free;

	namespace pkey
	{
		namespace
		{
			bio::bio_chain get_bio_chain_from_buffer(const void* buf, size_t buf_len)
			{
				return bio::bio_chain(BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(buf_len)));
			}
		}

		dh_key dh_key::generate_parameters(int prime_len, int generator, generate_callback_type callback, void* callback_arg)
		{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
			static_cast<void>(callback);
			static_cast<void>(callback_arg);

			dh_key dh = dh_key::create();

			if(DH_generate_parameters_ex(dh.raw(), prime_len, generator, NULL))
			{
				return dh;
			}
			else
			{
				return nullptr;
			}
#else
			return take_ownership(DH_generate_parameters(prime_len, generator, callback, callback_arg));
#endif
		}

		dh_key dh_key::from_parameters(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_parameters(bio_chain.first(), callback, callback_arg);
		}

		dh_key dh_key::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return dh_key(_ptr, deleter);
		}

		size_t dh_key::compute_key(void* out, size_t out_len, bn::bignum pub_key) const
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			int result = DH_compute_key(static_cast<unsigned char*>(out), pub_key.raw(), ptr().get());

			throw_error_if_not(result >= 0);

			return result;
		}
	}
}


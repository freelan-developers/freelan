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
 * \file dsa_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DSA key class.
 */

#include "pkey/dsa_key.hpp"

#include "bio/bio_chain.hpp"

#include <cassert>

namespace cryptoplus
{
	template <>
	pkey::dsa_key::deleter_type pointer_wrapper<pkey::dsa_key::value_type>::deleter = DSA_free;

	namespace pkey
	{
		namespace
		{
			bio::bio_chain get_bio_chain_from_buffer(const void* buf, size_t buf_len)
			{
				return bio::bio_chain(BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(buf_len)));
			}
		}

		dsa_key dsa_key::generate_parameters(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback, void* callback_arg, bool must_take_ownership)
		{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
			static_cast<void>(callback);
			static_cast<void>(callback_arg);
			static_cast<void>(must_take_ownership);
			dsa_key dsa = dsa_key::create();

			if(DSA_generate_parameters_ex(dsa.raw(), bits, static_cast<unsigned char*>(seed), static_cast<int>(seed_len), counter_ret, h_ret, NULL))
			{
				return dsa;
			}
			else
			{
				return nullptr;
			}
#else
			DSA* ptr = DSA_generate_parameters(bits, static_cast<unsigned char*>(seed), static_cast<int>(seed_len), counter_ret, h_ret, callback, callback_arg);

			if (must_take_ownership)
			{
				return take_ownership(ptr);
			}
			else
			{
				return ptr;
			}
#endif
		}

		dsa_key dsa_key::from_private_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_private_key(bio_chain.first(), callback, callback_arg);
		}

		dsa_key dsa_key::from_parameters(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_parameters(bio_chain.first(), callback, callback_arg);
		}

		dsa_key dsa_key::from_certificate_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_certificate_public_key(bio_chain.first(), callback, callback_arg);
		}

		dsa_key dsa_key::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return dsa_key(_ptr, deleter);
		}

		dsa_key dsa_key::to_public_key() const
		{
			bio::bio_chain bio_chain(BIO_s_mem());

			write_certificate_public_key(bio_chain.first());

			return from_certificate_public_key(bio_chain.first());
		}

		size_t dsa_key::sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type) const
		{
			unsigned int _out_len = static_cast<unsigned int>(out_len);

			throw_error_if_not(DSA_sign(type, static_cast<const unsigned char*>(buf), static_cast<int>(buf_len), static_cast<unsigned char*>(out), &_out_len, ptr().get()) != 0);

			return _out_len;
		}
	}
}


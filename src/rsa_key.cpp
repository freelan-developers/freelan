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
 * \file rsa_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A RSA key class.
 */

#include "pkey/rsa_key.hpp"

#include "bio/bio_chain.hpp"

#include <cassert>

namespace cryptopen
{
	namespace pkey
	{
		namespace
		{
			bio::bio_chain get_bio_chain_from_buffer(const void* buf, size_t buf_len)
			{
				return bio::bio_chain(BIO_new_mem_buf(const_cast<void*>(buf), buf_len));
			}
		}

		rsa_key rsa_key::generate_private_key(int num, unsigned long exponent, generate_callback_type callback, void* callback_arg)
		{
			// Exponent must be odd
			assert(exponent | 1);

			return rsa_key(boost::shared_ptr<RSA>(RSA_generate_key(num, exponent, callback, callback_arg), RSA_free));
		}

		rsa_key rsa_key::from_private_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_private_key(bio_chain.first(), callback, callback_arg);
		}

		rsa_key rsa_key::from_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_public_key(bio_chain.first(), callback, callback_arg);
		}

		rsa_key rsa_key::from_certificate_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_certificate_public_key(bio_chain.first(), callback, callback_arg);
		}

		rsa_key rsa_key::to_public_key()
		{
			bio::bio_chain bio_chain(BIO_s_mem());

			write_public_key(bio_chain.first());
			return from_public_key(bio_chain.first());
		}

		size_t rsa_key::private_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding)
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_private_encrypt(buf_len, static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), m_rsa.get(), padding);

			error::throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::public_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding)
		{
			assert(out_len >= size() - 11);

			if (out_len < size() - 11)
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_public_decrypt(buf_len, static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), m_rsa.get(), padding);

			error::throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::public_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding)
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_public_encrypt(buf_len, static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), m_rsa.get(), padding);

			error::throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::private_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding)
		{
			assert(out_len >= size() - 41);

			if (out_len < size() - 41)
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_private_decrypt(buf_len, static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), m_rsa.get(), padding);

			error::throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type)
		{
			unsigned int _out_len = out_len;

			error::throw_error_if_not(RSA_sign(type, static_cast<const unsigned char*>(buf), buf_len, static_cast<unsigned char*>(out), &_out_len, m_rsa.get()));

			return _out_len;
		}

		void rsa_key::verify(const void* _sign, size_t sign_len, const void* buf, size_t buf_len, int type)
		{
#if OPENSSL_VERSION_NUMBER >= 0x01000000
			error::throw_error_if_not(RSA_verify(type, static_cast<const unsigned char*>(buf), buf_len, static_cast<const unsigned char*>(_sign), sign_len, m_rsa.get()));
#else
			error::throw_error_if_not(RSA_verify(type, static_cast<unsigned char*>(const_cast<void*>(buf)), buf_len, static_cast<unsigned char*>(const_cast<void*>(_sign)), sign_len, m_rsa.get()));
#endif
		}
	}
}


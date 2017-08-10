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
 * \file rsa_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A RSA key class.
 */

#include "pkey/rsa_key.hpp"

#include "bio/bio_chain.hpp"

#include <cassert>

namespace cryptoplus
{
	template <>
	pkey::rsa_key::deleter_type pointer_wrapper<pkey::rsa_key::value_type>::deleter = RSA_free;

	namespace pkey
	{
		namespace
		{
			bio::bio_chain get_bio_chain_from_buffer(const void* buf, size_t buf_len)
			{
				return bio::bio_chain(BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(buf_len)));
			}
		}

		rsa_key rsa_key::generate_private_key(int num, unsigned long exponent, generate_callback_type callback, void* callback_arg, bool must_take_ownership)
		{
			static_cast<void>(callback);
			static_cast<void>(callback_arg);

			std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
			BN_set_word(bn.get(), exponent);

			rsa_key key = rsa_key::create();
			RSA_generate_key_ex(key.raw(), num, bn.get(), NULL);

			if (must_take_ownership)
			{
				return key;
			}
			else
			{
				return nullptr;
			}
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

		rsa_key rsa_key::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return rsa_key(_ptr, deleter);
		}

		rsa_key rsa_key::to_public_key() const
		{
			bio::bio_chain bio_chain(BIO_s_mem());

			write_public_key(bio_chain.first());
			return from_public_key(bio_chain.first());
		}

		void rsa_key::padding_add_PKCS1_PSS(void* out, size_t out_len, const void* buf, size_t buf_len, hash::message_digest_algorithm algorithm, int salt_len) const
		{
			assert(out_len >= algorithm.result_size());
			assert(buf_len >= algorithm.result_size());

			if (out_len < algorithm.result_size())
			{
				throw std::invalid_argument("out_len");
			}

			if (buf_len < algorithm.result_size())
			{
				throw std::invalid_argument("buf_len");
			}

			throw_error_if_not(RSA_padding_add_PKCS1_PSS(ptr().get(), static_cast<unsigned char*>(out), static_cast<const unsigned char*>(buf), algorithm.raw(), salt_len) != 0);
		}

		void rsa_key::verify_PKCS1_PSS(const void* digest, size_t digest_len, const void* buf, size_t /*buf_len*/, hash::message_digest_algorithm algorithm, int salt_len) const
		{
			assert(digest_len >= algorithm.result_size());

			if (digest_len < algorithm.result_size())
			{
				throw std::invalid_argument("digest_len");
			}

			//TODO: Use buf_len

			throw_error_if_not(RSA_verify_PKCS1_PSS(ptr().get(), static_cast<const unsigned char*>(digest), algorithm.raw(), static_cast<const unsigned char*>(buf), salt_len) != 0);
		}

		size_t rsa_key::private_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding) const
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_private_encrypt(static_cast<int>(buf_len), static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), ptr().get(), padding);

			throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::public_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding) const
		{
			assert(out_len >= size() - 11);

			if (out_len < size() - 11)
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_public_decrypt(static_cast<int>(buf_len), static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), ptr().get(), padding);

			throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::public_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding) const
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_public_encrypt(static_cast<int>(buf_len), static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), ptr().get(), padding);

			throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::private_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding) const
		{
			assert(out_len >= size() - 41);

			if (out_len < size() - 41)
			{
				throw std::invalid_argument("out_len");
			}

			int result = RSA_private_decrypt(static_cast<int>(buf_len), static_cast<const unsigned char*>(buf), static_cast<unsigned char*>(out), ptr().get(), padding);

			throw_error_if_not(result >= 0);

			return result;
		}

		size_t rsa_key::sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type) const
		{
			unsigned int _out_len = static_cast<unsigned int>(out_len);

			throw_error_if_not(RSA_sign(type, static_cast<const unsigned char*>(buf), static_cast<unsigned int>(buf_len), static_cast<unsigned char*>(out), &_out_len, ptr().get()) != 0);

			return _out_len;
		}

		void rsa_key::verify(const void* _sign, size_t sign_len, const void* buf, size_t buf_len, int type) const
		{
#if OPENSSL_VERSION_NUMBER >= 0x01000000
			throw_error_if_not(RSA_verify(type, static_cast<const unsigned char*>(buf), static_cast<unsigned int>(buf_len), static_cast<const unsigned char*>(_sign), static_cast<unsigned int>(sign_len), ptr().get()) != 0);
#else
			throw_error_if_not(RSA_verify(type, static_cast<unsigned char*>(const_cast<void*>(buf)), static_cast<unsigned int>(buf_len), static_cast<unsigned char*>(const_cast<void*>(_sign)), static_cast<unsigned int>(sign_len), ptr().get()) != 0);
#endif
		}
	}
}


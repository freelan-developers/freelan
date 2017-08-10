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
 * \file ecdhe.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Elliptical Curve Diffie-Hellman Ephemeral context class.
 */

#include "pkey/ecdhe.hpp"

#include <cassert>

#include "bio/bio_chain.hpp"

namespace cryptoplus
{
	namespace pkey
	{
		namespace
		{
			struct universal_deleter
			{
				void operator()(EVP_PKEY_CTX* ctx)
				{
					if (ctx)
					{
						::EVP_PKEY_CTX_free(ctx);
					}
				}
			};

			typedef std::unique_ptr<EVP_PKEY_CTX, universal_deleter> evp_pkey_context_type;
		}

		ecdhe_context::ecdhe_context(int nid) :
			m_nid(nid)
		{
		}

		void ecdhe_context::generate_keys()
		{
			evp_pkey_context_type parameters_context(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL));

			throw_error_if_not(parameters_context.get());
			throw_error_if(EVP_PKEY_paramgen_init(parameters_context.get()) != 1);
			throw_error_if(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(parameters_context.get(), m_nid) != 1);

			EVP_PKEY* cparameters = nullptr;
			throw_error_if_not(EVP_PKEY_paramgen(parameters_context.get(), &cparameters) == 1);
			pkey parameters = pkey::take_ownership(cparameters);

			evp_pkey_context_type key_generation_context(EVP_PKEY_CTX_new(parameters.raw(), NULL));

			throw_error_if_not(key_generation_context.get());
			throw_error_if(EVP_PKEY_keygen_init(key_generation_context.get()) != 1);

			EVP_PKEY* private_key = nullptr;
			throw_error_if(EVP_PKEY_keygen(key_generation_context.get(), &private_key) != 1);
			::EC_KEY_set_asn1_flag(EVP_PKEY_get1_EC_KEY(private_key), OPENSSL_EC_NAMED_CURVE);
			m_private_key = pkey::take_ownership(private_key);
		}

		buffer ecdhe_context::get_public_key()
		{
			if (!m_private_key)
			{
				generate_keys();
			}

			bio::bio_chain bio(::BIO_new(::BIO_s_mem()));
			m_private_key.write_certificate_public_key(bio.first());

			char* buf = nullptr;
			const size_t buf_len = bio.first().get_mem_data(buf);

			return buffer(buf, buf_len);
		}

		buffer ecdhe_context::derive_secret_key(const void* peer_key, size_t peer_key_len)
		{
			if (!m_private_key)
			{
				generate_keys();
			}

			bio::bio_chain bio(::BIO_new_mem_buf(const_cast<void*>(peer_key), static_cast<int>(peer_key_len)));
			pkey peer_pkey = pkey::from_certificate_public_key(bio.first());

			evp_pkey_context_type key_derivation_context(::EVP_PKEY_CTX_new(m_private_key.raw(), NULL));

			throw_error_if_not(key_derivation_context.get());
			throw_error_if(::EVP_PKEY_derive_init(key_derivation_context.get()) != 1);

			throw_error_if(::EVP_PKEY_derive_set_peer(key_derivation_context.get(), peer_pkey.raw()) != 1);

			size_t buf_len = 0;

			throw_error_if(::EVP_PKEY_derive(key_derivation_context.get(), NULL, &buf_len) != 1);

			buffer buf(buf_len);

			throw_error_if(::EVP_PKEY_derive(key_derivation_context.get(), buffer_cast<uint8_t*>(buf), &buf_len) != 1);

			return buf;
		}
	}
}


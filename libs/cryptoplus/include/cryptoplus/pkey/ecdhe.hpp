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
 * \file ecdhe.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Elliptical Curve Diffie-Hellman Ephemeral context class.
 */

#ifndef CRYPTOPLUS_PKEY_ECDHE_HPP
#define CRYPTOPLUS_PKEY_ECDHE_HPP

#include "pkey.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/obj_mac.h>

#include <memory>

namespace cryptoplus
{
	namespace pkey
	{
		class ecdhe_context
		{
			public:

				/**
				 * \brief Create a new context with the specified elliptic curve NID.
				 *
				 * See <openssl/obj_mac.h> for a list of possible NIDs.
				 */
				explicit ecdhe_context(int nid);

				ecdhe_context(const ecdhe_context&) = delete;
				ecdhe_context& operator=(const ecdhe_context&) = delete;

				/**
				 * \brief Generate new keys for the context.
				 */
				void generate_keys();

				/**
				 * \brief Get the internal public key, generating one if none exists yet.
				 * \return The public key.
				 */
				buffer get_public_key();

				/**
				 * \brief Derive the secret key from a given peer public key.
				 * \param peer_key The peer key buffer.
				 * \param peer_key_len The length of the peer key buffer.
				 * \return The buffer.
				 */
				buffer derive_secret_key(const void* peer_key, size_t peer_key_len);

				/**
				 * \brief Derive the secret key from a given peer public key.
				 * \param peer_key The peer key buffer.
				 * \return The buffer.
				 */
				template <typename BufferType>
				buffer derive_secret_key(const BufferType& peer_key)
				{
					return derive_secret_key(buffer_cast<const char*>(peer_key), buffer_size(peer_key));
				}

			private:

				int m_nid;
				pkey m_private_key;
		};
	}
}

#endif /* CRYPTOPLUS_PKEY_ECDHE_HPP */


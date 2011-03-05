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
 * \file cipher_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher context class.
 */

#ifndef CRYPTOPEN_HASH_CIPHER_CONTEXT_HPP
#define CRYPTOPEN_HASH_CIPHER_CONTEXT_HPP

#include "../error/cryptographic_exception.hpp"
#include "cipher_algorithm.hpp"

#include <openssl/evp.h>

#include <boost/noncopyable.hpp>

#include <vector>

namespace cryptopen
{
	namespace cipher
	{
		/**
		 * \brief A cipher context class.
		 *
		 * The cipher_context class ease the computation of a cipher.
		 *
		 * The list of the available cipher methods depends on the version of OpenSSL and can be found on the man page of EVP_EncryptInit().
		 *
		 * cipher_context is noncopyable by design.
		 */
		class cipher_context : public boost::noncopyable
		{
			public:

				/**
				 * \brief The cipher direction.
				 */
				enum cipher_direction
				{
					unchanged = -1, /** \brief The direction did not change since the last call. */
					decrypt = 0, /**< \brief Specifies a decryption cipher context. */
					encrypt = 1 /**< \brief Specifies an encryption cipher context. */
				};

				/**
				 * \brief Create a new cipher_context.
				 */
				cipher_context();

				/**
				 * \brief Destroy a cipher_context.
				 *
				 * Calls EVP_CIPHER_CTX_cleanup() on the internal EVP_CIPHER_CTX.
				 */
				~cipher_context();

				/**
				 * \brief Initialize the cipher_context.
				 * \param algorithm The cipher algorithm to use.
				 * \param direction The direction of the cipher_context. If a previous call to initialize() was done, you may specify cipher_direction::unchanged to keep the same direction value.
				 * \param key The key to use. Must match algorithm.key_length(). Cannot be NULL.
				 * \param iv The iv to use (if one is needed for the specified algorithm, NULL otherwise). Must match algorithm.iv_length().
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_EncryptInit().
				 */
				void initialize(const cipher_algorithm& algorithm, cipher_direction direction, const void* key, const void* iv, ENGINE* impl = NULL);

				/**
				 * \brief Update the cipher_context with some data.
				 * \param out The output buffer. Should be at least in_len + algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer. Will be updated to indicate the written bytes count.
				 * \param in The input buffer.
				 * \param in_len The length of the in buffer.
				 */
				void update(void* out, size_t& out_len, const void* in, size_t in_len);

				/**
				 * \brief Finalize the cipher_context and get the resulting buffer.
				 * \param out The output buffer. Should be at least algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer. Will be updated to indicate the written bytes count.
				 * \return The number of bytes written or 0 on failure.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				void finalize(void* out, size_t& out_len);

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				EVP_CIPHER_CTX& raw();

				/**
				 * \brief Get the associated cipher algorithm.
				 * \return The associated cipher algorithm. If no call to initialize was done, the behavior is undefined.
				 */
				cipher_algorithm algorithm() const;

			private:

				EVP_CIPHER_CTX m_ctx;
		};

		inline cipher_context::cipher_context()
		{
			EVP_CIPHER_CTX_init(&m_ctx);
		}

		inline cipher_context::~cipher_context()
		{
			EVP_CIPHER_CTX_cleanup(&m_ctx);
		}

		inline EVP_CIPHER_CTX& cipher_context::raw()
		{
			return m_ctx;
		}

		inline cipher_algorithm cipher_context::algorithm() const
		{
			return cipher_algorithm(EVP_CIPHER_CTX_cipher(&m_ctx));
		}
	}
}

#endif /* CRYPTOPEN_HASH_CIPHER_CONTEXT_HPP */


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
 * \file hmac_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A HMAC context class.
 */

#ifndef CRYPTOPLUS_HASH_HMAC_CONTEXT_HPP
#define CRYPTOPLUS_HASH_HMAC_CONTEXT_HPP

#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "message_digest_algorithm.hpp"

#include <openssl/opensslv.h>
#include <openssl/hmac.h>

#include <boost/noncopyable.hpp>

namespace cryptoplus
{
	namespace hash
	{
		/**
		 * \brief A HMAC context class.
		 *
		 * The hmac_context class ease the computation of a HMAC.
		 *
		 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
		 *
		 * A hmac_context is non-copyable by design.
		 */
		class hmac_context : public boost::noncopyable
		{
			public:

				/**
				 * \brief Create a new hmac_context.
				 */
				hmac_context();

				/**
				 * \brief Destroy a hmac_context.
				 *
				 * Calls HMAC_CTX_cleanup() on the internal HMAC_CTX.
				 */
				~hmac_context();

				/**
				 * \brief Initialize the hmac_context.
				 * \param key The key to use. If key is NULL, the previously used key is taken.
				 * \param key_len The key length. If key is NULL, key_len is not used.
				 * \param algorithm The message digest algorithm to use. If algorithm is NULL, then the previously specified algorithm is reused.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void initialize(const void* key, size_t key_len, const message_digest_algorithm* algorithm, ENGINE* impl = NULL);

				/**
				 * \brief Update the hmac_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void update(const void* data, size_t len);

				/**
				 * \brief Finalize the hmac_context and get the resulting buffer.
				 * \param md The resulting buffer. Cannot be NULL.
				 * \param len The length of md.
				 * \return The number of bytes written or 0 on failure.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				size_t finalize(void* md, size_t len);

				/**
				 * \brief Finalize the hmac_context and get the resulting buffer.
				 * \return The resulting buffer.
				 */
				buffer finalize();

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				const HMAC_CTX& raw() const;

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				HMAC_CTX& raw();

				/**
				 * \brief Get the associated message digest algorithm.
				 * \return The associated message digest algorithm. If no call to initialize() was done to specify a message digest algorithm, the behavior is undefined.
				 */
				message_digest_algorithm algorithm() const;

			private:

				HMAC_CTX* m_ctx;
		};

		inline hmac_context::hmac_context()
		{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
			m_ctx = new HMAC_CTX;
			HMAC_CTX_init(m_ctx);
#else
			m_ctx = HMAC_CTX_new();
#endif
		}

		inline hmac_context::~hmac_context()
		{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
			HMAC_CTX_cleanup(m_ctx);
			delete m_ctx;
#else
			HMAC_CTX_free(m_ctx);
#endif
		}

		inline void hmac_context::update(const void* data, size_t len)
		{
#if OPENSSL_VERSION_NUMBER < 0x01000000
			HMAC_Update(m_ctx, static_cast<const unsigned char*>(data), static_cast<int>(len));
#else
			throw_error_if_not(HMAC_Update(m_ctx, static_cast<const unsigned char*>(data), static_cast<int>(len)) != 0);
#endif
		}

		inline buffer hmac_context::finalize()
		{
			buffer result(algorithm().result_size());

			finalize(buffer_cast<uint8_t*>(result), buffer_size(result));

			return result;
		}

		inline const HMAC_CTX& hmac_context::raw() const
		{
			return *m_ctx;
		}

		inline HMAC_CTX& hmac_context::raw()
		{
			return *m_ctx;
		}

		inline message_digest_algorithm hmac_context::algorithm() const
		{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
			//WARNING: Here we directly use the undocumented HMAC_CTX.md field.
			return message_digest_algorithm(m_ctx->md);
#else
			return HMAC_CTX_get_md(m_ctx);
#endif
		}
	}
}

#endif /* CRYPTOPLUS_HASH_HMAC_CONTEXT_HPP */


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
 * \file message_digest_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A HMAC context class.
 */

#ifndef CRYPTOPEN_HASH_MESSAGE_DIGEST_CONTEXT_HPP
#define CRYPTOPEN_HASH_MESSAGE_DIGEST_CONTEXT_HPP

#include "../error/cryptographic_exception.hpp"

#include <openssl/evp.h>

#include <boost/noncopyable.hpp>

#include <vector>

namespace cryptopen
{
	namespace hash
	{
		/**
		 * \brief A message digest context class.
		 *
		 * The message_digest_context class ease the computation of a message digest (hash).
		 *
		 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
		 *
		 * message_digest_context is noncopyable by design, however you may copy an existing message_digest_context using copy(). This is useful when you have to compute the hash of several values that differ only in their last bytes.
		 */
		class message_digest_context : public boost::noncopyable
		{
			public:

				/**
				 * \brief Create a new message_digest_context.
				 */
				message_digest_context();

				/**
				 * \brief Destroy a message_digest_context.
				 *
				 * Calls EVP_MD_CTX_cleanup() on the internal EVP_MD_CTX.
				 */
				~message_digest_context();

				/**
				 * \brief Initialize the message_digest_context.
				 * \param md The message digest (hash) method to use. md cannot be NULL.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void initialize(const EVP_MD* md, ENGINE* impl = NULL);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void update(const void* data, size_t len);

				/**
				 * \brief Finalize the message_digest_context and get the resulting buffer.
				 * \param md The resulting buffer. Cannot be NULL.
				 * \param len The length of md.
				 * \return The number of bytes written or 0 on failure.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				size_t finalize(void* md, size_t len);

				/**
				 * \brief Finalize the message_digest_context and get the resulting buffer.
				 * \return The resulting buffer.
				 */
				template <typename T>
				std::vector<T> finalize();

				/**
				 * \brief Copy an existing message_digest_context, including its current state.
				 * \param ctx A message_digest_context to copy.
				 *
				 * This is useful if large amounts of data are to be hashed which only differ in the last few bytes.
				 */
				void copy(const message_digest_context& ctx);

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				EVP_MD_CTX& raw();

				/**
				 * \brief Get the associated message digest method.
				 * \return The associated message digest method. Might be NULL if no call to initialize() was done.
				 */
				const EVP_MD* message_digest_method() const;

				/**
				 * \brief Get the resulting message digest size.
				 * \return The resulting message digest size.
				 * \warning If no call initialize() was done to set a valid message digest method, the behavior is undefined.
				 */
				size_t message_digest_size() const;

				/**
				 * \brief Get the resulting message digest block size.
				 * \return The resulting message digest block size.
				 * \warning If no call initialize() was done to set a valid message digest method, the behavior is undefined.
				 */
				size_t message_digest_block_size() const;

				/**
				 * \brief Get the NID of the OBJECT IDENTIFIER representing the current message digest.
				 * \return The NID.
				 * \warning If no call initialize() was done to set a valid message digest method, the behavior is undefined.
				 */
				int message_digest_type() const;

				/**
				 * \brief Get the NID of the public key signing algorithm associated with the message digest.
				 * \warning The use of this method is discouraged as it may disappear in future versions of OpenSSL.
				 */
				int message_digest_public_key_type() const;

			private:

				EVP_MD_CTX m_ctx;
		};

		inline message_digest_context::message_digest_context()
		{
			EVP_MD_CTX_init(&m_ctx);
		}

		inline message_digest_context::~message_digest_context()
		{
			EVP_MD_CTX_cleanup(&m_ctx);
		}

		inline void message_digest_context::update(const void* data, size_t len)
		{
			error::throw_error_if_not(EVP_DigestUpdate(&m_ctx, data, len));
		}

		template <typename T>
		inline std::vector<T> message_digest_context::finalize()
		{
			std::vector<T> result(message_digest_size());

			finalize(&result[0], result.size());

			return result;
		}

		inline void message_digest_context::copy(const message_digest_context& ctx)
		{
			error::throw_error_if_not(EVP_MD_CTX_copy_ex(&m_ctx, &ctx.m_ctx));
		}

		inline EVP_MD_CTX& message_digest_context::raw()
		{
			return m_ctx;
		}

		inline const EVP_MD* message_digest_context::message_digest_method() const
		{
			return EVP_MD_CTX_md(&m_ctx);
		}

		inline size_t message_digest_context::message_digest_size() const
		{
			return EVP_MD_CTX_size(&m_ctx);
		}
		
		inline size_t message_digest_context::message_digest_block_size() const
		{
			return EVP_MD_CTX_block_size(&m_ctx);
		}

		inline int message_digest_context::message_digest_type() const
		{
			return EVP_MD_CTX_type(&m_ctx);
		}
		
		inline int message_digest_context::message_digest_public_key_type() const
		{
			return EVP_MD_pkey_type(message_digest_method());
		}
	}
}

#endif /* CRYPTOPEN_HASH_MESSAGE_DIGEST_CONTEXT_HPP */


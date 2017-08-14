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
 * \file message_digest_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A message digest context class.
 */

#ifndef CRYPTOPLUS_HASH_MESSAGE_DIGEST_CONTEXT_HPP
#define CRYPTOPLUS_HASH_MESSAGE_DIGEST_CONTEXT_HPP

#include "../error/helpers.hpp"
#include "message_digest_algorithm.hpp"
#include "../pkey/pkey.hpp"

#include <openssl/evp.h>

namespace cryptoplus
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
		class message_digest_context
		{
			public:

				/**
				 * \brief Create a new message_digest_context.
				 */
				message_digest_context()
				{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
					m_ctx = new EVP_MD_CTX;
					EVP_MD_CTX_init(m_ctx);
#else
					m_ctx = EVP_MD_CTX_new();
#endif
				}

				/**
				 * \brief Copy a message_digest_context.
				 * \param other The other instance.
				 */
				message_digest_context(const message_digest_context& other) : message_digest_context()
				{
					copy(other);
				}

				/**
				 * \brief Destroy a message_digest_context.
				 *
				 * Calls EVP_MD_CTX_cleanup() on the internal EVP_MD_CTX.
				 */
				~message_digest_context()
				{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
					EVP_MD_CTX_cleanup(m_ctx);
					delete m_ctx;
#else
					EVP_MD_CTX_free(m_ctx);
#endif
				}

				/**
				 * \brief Assign a message_digest_context to this instance.
				 * \param other The other instance.
				 * \return The current modified instance.
				 */
				message_digest_context& operator=(const message_digest_context& other)
				{
					if (&other != this)
					{
						copy(other);
					}

					return *this;
				}

				/**
				 * \brief Initialize the message_digest_context.
				 * \param algorithm The message digest algorithm to use.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void initialize(const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

				/**
				 * \brief Initialize the message_digest_context for signing.
				 * \param algorithm The message digest algorithm to use.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void sign_initialize(const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

				/**
				 * \brief Initialize the message_digest_context for signature verification.
				 * \param algorithm The message digest algorithm to use.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void verify_initialize(const message_digest_algorithm& algorithm, ENGINE* impl = NULL);

				/**
				 * \brief Initialize the message_digest_context for digest signing.
				 * \param algorithm The message digest algorithm to use.
				 * \param key The pkey to use.
				 * \param pctx If not NULL, *pctx will point to the signing operation context.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void digest_sign_initialize(const message_digest_algorithm& algorithm, const pkey::pkey& key, EVP_PKEY_CTX** pctx = NULL, ENGINE* impl = NULL);

				/**
				 * \brief Initialize the message_digest_context for digest signature verification.
				 * \param algorithm The message digest algorithm to use.
				 * \param key The pkey to use.
				 * \param pctx If not NULL, *pctx will point to the signing operation context.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
				 */
				void digest_verify_initialize(const message_digest_algorithm& algorithm, const pkey::pkey& key, EVP_PKEY_CTX** pctx = NULL, ENGINE* impl = NULL);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void update(const void* data, size_t len);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void sign_update(const void* data, size_t len);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void verify_update(const void* data, size_t len);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void digest_sign_update(const void* data, size_t len);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param data The data buffer.
				 * \param len The data length.
				 */
				void digest_verify_update(const void* data, size_t len);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param buf The data buffer.
				 */
				void update(const buffer& buf);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param buf The data buffer.
				 */
				void sign_update(const buffer& buf);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param buf The data buffer.
				 */
				void verify_update(const buffer& buf);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param buf The data buffer.
				 */
				void digest_sign_update(const buffer& buf);

				/**
				 * \brief Update the message_digest_context with some data.
				 * \param buf The data buffer.
				 */
				void digest_verify_update(const buffer& buf);

				/**
				 * \brief Finalize the message_digest_context and get the resulting buffer.
				 * \param md The resulting buffer. Cannot be NULL.
				 * \param md_len The length of md.
				 * \return The number of bytes written or 0 on failure.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				size_t finalize(void* md, size_t md_len);

				/**
				 * \brief Finalize the message_digest_context and get the resulting buffer.
				 * \return The resulting buffer.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				buffer finalize();

				/**
				 * \brief Finalize the message_digest_context and get the resulting signature.
				 * \param sig The resulting signature. Cannot be NULL. Must be at least pkey->size() bytes long.
				 * \param sig_len The length of sig.
				 * \param pkey The private pkey to use to generate the signature.
				 * \return The number of bytes written.
				 *
				 * After a call to sign_finalize() no more call to sign_update() can be made unless sign_initialize() is called again first.
				 */
				size_t sign_finalize(void* sig, size_t sig_len, pkey::pkey& pkey);

				/**
				 * \brief Finalize the message_digest_context and get the resulting signature.
				 * \param pkey The pkey to use to generate the signature.
				 * \return The resulting signature.
				 *
				 * After a call to sign_finalize() no more call to sign_update() can be made unless sign_initialize() is called again first.
				 */
				buffer sign_finalize(pkey::pkey& pkey);

				/**
				 * \brief Finalize the message_digest_context and compare its resulting signature to the specified signature.
				 * \param sig The signature to compare to. Cannot be NULL.
				 * \param sig_len The length of sig.
				 * \param pkey The public pkey to use to verify the signature.
				 * \return true if the signature matches, false otherwise.
				 *
				 * After a call to verify_finalize() no more call to verify_update() can be made unless verify_initialize() is called again first.
				 */
				bool verify_finalize(const void* sig, size_t sig_len, pkey::pkey& pkey);

				/**
				 * \brief Finalize the message_digest_context and compare its resulting signature to the specified signature.
				 * \param sig The signature to compare to.
				 * \param pkey The public pkey to use to verify the signature.
				 * \return true if the signature matches, false otherwise.
				 *
				 * After a call to verify_finalize() no more call to verify_update() can be made unless verify_initialize() is called again first.
				 */
				bool verify_finalize(const buffer& sig, pkey::pkey& pkey);

				/**
				 * \brief Finalize the message_digest_context and get the resulting signature.
				 * \param sig The resulting signature. If NULL, the required size will be returned.
				 * \param sig_len The length of sig.
				 * \return The number of bytes written.
				 *
				 * After a call to sign_finalize() no more call to sign_update() can be made unless sign_initialize() is called again first.
				 */
				size_t digest_sign_finalize(void* sig, size_t sig_len);

				/**
				 * \brief Finalize the message_digest_context and get the resulting signature.
				 * \return The resulting signature.
				 *
				 * After a call to sign_finalize() no more call to sign_update() can be made unless sign_initialize() is called again first.
				 */
				buffer digest_sign_finalize();

				/**
				 * \brief Finalize the message_digest_context and compare its resulting signature to the specified signature.
				 * \param sig The signature to compare to. Cannot be NULL.
				 * \param sig_len The length of sig.
				 * \return true if the signature matches, false otherwise.
				 *
				 * After a call to digest_verify_finalize() no more call to digest_verify_update() can be made unless digest_verify_initialize() is called again first.
				 */
				bool digest_verify_finalize(const void* sig, size_t sig_len);

				/**
				 * \brief Finalize the message_digest_context and compare its resulting signature to the specified signature.
				 * \param sig The signature to compare to.
				 * \return true if the signature matches, false otherwise.
				 *
				 * After a call to digest_verify_finalize() no more call to digest_verify_update() can be made unless digest_verify_initialize() is called again first.
				 */
				bool digest_verify_finalize(const buffer& sig);

				/**
				 * \brief Copy an existing message_digest_context, including its current state.
				 * \param ctx A message_digest_context to copy.
				 *
				 * This is useful if large amounts of data are to be hashed which only differ in the last few bytes.
				 */
				void copy(const message_digest_context& ctx);

				/**
				 * \brief Set the flags on the message digest context.
				 * \param flags The flags to set.
				 */
				void set_flags(int flags);

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				const EVP_MD_CTX& raw() const;

				/**
				 * \brief Get the underlying context.
				 * \return The underlying context.
				 * \warning This method is provided for compatibility issues only. Its use is greatly discouraged.
				 */
				EVP_MD_CTX& raw();

				/**
				 * \brief Get the associated message digest algorithm.
				 * \return The associated message digest algorithm. If no call to initialize was done, the behavior is undefined.
				 */
				message_digest_algorithm algorithm() const;

			private:

				EVP_MD_CTX* m_ctx;
		};

		inline void message_digest_context::initialize(const message_digest_algorithm& _algorithm, ENGINE* impl)
		{
			throw_error_if_not(EVP_DigestInit_ex(m_ctx, _algorithm.raw(), impl) == 1);
		}

		inline void message_digest_context::sign_initialize(const message_digest_algorithm& _algorithm, ENGINE* impl)
		{
			throw_error_if_not(EVP_SignInit_ex(m_ctx, _algorithm.raw(), impl) == 1);
		}

		inline void message_digest_context::verify_initialize(const message_digest_algorithm& _algorithm, ENGINE* impl)
		{
			throw_error_if_not(EVP_VerifyInit_ex(m_ctx, _algorithm.raw(), impl) == 1);
		}

		inline void message_digest_context::digest_sign_initialize(const message_digest_algorithm& _algorithm, const pkey::pkey& key, EVP_PKEY_CTX** pctx, ENGINE* impl)
		{
			throw_error_if_not(EVP_DigestSignInit(m_ctx, pctx, _algorithm.raw(), impl, const_cast<EVP_PKEY*>(key.raw())) == 1);
		}

		inline void message_digest_context::digest_verify_initialize(const message_digest_algorithm& _algorithm, const pkey::pkey& key, EVP_PKEY_CTX** pctx, ENGINE* impl)
		{
			throw_error_if_not(EVP_DigestVerifyInit(m_ctx, pctx, _algorithm.raw(), impl, const_cast<EVP_PKEY*>(key.raw())) == 1);
		}

		inline void message_digest_context::update(const void* data, size_t len)
		{
			throw_error_if_not(EVP_DigestUpdate(m_ctx, data, len) != 0);
		}

		inline void message_digest_context::sign_update(const void* data, size_t len)
		{
			throw_error_if_not(EVP_SignUpdate(m_ctx, data, len) != 0);
		}

		inline void message_digest_context::verify_update(const void* data, size_t len)
		{
			throw_error_if_not(EVP_VerifyUpdate(m_ctx, data, len) != 0);
		}

		inline void message_digest_context::digest_sign_update(const void* data, size_t len)
		{
			throw_error_if_not(EVP_DigestSignUpdate(m_ctx, data, len) != 0);
		}

		inline void message_digest_context::digest_verify_update(const void* data, size_t len)
		{
			throw_error_if_not(EVP_DigestVerifyUpdate(m_ctx, data, len) != 0);
		}

		inline void message_digest_context::update(const buffer& buf)
		{
			update(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline void message_digest_context::sign_update(const buffer& buf)
		{
			sign_update(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline void message_digest_context::verify_update(const buffer& buf)
		{
			verify_update(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline void message_digest_context::digest_sign_update(const buffer& buf)
		{
			digest_sign_update(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline void message_digest_context::digest_verify_update(const buffer& buf)
		{
			digest_verify_update(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline buffer message_digest_context::finalize()
		{
			buffer result(algorithm().result_size());

			finalize(buffer_cast<uint8_t*>(result), buffer_size(result));

			return result;
		}

		inline buffer message_digest_context::sign_finalize(pkey::pkey& pkey)
		{
			buffer result(pkey.size());

			sign_finalize(buffer_cast<uint8_t*>(result), buffer_size(result), pkey);

			return result;
		}

		inline bool message_digest_context::verify_finalize(const buffer& sig, pkey::pkey& pkey)
		{
			return verify_finalize(buffer_cast<const uint8_t*>(sig), buffer_size(sig), pkey);
		}

		inline buffer message_digest_context::digest_sign_finalize()
		{
			buffer result(digest_sign_finalize(nullptr, 0));

			digest_sign_finalize(buffer_cast<uint8_t*>(result), buffer_size(result));

			return result;
		}

		inline bool message_digest_context::digest_verify_finalize(const buffer& sig)
		{
			return digest_verify_finalize(buffer_cast<const uint8_t*>(sig), buffer_size(sig));
		}

		inline void message_digest_context::copy(const message_digest_context& ctx)
		{
			throw_error_if_not(EVP_MD_CTX_copy_ex(m_ctx, ctx.m_ctx) != 0);
		}

		inline void message_digest_context::set_flags(int flags)
		{
			EVP_MD_CTX_set_flags(m_ctx, flags);
		}

		inline const EVP_MD_CTX& message_digest_context::raw() const
		{
			return *m_ctx;
		}

		inline EVP_MD_CTX& message_digest_context::raw()
		{
			return *m_ctx;
		}

		inline message_digest_algorithm message_digest_context::algorithm() const
		{
			return message_digest_algorithm(EVP_MD_CTX_md(m_ctx));
		}
	}
}

#endif /* CRYPTOPLUS_HASH_MESSAGE_DIGEST_CONTEXT_HPP */


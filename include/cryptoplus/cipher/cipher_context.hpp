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
 * \file cipher_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher context class.
 */

#ifndef CRYPTOPLUS_CIPHER_CIPHER_CONTEXT_HPP
#define CRYPTOPLUS_CIPHER_CIPHER_CONTEXT_HPP

#include "../error/cryptographic_exception.hpp"
#include "cipher_algorithm.hpp"

#include <openssl/evp.h>

#include <boost/noncopyable.hpp>

#include <vector>

namespace cryptoplus
{
	namespace pkey
	{
		class pkey;
	}

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
				    unchanged = -1, /**< \brief The direction did not change since the last call. */
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
				 * \param key The key to use. Cannot be NULL.
				 * \param key_len The length of key. Must match algorithm.key_length() or a std::runtime_error is thrown.
				 * \param iv The iv to use (if one is needed for the specified algorithm, NULL otherwise).
				 * \param iv_len The length of iv. Must match algorithm.iv_length() or a std::runtime_error is thrown.
				 * \param impl The engine to use. Default is NULL which indicates that no engine should be used.
				 * \see set_padding
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_EncryptInit().
				 *
				 * Once the cipher_context is initialized, you may enable or disable PKCS padding by calling set_padding(). By default, PKCS padding is enabled.
				 */
				void initialize(const cipher_algorithm& algorithm, cipher_direction direction, const void* key, size_t key_len, const void* iv, size_t iv_len, ENGINE* impl = NULL);

				/**
				 * \brief Initialize the cipher_context for envelope sealing.
				 * \param algorithm The cipher algorithm to use.
				 * \param iv The iv that was generated (if one is needed for the specified algorithm, NULL otherwise).
				 * \param iv_len The length of iv. Must match algorithm.iv_length() or a std::runtime_error is thrown.
				 * \param pkeys_begin A pointer to the first public pkey to use.
				 * \param pkeys_end A pointer past the last public pkey to use.
				 * \return The public encrypted shared secret keys array.
				 * \see seal_update
				 * \see seal_finalize
				 */
				template <typename T>
				std::vector<std::vector<unsigned char> > seal_initialize(const cipher_algorithm& algorithm, void* iv, size_t iv_len, T pkeys_begin, T pkeys_end);

				/**
				 * \brief Initialize the cipher_context for envelope sealing.
				 * \param algorithm The cipher algorithm to use.
				 * \param iv The iv that was generated (if one is needed for the specified algorithm, NULL otherwise).
				 * \param iv_len The length of iv. Must match algorithm.iv_length() or a std::runtime_error is thrown.
				 * \param pkey The public pkey to use.
				 * \return The public encrypted shared secret key.
				 * \see seal_update
				 * \see seal_finalize
				 */
				std::vector<unsigned char> seal_initialize(const cipher_algorithm& algorithm, void* iv, size_t iv_len, pkey::pkey pkey);

				/**
				 * \brief Initialize the cipher_context for envelope opening.
				 * \param algorithm The cipher algorithm to use.
				 * \param key The encrypted shared secret key.
				 * \param key_len The length of key. Must match algorithm.key_length() or a std::runtime_error is thrown.
				 * \param iv The iv to use (if one is needed for the specified algorithm, NULL otherwise).
				 * \param iv_len The length of iv. Must match algorithm.iv_length() or a std::runtime_error is thrown.
				 * \param pkey The private pkey to use.
				 * \see open_update
				 * \see open_finalize
				 */
				void open_initialize(const cipher_algorithm& algorithm, const void* key, size_t key_len, const void* iv, size_t iv_len, pkey::pkey pkey);

				/**
				 * \brief Set PKCS padding state.
				 * \param enabled If enabled is true, PKCS padding will be enabled.
				 * \see cipher_algorithm::block_size
				 *
				 * If PKCS padding is disabled, the input data size must be an exact multiple of the specified algorithm block size. See cipher_algorithm::block_size().
				 */
				void set_padding(bool enabled);

				/**
				 * \brief Get the key length.
				 * \return The key length.
				 * \see set_key_length
				 *
				 * For fixed key length ciphers, returns the same value as algorithm().key_length().
				 * For variable key length ciphers, returns the current used key length value. See set_key_length().
				 */
				size_t key_length() const;

				/**
				 * \brief Set the current key length for variable key length ciphers.
				 * \param len The new key length.
				 * \warning Attempting to set the key length of a fixed key length cipher is an error.
				 */
				void set_key_length(size_t len);

				/**
				 * \brief Get cipher specific parameters.
				 * \param type The type.
				 * \param value The value to get.
				 */
				template <typename T>
				void ctrl_get(int type, T& value);

				/**
				 * \brief Set cipher specific parameters.
				 * \param type The type.
				 * \param value The value to set.
				 */
				void ctrl_set(int type, int value);

				/**
				 * \brief Update the cipher_context with some data.
				 * \param out The output buffer. Should be at least in_len + algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer.
				 * \param in The input buffer.
				 * \param in_len The length of the in buffer.
				 * \return The count of bytes written.
				 */
				size_t update(void* out, size_t out_len, const void* in, size_t in_len);

				/**
				 * \brief Update the cipher_context with some data.
				 * \param out The output buffer. Should be at least in_len + algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer. Will be updated to indicate the written bytes count.
				 * \param in The input buffer.
				 * \param in_len The length of the in buffer.
				 * \return The count of bytes written.
				 */
				size_t seal_update(void* out, size_t out_len, const void* in, size_t in_len);

				/**
				 * \brief Update the cipher_context with some data.
				 * \param out The output buffer. Should be at least in_len + algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer. Will be updated to indicate the written bytes count.
				 * \param in The input buffer.
				 * \param in_len The length of the in buffer.
				 * \return The count of bytes written.
				 */
				size_t open_update(void* out, size_t out_len, const void* in, size_t in_len);

				/**
				 * \brief Finalize the cipher_context and get the resulting buffer.
				 * \param out The output buffer. Should be at least algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer.
				 * \return The count of bytes written.
				 *
				 * After a call to finalize() no more call to update() can be made unless initialize() is called again first.
				 */
				size_t finalize(void* out, size_t out_len);

				/**
				 * \brief Finalize the cipher_context and get the resulting buffer.
				 * \param out The output buffer. Should be at least algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer.
				 * \return The count of bytes written.
				 *
				 * After a call to seal_finalize() no more call to seal_update() can be made unless seal_initialize() is called again first.
				 */
				size_t seal_finalize(void* out, size_t out_len);

				/**
				 * \brief Finalize the cipher_context and get the resulting buffer.
				 * \param out The output buffer. Should be at least algorithm().block_size() bytes long. Cannot be NULL.
				 * \param out_len The length of the out buffer.
				 * \return The count of bytes written.
				 *
				 * After a call to open_finalize() no more call to open_update() can be made unless open_initialize() is called again first.
				 */
				size_t open_finalize(void* out, size_t out_len);

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

		template <typename T>
		inline std::vector<std::vector<unsigned char> > cipher_context::seal_initialize(const cipher_algorithm& _algorithm, void* iv, size_t iv_len, T pkeys_begin, T pkeys_end)
		{
			if (iv && (iv_len != _algorithm.iv_length()))
			{
				throw std::runtime_error("iv_len");
			}

			size_t pkeys_count = std::distance(pkeys_begin, pkeys_end);

			std::vector<std::vector<unsigned char> > result;
			std::vector<unsigned char*> ek;
			std::vector<int> ekl(pkeys_count);
			std::vector<EVP_PKEY*> pubk;

			ek.reserve(pkeys_count);
			pubk.reserve(pkeys_count);
			result.reserve(pkeys_count);

			try
			{
				for (T pkey = pkeys_begin; pkey != pkeys_end; ++pkey)
				{
					ek.push_back(new unsigned char[pkey->size()]);
					pubk.push_back(pkey->raw());
				}

				error::throw_error_if_not(EVP_SealInit(&m_ctx, _algorithm.raw(), &ek[0], &ekl[0], static_cast<unsigned char*>(iv), &pubk[0], static_cast<int>(pkeys_count)) != 0);

				for (std::vector<unsigned char*>::iterator p = ek.begin(); p != ek.end(); ++p)
				{
					result.push_back(std::vector<unsigned char>(*p, *p + ekl[std::distance(ek.begin(), p)]));
				}
			}
			catch (...)
			{
				for (std::vector<unsigned char*>::iterator p = ek.begin(); p != ek.end(); ++p)
				{
					delete[] *p;
				}

				throw;
			}

			for (std::vector<unsigned char*>::iterator p = ek.begin(); p != ek.end(); ++p)
			{
				delete[] *p;
			}

			return result;
		}

		inline void cipher_context::set_padding(bool enabled)
		{
			// The call always returns 1 so testing its return value is useless.
			EVP_CIPHER_CTX_set_padding(&m_ctx, static_cast<int>(enabled));
		}

		inline size_t cipher_context::key_length() const
		{
			return EVP_CIPHER_CTX_key_length(&m_ctx);
		}

		inline void cipher_context::set_key_length(size_t len)
		{
			error::throw_error_if_not(EVP_CIPHER_CTX_set_key_length(&m_ctx, static_cast<int>(len)) != 0);
		}

		template <typename T>
		inline void cipher_context::ctrl_get(int type, T& value)
		{
			error::throw_error_if_not(EVP_CIPHER_CTX_ctrl(&m_ctx, type, 0, &value) != 0);
		}

		inline void cipher_context::ctrl_set(int type, int value)
		{
			error::throw_error_if_not(EVP_CIPHER_CTX_ctrl(&m_ctx, type, value, NULL) != 0);
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

#endif /* CRYPTOPLUS_CIPHER_CIPHER_CONTEXT_HPP */


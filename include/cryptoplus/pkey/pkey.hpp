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
 * \file pkey.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An EVP_PKEY class.
 */

#ifndef CRYPTOPEN_PKEY_PKEY_HPP
#define CRYPTOPEN_PKEY_PKEY_HPP

#include "../error/cryptographic_exception.hpp"
#include "rsa_key.hpp"
#include "dsa_key.hpp"
#include "dh_key.hpp"

#include <boost/shared_ptr.hpp>

namespace cryptoplus
{
	namespace pkey
	{
		/**
		 * \brief A EVP_PKEY.
		 *
		 * The pkey class represents an EVP_PKEY structure.
		 * pkey is a low level structure. It allows you to check the type of the contained key but no further check is done when it comes to convert the pkey to one of the native types (aka. rsa_key, dsa_key, dh_key).
		 *
		 * A pkey instance has the same semantic as a EVP_PKEY* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class pkey
		{
			public:

				/**
				 * \brief A generate callback type.
				 */
				typedef void (*generate_callback_type)(int, int, void*);

				/**
				 * \brief A PEM passphrase callback type.
				 */
				typedef int (*pem_passphrase_callback_type)(char*, int, int, void*);

				/**
				 * \brief Load a private EVP_PKEY key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public EVP_PKEY key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a private EVP_PKEY key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_private_key(FILE* file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public EVP_PKEY key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_certificate_public_key(FILE* file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a EVP_PKEY key from a private key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_private_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a EVP_PKEY key from a certificate public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_certificate_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty EVP_PKEY.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				pkey();

				/**
				 * \brief Create a EVP_PKEY by taking ownership of an existing EVP_PKEY* pointer.
				 * \param evp_pkey The EVP_PKEY* pointer. Cannot be NULL.
				 */
				explicit pkey(EVP_PKEY* evp_pkey);

				/**
				 * \brief Write the private EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len);

				/**
				 * \brief Write the private EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL);

				/**
				 * \brief Write the certificate public EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_public_key(bio::bio_ptr bio);

				/**
				 * \brief Write the private EVP_PKEY key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(FILE* file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len);

				/**
				 * \brief Write the private EVP_PKEY key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(FILE* file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL);

				/**
				 * \brief Write the certificate public EVP_PKEY key to a file.
				 * \param file The file.
				 */
				void write_certificate_public_key(FILE* file);

				/**
				 * \brief Get the raw EVP_PKEY pointer.
				 * \return The raw EVP_PKEY pointer.
				 * \warning The instance has ownership of the return pointer. Calling EVP_PKEY_free() on the returned value will result in undefined behavior.
				 */
				EVP_PKEY* raw();

				/**
				 * \brief Get the raw EVP_PKEY pointer.
				 * \return The raw EVP_PKEY pointer.
				 * \warning The instance has ownership of the return pointer. Calling EVP_PKEY_free() on the returned value will result in undefined behavior.
				 */
				const EVP_PKEY* raw() const;

			private:

				explicit pkey(boost::shared_ptr<EVP_PKEY> evp_pkey);

				boost::shared_ptr<EVP_PKEY> m_evp_pkey;
		};

		/**
		 * \brief Compare two pkey instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two pkey instances share the same underlying pointer.
		 */
		bool operator==(const pkey& lhs, const pkey& rhs);

		/**
		 * \brief Compare two pkey instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two pkey instances do not share the same underlying pointer.
		 */
		bool operator!=(const pkey& lhs, const pkey& rhs);

		inline pkey pkey::from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return pkey(boost::shared_ptr<EVP_PKEY>(PEM_read_bio_PrivateKey(bio.raw(), NULL, callback, callback_arg), EVP_PKEY_free));
		}
		inline pkey pkey::from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return pkey(boost::shared_ptr<EVP_PKEY>(PEM_read_bio_PUBKEY(bio.raw(), NULL, callback, callback_arg), EVP_PKEY_free));
		}
		inline pkey pkey::from_private_key(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return pkey(boost::shared_ptr<EVP_PKEY>(PEM_read_PrivateKey(file, NULL, callback, callback_arg), EVP_PKEY_free));
		}
		inline pkey pkey::from_certificate_public_key(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return pkey(boost::shared_ptr<EVP_PKEY>(PEM_read_PUBKEY(file, NULL, callback, callback_arg), EVP_PKEY_free));
		}
		inline pkey::pkey() : m_evp_pkey(EVP_PKEY_new(), EVP_PKEY_free)
		{
			error::throw_error_if_not(m_evp_pkey);
		}
		inline pkey::pkey(EVP_PKEY* evp_pkey) : m_evp_pkey(evp_pkey, EVP_PKEY_free)
		{
			if (!m_evp_pkey)
			{
				throw std::invalid_argument("evp_pkey");
			}
		}
		inline void pkey::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len)
		{
			error::throw_error_if_not(PEM_write_bio_PrivateKey(bio.raw(), m_evp_pkey.get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), passphrase_len, NULL, NULL));
		}
		inline void pkey::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg)
		{
			error::throw_error_if_not(PEM_write_bio_PrivateKey(bio.raw(), m_evp_pkey.get(), algorithm.raw(), NULL, 0, callback, callback_arg));
		}
		inline void pkey::write_certificate_public_key(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_PUBKEY(bio.raw(), m_evp_pkey.get()));
		}
		inline void pkey::write_private_key(FILE* file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len)
		{
			error::throw_error_if_not(PEM_write_PrivateKey(file, m_evp_pkey.get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), passphrase_len, NULL, NULL));
		}
		inline void pkey::write_private_key(FILE* file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg)
		{
			error::throw_error_if_not(PEM_write_PrivateKey(file, m_evp_pkey.get(), algorithm.raw(), NULL, 0, callback, callback_arg));
		}
		inline void pkey::write_certificate_public_key(FILE* file)
		{
			error::throw_error_if_not(PEM_write_PUBKEY(file, m_evp_pkey.get()));
		}
		inline EVP_PKEY* pkey::raw()
		{
			return m_evp_pkey.get();
		}
		inline const EVP_PKEY* pkey::raw() const
		{
			return m_evp_pkey.get();
		}
		inline pkey::pkey(boost::shared_ptr<EVP_PKEY> evp_pkey) : m_evp_pkey(evp_pkey)
		{
			error::throw_error_if_not(m_evp_pkey);
		}
		inline bool operator==(const pkey& lhs, const pkey& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const pkey& lhs, const pkey& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_PKEY_PKEY_HPP */


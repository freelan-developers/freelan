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

#ifndef CRYPTOPLUS_PKEY_PKEY_HPP
#define CRYPTOPLUS_PKEY_PKEY_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"
#include "../file.hpp"
#include "rsa_key.hpp"
#include "dsa_key.hpp"
#include "dh_key.hpp"

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
		class pkey : public pointer_wrapper<EVP_PKEY>
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
				 * \brief Create a new pkey.
				 * \return The pkey.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static pkey create();

				/**
				 * \brief Take ownership of a specified EVP_PKEY pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An pkey.
				 */
				static pkey take_ownership(pointer ptr);

				/**
				 * \brief Create a pkey from a RSA key.
				 * \param key The RSA key.
				 * \return The pkey.
				 */
				static pkey from_rsa_key(rsa_key key);

				/**
				 * \brief Create a pkey from a DSA key.
				 * \param key The DSA key.
				 * \return The pkey.
				 */
				static pkey from_dsa_key(dsa_key key);

				/**
				 * \brief Create a pkey from a DH key.
				 * \param key The DH key.
				 * \return The pkey.
				 */
				static pkey from_dh_key(dh_key key);

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
				static pkey from_private_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public EVP_PKEY key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The pkey.
				 */
				static pkey from_certificate_public_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

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
				 * \brief Create a new EVP_PKEY from a HMAC secret key.
				 * \brief buf Is a buffer that contains the secret key.
				 * \brief buf_len Is the length of buf.
				 * \brief engine Is the engine to use, if any.
				 */
				static pkey from_hmac_key(const void* buf, size_t buf_len, ENGINE* engine = NULL);

				/**
				 * \brief Create a new empty pkey.
				 */
				pkey();

				/**
				 * \brief Create an pkey by *NOT* taking ownership of an existing EVP_PKEY pointer.
				 * \param ptr The EVP_PKEY pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				pkey(pointer ptr);

				/**
				 * \brief Set the associated RSA key.
				 * \param rsa The RSA key.
				 */
				void set_rsa_key(rsa_key rsa) const;

				/**
				 * \brief Get the associated RSA key.
				 * \return The RSA key.
				 * \warning If no key was set, an exception is thrown.
				 */
				rsa_key get_rsa_key() const;

				/**
				 * \brief Take ownership of a RSA key.
				 * \param rsa The RSA key.
				 */
				void assign_rsa_key(RSA* rsa) const;

				/**
				 * \brief Set the associated DSA key.
				 * \param dsa The DSA key.
				 */
				void set_dsa_key(dsa_key dsa) const;

				/**
				 * \brief Get the associated DSA key.
				 * \return The DSA key.
				 * \warning If no key was set, an exception is thrown.
				 */
				dsa_key get_dsa_key() const;

				/**
				 * \brief Take ownership of a DSA key.
				 * \param dsa The DSA key.
				 */
				void assign_dsa_key(DSA* dsa) const;

				/**
				 * \brief Set the associated DH key.
				 * \param dh The DH key.
				 */
				void set_dh_key(dh_key dh) const;

				/**
				 * \brief Get the associated DH key.
				 * \return The DH key.
				 * \warning If no key was set, an exception is thrown.
				 */
				dh_key get_dh_key() const;

				/**
				 * \brief Take ownership of a DH key.
				 * \param dh The DH key.
				 */
				void assign_dh_key(DH* dh) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO according to PKCS8.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key_pkcs8(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO according to PKCS8.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key_pkcs8(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO according to PKCS8, but use PKCS5 or PKCS12 as an encryption algorithm.
				 * \param bio The BIO.
				 * \param nid The nid of the encryption algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key_pkcs8_nid(bio::bio_ptr bio, int nid, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a BIO according to PKCS8, but use PKCS5 or PKCS12 as an encryption algorithm.
				 * \param bio The BIO.
				 * \param nid The nid of the encryption algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key_pkcs8_nid(bio::bio_ptr bio, int nid, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the certificate public EVP_PKEY key to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_public_key(bio::bio_ptr bio) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file according to PKCS8.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key_pkcs8(file file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file according to PKCS8.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key_pkcs8(file file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file according to PKCS8, but use PKCS5 or PKCS12 as an encryption algorithm.
				 * \param file The file.
				 * \param nid The nid of the encryption algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key_pkcs8_nid(file file, int nid, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private EVP_PKEY key to a file according to PKCS8, but use PKCS5 or PKCS12 as an encryption algorithm.
				 * \param file The file.
				 * \param nid The nid of the encryption algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key_pkcs8_nid(file file, int nid, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the certificate public EVP_PKEY key to a file.
				 * \param file The file.
				 */
				void write_certificate_public_key(file file) const;

				/**
				 * \brief Get the maximum size of a signature.
				 * \return The maximum size of a signature.
				 */
				size_t size() const;

				/**
				 * \brief Get the private key type.
				 * \return The nid of the private key type.
				 */
				int type() const;

				/**
				 * \brief Check if the pkey holds a RSA key.
				 * \return true or false.
				 */
				bool is_rsa() const;

				/**
				 * \brief Check if the pkey holds a DSA key.
				 * \return true or false.
				 */
				bool is_dsa() const;

				/**
				 * \brief Check if the pkey holds a DH key.
				 * \return true or false.
				 */
				bool is_dh() const;

			private:

				explicit pkey(pointer _ptr, deleter_type _del);
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

		inline pkey pkey::create()
		{
			return take_ownership(EVP_PKEY_new());
		}
		inline pkey pkey::from_rsa_key(rsa_key key)
		{
			pkey result = create();
			result.set_rsa_key(key);

			return result;
		}
		inline pkey pkey::from_dsa_key(dsa_key key)
		{
			pkey result = create();
			result.set_dsa_key(key);

			return result;
		}
		inline pkey pkey::from_dh_key(dh_key key)
		{
			pkey result = create();
			result.set_dh_key(key);

			return result;
		}
		inline pkey pkey::from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_PrivateKey(bio.raw(), NULL, callback, callback_arg));
		}
		inline pkey pkey::from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_PUBKEY(bio.raw(), NULL, callback, callback_arg));
		}
		inline pkey pkey::from_hmac_key(const void* buf, size_t buf_len, ENGINE* engine)
		{
			return take_ownership(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, engine, static_cast<const unsigned char*>(buf), static_cast<int>(buf_len)));
		}
		inline pkey pkey::from_private_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_PrivateKey(_file.raw(), NULL, callback, callback_arg));
		}
		inline pkey pkey::from_certificate_public_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_PUBKEY(_file.raw(), NULL, callback, callback_arg));
		}
		inline pkey::pkey()
		{
		}
		inline pkey::pkey(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void pkey::set_rsa_key(rsa_key rsa) const
		{
			throw_error_if_not(EVP_PKEY_set1_RSA(ptr().get(), rsa.raw()) != 0);
		}
		inline rsa_key pkey::get_rsa_key() const
		{
			return rsa_key::take_ownership(EVP_PKEY_get1_RSA(ptr().get()));
		}
		inline void pkey::assign_rsa_key(RSA* rsa) const
		{
			throw_error_if_not(EVP_PKEY_assign_RSA(ptr().get(), rsa) != 0);
		}
		inline void pkey::set_dsa_key(dsa_key dsa) const
		{
			throw_error_if_not(EVP_PKEY_set1_DSA(ptr().get(), dsa.raw()) != 0);
		}
		inline dsa_key pkey::get_dsa_key() const
		{
			return dsa_key::take_ownership(EVP_PKEY_get1_DSA(ptr().get()));
		}
		inline void pkey::assign_dsa_key(DSA* dsa) const
		{
			throw_error_if_not(EVP_PKEY_assign_DSA(ptr().get(), dsa) != 0);
		}
		inline void pkey::set_dh_key(dh_key dh) const
		{
			throw_error_if_not(EVP_PKEY_set1_DH(ptr().get(), dh.raw()) != 0);
		}
		inline dh_key pkey::get_dh_key() const
		{
			return dh_key::take_ownership(EVP_PKEY_get1_DH(ptr().get()));
		}
		inline void pkey::assign_dh_key(DH* dh) const
		{
			throw_error_if_not(EVP_PKEY_assign_DH(ptr().get(), dh) != 0);
		}
		inline void pkey::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_bio_PrivateKey(bio.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_bio_PrivateKey(bio.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_private_key_pkcs8(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_bio_PKCS8PrivateKey(bio.raw(), ptr().get(), algorithm.raw(), static_cast<char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key_pkcs8(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_bio_PKCS8PrivateKey(bio.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_private_key_pkcs8_nid(bio::bio_ptr bio, int nid, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_bio_PKCS8PrivateKey_nid(bio.raw(), ptr().get(), nid, static_cast<char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key_pkcs8_nid(bio::bio_ptr bio, int nid, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_bio_PKCS8PrivateKey_nid(bio.raw(), ptr().get(), nid, NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_certificate_public_key(bio::bio_ptr bio) const
		{
			throw_error_if_not(PEM_write_bio_PUBKEY(bio.raw(), ptr().get()) != 0);
		}
		inline void pkey::write_private_key(file _file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_PrivateKey(_file.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key(file _file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_PrivateKey(_file.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_private_key_pkcs8(file _file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_PKCS8PrivateKey(_file.raw(), ptr().get(), algorithm.raw(), static_cast<char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key_pkcs8(file _file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_PKCS8PrivateKey(_file.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_private_key_pkcs8_nid(file _file, int nid, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_PKCS8PrivateKey_nid(_file.raw(), ptr().get(), nid, static_cast<char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void pkey::write_private_key_pkcs8_nid(file _file, int nid, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_PKCS8PrivateKey_nid(_file.raw(), ptr().get(), nid, NULL, 0, callback, callback_arg) != 0);
		}
		inline void pkey::write_certificate_public_key(file _file) const
		{
			throw_error_if_not(PEM_write_PUBKEY(_file.raw(), ptr().get()) != 0);
		}
		inline size_t pkey::size() const
		{
			return EVP_PKEY_size(ptr().get());
		}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
		inline int pkey::type() const
		{
			return EVP_PKEY_base_id(ptr().get());
		}
#else
		inline int pkey::type() const
		{
			return EVP_PKEY_type(ptr()->type);
		}
#endif
		inline bool pkey::is_rsa() const
		{
			return (type() == EVP_PKEY_RSA);
		}
		inline bool pkey::is_dsa() const
		{
			return (type() == EVP_PKEY_DSA);
		}
		inline bool pkey::is_dh() const
		{
			return (type() == EVP_PKEY_DH);
		}
		inline pkey::pkey(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
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

#endif /* CRYPTOPLUS_PKEY_PKEY_HPP */


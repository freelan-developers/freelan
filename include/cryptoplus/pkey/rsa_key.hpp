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
 * \file rsa_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A RSA key class.
 */

#ifndef CRYPTOPLUS_PKEY_RSA_KEY_HPP
#define CRYPTOPLUS_PKEY_RSA_KEY_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../hash/message_digest_algorithm.hpp"
#include "../file.hpp"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/engine.h>

#include <vector>

namespace cryptoplus
{
	namespace pkey
	{
		/**
		 * \brief A RSA key.
		 *
		 * The rsa_key class represents a RSA key (with or without a private compound).
		 * rsa_key is a low level structure that offers no mean to know whether the represented RSA key is a public or private key: it is up to the user to ensure that private key related functions will only get called on rsa_key instances that have private key information.
		 *
		 * A rsa_key instance has the same semantic as a RSA* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class rsa_key : public pointer_wrapper<RSA>
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
				 * \brief Create a new rsa_key.
				 * \return The rsa_key.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static rsa_key create();

				/**
				 * \brief Take ownership of a specified RSA pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An rsa_key.
				 */
				static rsa_key take_ownership(pointer ptr);

				/**
				 * \brief Generate a new RSA private key.
				 * \param num The size (in bits) of the modulus. As specified in OpenSSL documentation, key sizes with num < 1024 should be considered insecure.
				 * \param exponent The exponent. Must be an odd number: typically 3, 17 or 65537.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of RSA_generate_key(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key generate_private_key(int num, unsigned long exponent, generate_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a private RSA key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a public RSA key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public RSA key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a private RSA key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_private_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a public RSA key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_public_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public RSA key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_certificate_public_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a RSA key from a private key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_private_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a RSA key from a public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a RSA key from a certificate public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The rsa_key.
				 */
				static rsa_key from_certificate_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty rsa_key.
				 */
				rsa_key();

				/**
				 * \brief Create an rsa_key by *NOT* taking ownership of an existing RSA pointer.
				 * \param ptr The RSA pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				rsa_key(pointer ptr);

				/**
				 * \brief Write the private RSA key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len);

				/**
				 * \brief Write the private RSA key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL);

				/**
				 * \brief Write the public RSA key to a BIO.
				 * \param bio The BIO.
				 */
				void write_public_key(bio::bio_ptr bio);

				/**
				 * \brief Write the certificate public RSA key to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_public_key(bio::bio_ptr bio);

				/**
				 * \brief Write the private RSA key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len);

				/**
				 * \brief Write the private RSA key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL);

				/**
				 * \brief Write the public RSA key to a file.
				 * \param file The file.
				 */
				void write_public_key(file file);

				/**
				 * \brief Write the certificate public RSA key to a file.
				 * \param file The file.
				 */
				void write_certificate_public_key(file file);

				/**
				 * \brief Enable blinding of the rsa_key to prevent timing attacks.
				 * \param ctx A BN_CTX to use or NULL (the default) if one is to be created.
				 * \warning The PRNG must be seeded prior to calling enable_blinding().
				 * \see disable_blinding
				 */
				void enable_blinding(BN_CTX* ctx = NULL);

				/**
				 * \brief Disable blinding on the rsa_key after a previous call to enable_blinding().
				 * \see enable_blinding
				 */
				void disable_blinding();

				/**
				 * \brief Return the RSA modulus size in bytes.
				 * \return The RSA modulus size, in bytes.
				 */
				size_t size() const;

				/**
				 * \brief Check the rsa_key for validity.
				 * \warning The instance must contain both public and private key data.
				 *
				 * If the rsa_key is not valid, an exception is thrown.
				 */
				void check();

				/**
				 * \brief Print the RSA key in a human-readable hexadecimal form to a specified BIO.
				 * \param bio The BIO to use.
				 * \param offset The number of offset spaces to output.
				 */
				void print(bio::bio_ptr bio, int offset = 0);

				/**
				 * \brief Print the RSA key in a human-readable hexadecimal form to a specified file.
				 * \param file The file.
				 * \param offset The number of offset spaces to output.
				 */
				void print(file file, int offset = 0);

				/**
				 * \brief Extract a public RSA key from a private RSA key.
				 * \return A public RSA key.
				 */
				rsa_key to_public_key();

				/**
				 * \brief Add a PKCS#1 V2.0 PSS padding.
				 * \param out The destination buffer.
				 * \param out_len The length of out.
				 * \param buf The data to read from.
				 * \param buf_len The length of buf.
				 * \param algorithm The message digest algorithm to use.
				 * \param salt_len The salt_len. Should be -1 or -2.
				 *
				 * The result out buffer should then be encrypted using private_encrypt().
				 */
				void padding_add_PKCS1_PSS(void* out, size_t out_len, const void* buf, size_t buf_len, hash::message_digest_algorithm algorithm, int salt_len);

				/**
				 * \brief Verify a PKCS#1 V2.0 PSS padding.
				 * \param digest The message digest.
				 * \param digest_len The length of digest.
				 * \param buf The decrypted signature, obtained from public_decrypt().
				 * \param buf_len The length of buf.
				 * \param algorithm The message digest algorithm to use.
				 * \param salt_len The salt_len. Should be -1 or -2.
				 */
				void verify_PKCS1_PSS(const void* digest, size_t digest_len, const void* buf, size_t buf_len, hash::message_digest_algorithm algorithm, int salt_len);

				/**
				 * \brief Encrypt data bytes using the private key information.
				 * \param out The buffer to write the result to. Must be at least size() bytes long.
				 * \param out_len The length of out.
				 * \param buf The data to encrypt. Must be smaller than size() - 11.
				 * \param buf_len The length of buf.
				 * \param padding The padding mode to use. The list of available padding mode can be found in the man page of RSA_private_encrypt(3).
				 * \return The count of bytes written to out. Should be size().
				 * \see public_decrypt
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t private_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding);

				/**
				 * \brief Decrypt data bytes using the public key information.
				 * \param out The buffer to write the result to. Must be at least size() - 11 bytes long.
				 * \param out_len The length of out.
				 * \param buf The data to decrypt.
				 * \param buf_len The length of buf.
				 * \param padding The padding mode to use. The list of available padding mode can be found in the man page of RSA_private_encrypt(3).
				 * \return The count of bytes written to out. Should be size().
				 * \see private_encrypt
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t public_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding);

				/**
				 * \brief Encrypt data bytes using the public key information.
				 * \param out The buffer to write the result to. Must be at least size() bytes long.
				 * \param out_len The length of out.
				 * \param buf The data to encrypt. Depending on the padding mode, buf must be either smaller than size() - 11, size() - 41 or size(). For additional information, take a look at RSA_public_encrypt(3).
				 * \param buf_len The length of buf.
				 * \param padding The padding mode to use. The list of available padding mode can be found in the man page of RSA_public_encrypt(3).
				 * \return The count of bytes written to out. Should be size().
				 * \see public_decrypt
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t public_encrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding);

				/**
				 * \brief Decrypt data bytes using the public key information.
				 * \param out The buffer to write the result to. Whose size depends on the padding mode. If out is at least size() bytes long, no size problem will occur.
				 * \param out_len The length of out.
				 * \param buf The data to decrypt.
				 * \param buf_len The length of buf.
				 * \param padding The padding mode to use. The list of available padding mode can be found in the man page of RSA_public_encrypt(3).
				 * \return The count of bytes written to out. Should be size().
				 * \see private_encrypt
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t private_decrypt(void* out, size_t out_len, const void* buf, size_t buf_len, int padding);

				/**
				 * \brief Sign a message digest, as specified by PCKS #1 v2.0.
				 * \param out The buffer to write the signature to. Must be size() bytes long.
				 * \param out_len The length of out.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The NID of the message digest algorithm that was used to generate the message digest buffer. Usually one of NID_sha1, NID_ripemd160, NID_md5. See RSA_sign(3) for additional information.
				 * \return The number of bytes written to out.
				 * \see verify
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type);

				/**
				 * \brief Sign a message digest, as specified by PCKS #1 v2.0.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The NID of the message digest algorithm that was used to generate the message digest buffer. Usually one of NID_sha1, NID_ripemd160, NID_md5. See RSA_sign(3) for additional information.
				 * \return The signature.
				 * \see verify
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				template <typename T>
				std::vector<T> sign(const void* buf, size_t buf_len, int type);

				/**
				 * \brief Verify a message digest signature, as specified by PCKS #1 v2.0.
				 * \param sign The signature buffer, as generated by sign().
				 * \param sign_len The length of sign.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The NID of the message digest algorithm that was used to generate the message digest buffer. Usually one of NID_sha1, NID_ripemd160, NID_md5. See RSA_sign(3) for additional information.
				 * \see sign
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				void verify(const void* sign, size_t sign_len, const void* buf, size_t buf_len, int type);

			private:

				explicit rsa_key(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two rsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two rsa_key instances share the same underlying pointer.
		 */
		bool operator==(const rsa_key& lhs, const rsa_key& rhs);

		/**
		 * \brief Compare two rsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two rsa_key instances do not share the same underlying pointer.
		 */
		bool operator!=(const rsa_key& lhs, const rsa_key& rhs);

		inline rsa_key rsa_key::create()
		{
			return take_ownership(RSA_new());
		}
		inline rsa_key rsa_key::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return rsa_key(_ptr, deleter);
		}
		inline rsa_key rsa_key::from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_RSAPrivateKey(bio.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key rsa_key::from_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_RSAPublicKey(bio.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key rsa_key::from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_RSA_PUBKEY(bio.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key rsa_key::from_private_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_RSAPrivateKey(_file.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key rsa_key::from_public_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_RSAPublicKey(_file.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key rsa_key::from_certificate_public_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_RSA_PUBKEY(_file.raw(), NULL, callback, callback_arg));
		}
		inline rsa_key::rsa_key()
		{
		}
		inline rsa_key::rsa_key(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void rsa_key::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len)
		{
			error::throw_error_if_not(PEM_write_bio_RSAPrivateKey(bio.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void rsa_key::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg)
		{
			error::throw_error_if_not(PEM_write_bio_RSAPrivateKey(bio.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void rsa_key::write_public_key(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_RSAPublicKey(bio.raw(), ptr().get()) != 0);
		}
		inline void rsa_key::write_certificate_public_key(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_RSA_PUBKEY(bio.raw(), ptr().get()) != 0);
		}
		inline void rsa_key::write_private_key(file _file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len)
		{
			error::throw_error_if_not(PEM_write_RSAPrivateKey(_file.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void rsa_key::write_private_key(file _file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg)
		{
			error::throw_error_if_not(PEM_write_RSAPrivateKey(_file.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void rsa_key::write_public_key(file _file)
		{
			error::throw_error_if_not(PEM_write_RSAPublicKey(_file.raw(), ptr().get()) != 0);
		}
		inline void rsa_key::write_certificate_public_key(file _file)
		{
			error::throw_error_if_not(PEM_write_RSA_PUBKEY(_file.raw(), ptr().get()) != 0);
		}
		inline void rsa_key::enable_blinding(BN_CTX* ctx)
		{
			error::throw_error_if_not(RSA_blinding_on(ptr().get(), ctx) != 0);
		}
		inline void rsa_key::disable_blinding()
		{
			RSA_blinding_off(ptr().get());
		}
		inline size_t rsa_key::size() const
		{
			return RSA_size(ptr().get());
		}
		inline void rsa_key::check()
		{
			error::throw_error_if_not(RSA_check_key(ptr().get()) > 0);
		}
		inline void rsa_key::print(bio::bio_ptr bio, int offset)
		{
			error::throw_error_if_not(RSA_print(bio.raw(), ptr().get(), offset) != 0);
		}
		inline void rsa_key::print(file _file, int offset)
		{
			error::throw_error_if_not(RSA_print_fp(_file.raw(), ptr().get(), offset) != 0);
		}
		template <typename T>
		inline std::vector<T> rsa_key::sign(const void* buf, size_t buf_len, int type)
		{
			std::vector<T> result(size());

			result.resize(sign(&result[0], result.size(), buf, buf_len, type));

			return result;
		}
		inline rsa_key::rsa_key(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const rsa_key& lhs, const rsa_key& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const rsa_key& lhs, const rsa_key& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_PKEY_RSA_KEY_HPP */


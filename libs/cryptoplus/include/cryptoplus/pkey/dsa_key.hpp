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
 * \file dsa_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DSA key class.
 */

#ifndef CRYPTOPLUS_PKEY_DSA_KEY_HPP
#define CRYPTOPLUS_PKEY_DSA_KEY_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "../bio/bio_ptr.hpp"
#include "../file.hpp"
#include "dh_key.hpp"

#include <openssl/dsa.h>
#include <openssl/pem.h>
#include <openssl/engine.h>

namespace cryptoplus
{
	namespace pkey
	{
		/**
		 * \brief A DSA key.
		 *
		 * The dsa_key class represents a DSA key (with or without a private compound).
		 * dsa_key is a low level structure that offers no mean to know what components are set up in the underlying DSA structure. It is up to the caller to perform only allowed operations on the dsa_key instance.
		 *
		 * A dsa_key instance has the same semantic as a DSA* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class dsa_key : public pointer_wrapper<DSA>
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
				 * \brief Create a new dsa_key.
				 * \return The dsa_key.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static dsa_key create();

				/**
				 * \brief Take ownership of a specified ASN1_dsa_key pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An dsa_key.
				 */
				static dsa_key take_ownership(pointer ptr);

				/**
				 * \brief Create a new DSA key with the specified parameters.
				 * \param bits The length, in bits, of the prime to be generated. Maximum value is 1024.
				 * \param seed The seed to use. Can be NULL.
				 * \param seed_len The length of seed. Can't exceed 20.
				 * \param counter_ret The iteration counter. Can be NULL.
				 * \param h_ret The counter for finding a generator. Can be NULL.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of DSA_generate_parameters(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \param must_take_ownership If set to true (the default), the resulting DSA key will own the underlying DSA pointer. If set to false, the caller is responsible for handling the lifetime of the underlying DSA pointer. Failing to do so will cause memory leaks.
				 * \return The dsa_key.
				 */
				static dsa_key generate_parameters(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback = NULL, void* callback_arg = NULL, bool must_take_ownership = true);

				/**
				 * \brief Generate a new DSA private key.
				 * \param bits The length, in bits, of the prime to be generated. Maximum value is 1024.
				 * \param seed The seed to use. Can be NULL.
				 * \param seed_len The length of seed. Can't exceed 20.
				 * \param counter_ret The iteration counter. Can be NULL.
				 * \param h_ret The counter for finding a generator. Can be NULL.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of DSA_generate_parameters(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 *
				 * generate_private_key() is equivalent to a call to generate_parameters() followed by a call to generate().
				 */
				static dsa_key generate_private_key(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a private DSA key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DSA parameters from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public DSA key from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a private DSA key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_private_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DSA parameters from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_parameters(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a certificate public DSA key from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_certificate_public_key(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a DSA key from a private key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_private_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DSA parameters from a public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_parameters(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a DSA key from a certificate public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key from_certificate_public_key(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty dsa_key.
				 */
				dsa_key();

				/**
				 * \brief Create an dsa_key by *NOT* taking ownership of an existing DSA pointer.
				 * \param ptr The DSA pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				dsa_key(pointer ptr);

				/**
				 * \brief Write the private DSA key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private DSA key to a BIO.
				 * \param bio The BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the DSA parameters to a BIO.
				 * \param bio The BIO.
				 */
				void write_parameters(bio::bio_ptr bio) const;

				/**
				 * \brief Write the certificate public DSA key to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_public_key(bio::bio_ptr bio) const;

				/**
				 * \brief Write the private DSA key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param passphrase The passphrase to use.
				 * \param passphrase_len The length of passphrase.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const;

				/**
				 * \brief Write the private DSA key to a file.
				 * \param file The file.
				 * \param algorithm The cipher algorithm to use.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 */
				void write_private_key(file file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg = NULL) const;

				/**
				 * \brief Write the DSA parameters to a file.
				 * \param file The file.
				 */
				void write_parameters(file file) const;

				/**
				 * \brief Write the certificate public DSA key to a file.
				 * \param file The file.
				 */
				void write_certificate_public_key(file file) const;

				/**
				 * \brief Generate the DSA key, reading its parameters.
				 * \return The current instance.
				 *
				 * On error, an exception is thrown.
				 */
				dsa_key& generate();

				/**
				 * \brief Generate the DSA key, reading its parameters.
				 * \return The current instance.
				 *
				 * On error, an exception is thrown.
				 */
				const dsa_key& generate() const;

				/**
				 * \brief Return the size of a DSA signature in bytes.
				 * \return The DSA signature size, in bytes.
				 */
				size_t size() const;

				/**
				 * \brief Print the DSA key in a human-readable hexadecimal form to a specified BIO.
				 * \param bio The BIO to use.
				 * \param offset The number of offset spaces to output.
				 */
				void print(bio::bio_ptr bio, int offset = 0) const;

				/**
				 * \brief Print the DSA key in a human-readable hexadecimal form to a specified file.
				 * \param file The file.
				 * \param offset The number of offset spaces to output.
				 */
				void print(file file, int offset = 0) const;

				/**
				 * \brief Print the DSA parameters in a human-readable hexadecimal form to a specified BIO.
				 * \param bio The BIO to use.
				 */
				void print_parameters(bio::bio_ptr bio) const;

				/**
				 * \brief Print the DSA parameters in a human-readable hexadecimal form to a specified file.
				 * \param file The file.
				 */
				void print_parameters(file file) const;

				/**
				 * \brief Extract a public DSA key from a private DSA key.
				 * \return A public DSA key.
				 */
				dsa_key to_public_key() const;

				/**
				 * \brief Sign a message digest.
				 * \param out The buffer to write the signature to. Must be size() bytes long.
				 * \param out_len The length of out.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The type parameter is ignored.
				 * \return The number of bytes written to out.
				 * \see verify
				 *
				 * In case of failure, an exception is thrown.
				 */
				size_t sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type) const;

				/**
				 * \brief Sign a message digest.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The type parameter is ignored.
				 * \return The signature.
				 * \see verify
				 *
				 * In case of failure, an exception is thrown.
				 */
				buffer sign(const void* buf, size_t buf_len, int type) const;

				/**
				 * \brief Sign a message digest.
				 * \param buf The message digest buffer.
				 * \param type The type parameter is ignored.
				 * \return The signature.
				 * \see verify
				 *
				 * In case of failure, an exception is thrown.
				 */
				buffer sign(const buffer& buf, int type) const;

				/**
				 * \brief Verify a message digest signature, as specified by PCKS #1 v2.0.
				 * \param sign The signature buffer, as generated by sign().
				 * \param sign_len The length of sign.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The type parameter is ignored.
				 * \see sign
				 *
				 * In case of failure, an exception is thrown.
				 */
				void verify(const void* sign, size_t sign_len, const void* buf, size_t buf_len, int type) const;

				/**
				 * \brief Verify a message digest signature, as specified by PCKS #1 v2.0.
				 * \param sign The signature buffer, as generated by sign().
				 * \param buf The message digest buffer.
				 * \param type The type parameter is ignored.
				 * \see sign
				 *
				 * In case of failure, an exception is thrown.
				 */
				void verify(const buffer& sign, const buffer& buf, int type) const;

				/**
				 * \brief Duplicate DSA parameters and keys as DH parameters and keys.
				 * \return A new DH structure.
				 */
				dh_key to_dh_key() const;

			private:

				explicit dsa_key(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instances share the same underlying pointer.
		 */
		bool operator==(const dsa_key& lhs, const dsa_key& rhs);

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instances do not share the same underlying pointer.
		 */
		bool operator!=(const dsa_key& lhs, const dsa_key& rhs);

		inline dsa_key dsa_key::create()
		{
			return take_ownership(DSA_new());
		}
		inline dsa_key dsa_key::generate_private_key(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback, void* callback_arg)
		{
			return generate_parameters(bits, seed, seed_len, counter_ret, h_ret, callback, callback_arg).generate();
		}
		inline dsa_key dsa_key::from_private_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_DSAPrivateKey(bio.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key dsa_key::from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_DSAparams(bio.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key dsa_key::from_certificate_public_key(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_DSA_PUBKEY(bio.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key dsa_key::from_private_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_DSAPrivateKey(_file.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key dsa_key::from_parameters(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_DSAparams(_file.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key dsa_key::from_certificate_public_key(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_DSA_PUBKEY(_file.raw(), NULL, callback, callback_arg));
		}
		inline dsa_key::dsa_key()
		{
		}
		inline dsa_key::dsa_key(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void dsa_key::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_bio_DSAPrivateKey(bio.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void dsa_key::write_private_key(bio::bio_ptr bio, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_bio_DSAPrivateKey(bio.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void dsa_key::write_parameters(bio::bio_ptr bio) const
		{
			throw_error_if_not(PEM_write_bio_DSAparams(bio.raw(), ptr().get()) != 0);
		}
		inline void dsa_key::write_certificate_public_key(bio::bio_ptr bio) const
		{
			throw_error_if_not(PEM_write_bio_DSA_PUBKEY(bio.raw(), ptr().get()) != 0);
		}
		inline void dsa_key::write_private_key(file _file, cipher::cipher_algorithm algorithm, const void* passphrase, size_t passphrase_len) const
		{
			throw_error_if_not(PEM_write_DSAPrivateKey(_file.raw(), ptr().get(), algorithm.raw(), static_cast<unsigned char*>(const_cast<void*>(passphrase)), static_cast<int>(passphrase_len), NULL, NULL) != 0);
		}
		inline void dsa_key::write_private_key(file _file, cipher::cipher_algorithm algorithm, pem_passphrase_callback_type callback, void* callback_arg) const
		{
			throw_error_if_not(PEM_write_DSAPrivateKey(_file.raw(), ptr().get(), algorithm.raw(), NULL, 0, callback, callback_arg) != 0);
		}
		inline void dsa_key::write_parameters(file _file) const
		{
			throw_error_if_not(PEM_write_DSAparams(_file.raw(), ptr().get()) != 0);
		}
		inline void dsa_key::write_certificate_public_key(file _file) const
		{
			throw_error_if_not(PEM_write_DSA_PUBKEY(_file.raw(), ptr().get()) != 0);
		}
		inline dsa_key& dsa_key::generate()
		{
			throw_error_if_not(DSA_generate_key(ptr().get()) != 0);

			return *this;
		}
		inline const dsa_key& dsa_key::generate() const
		{
			throw_error_if_not(DSA_generate_key(ptr().get()) != 0);

			return *this;
		}
		inline size_t dsa_key::size() const
		{
			return DSA_size(ptr().get());
		}
		inline void dsa_key::print(bio::bio_ptr bio, int offset) const
		{
			throw_error_if_not(DSA_print(bio.raw(), ptr().get(), offset) != 0);
		}
		inline void dsa_key::print(file _file, int offset) const
		{
			throw_error_if_not(DSA_print_fp(_file.raw(), ptr().get(), offset) != 0);
		}
		inline void dsa_key::print_parameters(bio::bio_ptr bio) const
		{
			throw_error_if_not(DSAparams_print(bio.raw(), ptr().get()) != 0);
		}
		inline void dsa_key::print_parameters(file _file) const
		{
			throw_error_if_not(DSAparams_print_fp(_file.raw(), ptr().get()) != 0);
		}
		inline buffer dsa_key::sign(const void* buf, size_t buf_len, int type) const
		{
			buffer result(size());

			result.data().resize(sign(buffer_cast<uint8_t*>(result), buffer_size(result), buf, buf_len, type));

			return result;
		}
		inline buffer dsa_key::sign(const buffer& buf, int type) const
		{
			return sign(buffer_cast<const uint8_t*>(buf), buffer_size(buf), type);
		}
		inline void dsa_key::verify(const void* _sign, size_t sign_len, const void* buf, size_t buf_len, int type) const
		{
			throw_error_if_not(DSA_verify(type, static_cast<const unsigned char*>(buf), static_cast<int>(buf_len), static_cast<const unsigned char*>(_sign), static_cast<int>(sign_len), ptr().get()) != 0);
		}
		inline void dsa_key::verify(const buffer& _sign, const buffer& buf, int type) const
		{
			verify(buffer_cast<const uint8_t*>(_sign), buffer_size(_sign), buffer_cast<const uint8_t*>(buf), buffer_size(buf), type);
		}
		inline dh_key dsa_key::to_dh_key() const
		{
			return dh_key::take_ownership(DSA_dup_DH(ptr().get()));
		}
		inline dsa_key::dsa_key(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_PKEY_DSA_KEY_HPP */


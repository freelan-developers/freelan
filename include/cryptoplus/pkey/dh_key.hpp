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
 * \file dh_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A Diffie-Hellman key class.
 */

#ifndef CRYPTOPLUS_PKEY_DH_KEY_HPP
#define CRYPTOPLUS_PKEY_DH_KEY_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../bn/bignum.hpp"
#include "../file.hpp"

#include <openssl/dh.h>
#include <openssl/pem.h>
#include <openssl/engine.h>

#include <vector>

namespace cryptoplus
{
	namespace pkey
	{
		/**
		 * \brief A Diffie-Hellman helper class.
		 *
		 * The dh_key class represents a DH structure (with or without a private compound).
		 * dh_key is a low level structure that offers no mean to know what components are set up in the underlying DH structure. It is up to the caller to perform only allowed operations on the dh_key instance.
		 *
		 * A dh_key instance has the same semantic as a DH* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class dh_key : public pointer_wrapper<DH>
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
				 * \brief Create a new dh_key.
				 * \return The dh_key.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static dh_key create();

				/**
				 * \brief Take ownership of a specified DH pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An dh_key.
				 */
				static dh_key take_ownership(pointer ptr);

				/**
				 * \brief Create a new DH with the specified parameters.
				 * \param prime_len The length, in bits, of the safe prime number to be generated.
				 * \param generator A small number greater than 1. Typically 2 or 5.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of DH_generate_parameters(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dh_key.
				 */
				static dh_key generate_parameters(int prime_len, int generator, generate_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh_key from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh_key from_parameters(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh_key from_parameters(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty dh_key.
				 */
				dh_key();

				/**
				 * \brief Create an dh_key by *NOT* taking ownership of an existing DH pointer.
				 * \param ptr The DH pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				dh_key(pointer ptr);

				/**
				 * \brief Write the DH parameters to a BIO.
				 * \param bio The BIO.
				 */
				void write_parameters(bio::bio_ptr bio);

				/**
				 * \brief Write the DH parameters to a file.
				 * \param file The file.
				 */
				void write_parameters(file file);

				/**
				 * \brief Get the private key component.
				 * \return the private key component.
				 */
				bn::bignum private_key() const;

				/**
				 * \brief Get the public key component.
				 * \return the public key component.
				 */
				bn::bignum public_key() const;

				/**
				 * \brief Return the size of a DH signature in bytes.
				 * \return The DH signature size, in bytes.
				 */
				size_t size() const;

				/**
				 * \brief Validates the Diffie-Hellman parameters.
				 * \param codes An integer whose content is updated according to the success or failure of the check operation. For more information, take a look a the DH_generate_parameters(3) man page.
				 *
				 * On failure, a cryptographic_exception is thrown.
				 */
				void check(int& codes);

				/**
				 * \brief Performs the first step of a Diffie-Hellman key exchange by generating private and public DH values.
				 * \return *this
				 *
				 * On failure, a cryptographic_exception is thrown.
				 */
				dh_key& generate_key();

				/**
				 * \brief Compute the shared secret from the private DH value in the instance and other party's public value.
				 * \param out The buffer to which the shared secret is written. Must be at least size() bytes long.
				 * \param out_len The length of out.
				 * \param pub_key The other party's public key.
				 * \return The number of bytes written to out.
				 *
				 * On failure, a cryptographic_exception is thrown.
				 */
				size_t compute_key(void* out, size_t out_len, bn::bignum pub_key);

				/**
				 * \brief Compute the shared secret from the private DH value in the instance and other party's public value.
				 * \param pub_key The other party's public key.
				 * \return The shared secret.
				 *
				 * On failure, a cryptographic_exception is thrown.
				 */
				template <typename T>
				std::vector<T> compute_key(bn::bignum pub_key);

				/**
				 * \brief Print the DH parameters in a human-readable hexadecimal form to a specified BIO.
				 * \param bio The BIO to use.
				 */
				void print_parameters(bio::bio_ptr bio);

				/**
				 * \brief Print the DH parameters in a human-readable hexadecimal form to a specified file.
				 * \param file The file.
				 */
				void print_parameters(file file);

			private:

				explicit dh_key(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two dh_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dh_key instances share the same underlying pointer.
		 */
		bool operator==(const dh_key& lhs, const dh_key& rhs);

		/**
		 * \brief Compare two dh_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dh_key instances do not share the same underlying pointer.
		 */
		bool operator!=(const dh_key& lhs, const dh_key& rhs);

		inline dh_key dh_key::create()
		{
			return take_ownership(DH_new());
		}
		inline dh_key dh_key::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return dh_key(_ptr, deleter);
		}
		inline dh_key dh_key::from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_DHparams(bio.raw(), NULL, callback, callback_arg));
		}
		inline dh_key dh_key::from_parameters(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_DHparams(_file.raw(), NULL, callback, callback_arg));
		}
		inline dh_key::dh_key()
		{
		}
		inline dh_key::dh_key(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void dh_key::write_parameters(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_DHparams(bio.raw(), ptr().get()) != 0);
		}
		inline void dh_key::write_parameters(file _file)
		{
			error::throw_error_if_not(PEM_write_DHparams(_file.raw(), ptr().get()) != 0);
		}
		inline bn::bignum dh_key::private_key() const
		{
			return raw()->priv_key;
		}
		inline bn::bignum dh_key::public_key() const
		{
			return raw()->pub_key;
		}
		inline size_t dh_key::size() const
		{
			return DH_size(ptr().get());
		}
		inline void dh_key::check(int& codes)
		{
			error::throw_error_if_not(DH_check(ptr().get(), &codes) != 0);
		}
		inline dh_key& dh_key::generate_key()
		{
			error::throw_error_if_not(DH_generate_key(ptr().get()) != 0);

			return *this;
		}
		template <typename T>
		inline std::vector<T> dh_key::compute_key(bn::bignum pub_key)
		{
			std::vector<T> result(size());

			result.resize(compute_key(&result[0], result.size(), pub_key.raw()));

			return result;
		}
		inline void dh_key::print_parameters(bio::bio_ptr bio)
		{
			error::throw_error_if_not(DHparams_print(bio.raw(), ptr().get()) != 0);
		}
		inline void dh_key::print_parameters(file _file)
		{
			error::throw_error_if_not(DHparams_print_fp(_file.raw(), ptr().get()) != 0);
		}
		inline bool operator==(const dh_key& lhs, const dh_key& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const dh_key& lhs, const dh_key& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline dh_key::dh_key(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
	}
}

#endif /* CRYPTOPLUS_PKEY_DH_KEY_HPP */


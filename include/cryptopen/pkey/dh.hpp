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
 * \file dh.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A Diffie-Hellman class.
 */

#ifndef CRYPTOPEN_PKEY_DH_HPP
#define CRYPTOPEN_PKEY_DH_HPP

#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"

#include <openssl/dh.h>
#include <openssl/pem.h>
#include <openssl/engine.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace cryptopen
{
	namespace pkey
	{
		/**
		 * \brief A Diffie-Hellman helper class.
		 *
		 * The dh class represents a DH structure (with or without a private compound).
		 * dh is a low level structure that offers no mean to know what components are set up in the underlying DH structure. It is up to the caller to perform only allowed operations on the dh instance.
		 *
		 * A dh instance has the same semantic as a DH* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class dh
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
				 * \brief Create a new DH with the specified parameters.
				 * \param prime_len The length, in bits, of the safe prime number to be generated.
				 * \param generator A small number greater than 1. Typically 2 or 5.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of DH_generate_parameters(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dh.
				 */
				static dh generate_parameters(int prime_len, int generator, generate_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh from_parameters(FILE* file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load DH parameters from a public key buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dh from_parameters(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new DH.
				 */
				dh();

				/**
				 * \brief Create a DH by taking ownership of an existing DH* pointer.
				 * \param dh The DH* pointer. Cannot be NULL.
				 */
				explicit dh(DH* dh);

				/**
				 * \brief Write the DH parameters to a BIO.
				 * \param bio The BIO.
				 */
				void write_parameters(bio::bio_ptr bio);

				/**
				 * \brief Write the DH parameters to a file.
				 * \param file The file.
				 */
				void write_parameters(FILE* file);

				/**
				 * \brief Get the raw DH pointer.
				 * \return The raw DH pointer.
				 * \warning The instance has ownership of the return pointer. Calling DH_free() on the returned value will result in undefined behavior.
				 */
				DH* raw();

				/**
				 * \brief Get the raw DH pointer.
				 * \return The raw DH pointer.
				 * \warning The instance has ownership of the return pointer. Calling DH_free() on the returned value will result in undefined behavior.
				 */
				const DH* raw() const;

				/**
				 * \brief Return the size of a DH signature in bytes.
				 * \return The DH signature size, in bytes.
				 */
				size_t size() const;

				/**
				 * \brief Validates the Diffie-Hellman parameters.
				 * \param codes An integer whose content is updated according to the success or failure of the check operation. For more information, take a look a the DH_generate_parameters(3) man page.
				 * \return true if the DH structure was validated, false otherwise.
				 */
				bool check(int& codes);

				/**
				 * \brief Performs the first step of a Diffie-Hellman key exchange by generating private and public DH values.
				 * 
				 * On failure, a cryptographic_exception is thrown.
				 */
				void generate_key();

				/**
				 * \brief Compute the shared secret from the private DH value in the instance and other party's public value.
				 * \param out The buffer to which the shared secret is written. Must be at least size() bytes long.
				 * \param out_len The length of out.
				 * \param pub_key The other party's public key.
				 * \return The number of bytes written to out.
				 * 
				 * On failure, a cryptographic_exception is thrown.
				 */
				size_t compute_key(void* out, size_t out_len, BIGNUM* pub_key);

				/**
				 * \brief Compute the shared secret from the private DH value in the instance and other party's public value.
				 * \param pub_key The other party's public key.
				 * \return The shared secret.
				 * 
				 * On failure, a cryptographic_exception is thrown.
				 */
				template <typename T>
				std::vector<T> compute_key(BIGNUM* pub_key);

				/**
				 * \brief Print the DH parameters in a human-readable hexadecimal form to a specified BIO.
				 * \param bio The BIO to use.
				 */
				void print_parameters(bio::bio_ptr bio);

				/**
				 * \brief Print the DH parameters in a human-readable hexadecimal form to a specified FILE.
				 * \param file The file.
				 */
				void print_parameters(FILE* file);

			private:

				explicit dh(boost::shared_ptr<DH> dh);

				boost::shared_ptr<DH> m_dh;
		};

		/**
		 * \brief Compare two dh instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dh instances share the same underlying pointer.
		 */
		bool operator==(const dh& lhs, const dh& rhs);

		/**
		 * \brief Compare two dh instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dh instances do not share the same underlying pointer.
		 */
		bool operator!=(const dh& lhs, const dh& rhs);
		
		inline dh dh::from_parameters(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return dh(boost::shared_ptr<DH>(PEM_read_bio_DHparams(bio.raw(), NULL, callback, callback_arg), DH_free));
		}
		inline dh dh::from_parameters(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return dh(boost::shared_ptr<DH>(PEM_read_DHparams(file, NULL, callback, callback_arg), DH_free));
		}
		inline dh::dh() : m_dh(DH_new(), DH_free)
		{
			error::throw_error_if_not(m_dh);
		}
		inline dh::dh(DH* _dh) : m_dh(_dh, DH_free)
		{
			if (!m_dh)
			{
				throw std::invalid_argument("dh");
			}
		}
		inline void dh::write_parameters(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_DHparams(bio.raw(), m_dh.get()));
		}
		inline void dh::write_parameters(FILE* file)
		{
			error::throw_error_if_not(PEM_write_DHparams(file, m_dh.get()));
		}
		inline DH* dh::raw()
		{
			return m_dh.get();
		}
		inline const DH* dh::raw() const
		{
			return m_dh.get();
		}
		inline size_t dh::size() const
		{
			return DH_size(m_dh.get());
		}
		inline bool dh::check(int& codes)
		{
			return DH_check(m_dh.get(), &codes);
		}
		inline void dh::generate_key()
		{
			error::throw_error_if_not(DH_generate_key(m_dh.get()));
		}
		template <typename T>
		inline std::vector<T> dh::compute_key(BIGNUM* pub_key)
		{
			std::vector<T> result(size());

			result.resize(compute_key(&result[0], result.size(), pub_key));

			return result;
		}
		inline void dh::print_parameters(bio::bio_ptr bio)
		{
			error::throw_error_if_not(DHparams_print(bio.raw(), m_dh.get()));
		}
		inline void dh::print_parameters(FILE* file)
		{
			error::throw_error_if_not(DHparams_print_fp(file, m_dh.get()));
		}
		inline bool operator==(const dh& lhs, const dh& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const dh& lhs, const dh& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline dh::dh(boost::shared_ptr<DH> dsa) : m_dh(dsa)
		{
			error::throw_error_if_not(m_dh);
		}
	}
}

#endif /* CRYPTOPEN_PKEY_DH_HPP */


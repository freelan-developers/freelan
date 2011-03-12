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
 * \file dsa_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DSA key class.
 */

#ifndef CRYPTOPEN_PKEY_DSA_KEY_HPP
#define CRYPTOPEN_PKEY_DSA_KEY_HPP

#include "../error/cryptographic_exception.hpp"

#include <openssl/dsa.h>
#include <openssl/engine.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace cryptopen
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
		class dsa_key
		{
			public:

				/**
				 * \brief A generate callback type.
				 */
				typedef void (*generate_callback_type)(int, int, void*);

				/**
				 * \brief Create a new DSA key with the specified parameters.
				 * \param bits The length, in bits, of the prime to be generated. Maximum value is 1024.
				 * \param seed The seed to use. Can be NULL.
				 * \param seed_len The length of seed. Can't exceed 20.
				 * \param counter_ret The iteration counter. Can be NULL.
				 * \param h_ret The counter for finding a generator. Can be NULL.
				 * \param callback A callback that will get notified about the key generation, as specified in the documentation of DSA_generate_parameters(3). callback might be NULL (the default).
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The dsa_key.
				 */
				static dsa_key generate_parameters(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback = NULL, void* callback_arg = NULL);

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
				 * \brief Create a new DSA key.
				 */
				dsa_key();

				/**
				 * \brief Generate the DSA key, reading its parameters.
				 * \return The current instance.
				 *
				 * On error, a cryptographic_exception is thrown.
				 */
				dsa_key& generate();

				/**
				 * \brief Get the raw DSA pointer.
				 * \return The raw DSA pointer.
				 * \warning The instance has ownership of the return pointer. Calling DSA_free() on the returned value will result in undefined behavior.
				 */
				DSA* raw();

				/**
				 * \brief Get the raw DSA pointer.
				 * \return The raw DSA pointer.
				 * \warning The instance has ownership of the return pointer. Calling DSA_free() on the returned value will result in undefined behavior.
				 */
				const DSA* raw() const;

				/**
				 * \brief Return the size of a DSA signature in bytes.
				 * \return The DSA signature size, in bytes.
				 */
				size_t size() const;

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
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				size_t sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type);

				/**
				 * \brief Sign a message digest.
				 * \param buf The message digest buffer.
				 * \param buf_len The length of buf.
				 * \param type The type parameter is ignored.
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
				 * \param type The type parameter is ignored.
				 * \see sign
				 *
				 * In case of failure, a cryptographic_exception is thrown.
				 */
				void verify(const void* sign, size_t sign_len, const void* buf, size_t buf_len, int type);

			private:

				explicit dsa_key(boost::shared_ptr<DSA> dsa);

				boost::shared_ptr<DSA> m_dsa;
		};

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instance share the same underlying pointer.
		 */
		bool operator==(const dsa_key& lhs, const dsa_key& rhs);

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instance do not share the same underlying pointer.
		 */
		bool operator!=(const dsa_key& lhs, const dsa_key& rhs);
		
		inline dsa_key dsa_key::generate_private_key(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback, void* callback_arg)
		{
			return generate_parameters(bits, seed, seed_len, counter_ret, h_ret, callback, callback_arg).generate();
		}
		inline dsa_key::dsa_key() : m_dsa(DSA_new(), DSA_free)
		{
			error::throw_error_if_not(m_dsa);
		}
		inline dsa_key& dsa_key::generate()
		{
			error::throw_error_if_not(DSA_generate_key(m_dsa.get()));

			return *this;
		}
		inline DSA* dsa_key::raw()
		{
			return m_dsa.get();
		}
		inline const DSA* dsa_key::raw() const
		{
			return m_dsa.get();
		}
		inline size_t dsa_key::size() const
		{
			return DSA_size(m_dsa.get());
		}
		template <typename T>
		inline std::vector<T> dsa_key::sign(const void* buf, size_t buf_len, int type)
		{
			std::vector<T> result(size());

			result.resize(sign(&result[0], result.size(), buf, buf_len, type));

			return result;
		}
		inline bool operator==(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline dsa_key::dsa_key(boost::shared_ptr<DSA> dsa) : m_dsa(dsa)
		{
			error::throw_error_if_not(m_dsa);
		}
	}
}

#endif /* CRYPTOPEN_PKEY_DSA_KEY_HPP */


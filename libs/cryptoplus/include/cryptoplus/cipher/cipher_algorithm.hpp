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
 * \file cipher_algorithm.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher algorithm wrapper class.
 */

#ifndef CRYPTOPLUS_CIPHER_CIPHER_ALGORITHM_HPP
#define CRYPTOPLUS_CIPHER_CIPHER_ALGORITHM_HPP

#include "../nullable.hpp"

#include <openssl/evp.h>

#include <string>
#include <cassert>

namespace cryptoplus
{
	namespace cipher
	{
		/**
		 * \brief A cipher algorithm.
		 */
		class cipher_algorithm : public nullable<cipher_algorithm>
		{
			public:

				/**
				 * \brief The maximum key length.
				 */
				static const size_t max_key_length;

				/**
				 * \brief The maximum iv length.
				 */
				static const size_t max_iv_length;

				/**
				 * \brief Create a new cipher_algorithm from a const EVP_CIPHER pointer.
				 * \param cipher The raw const EVP_CIPHER pointer. If cipher is NULL, the cipher_algorithm is a null object on which no methods except raw() should be called.
				 */
				cipher_algorithm(const EVP_CIPHER* cipher = NULL);

				/**
				 * \brief Create a new cipher_algorithm from its type (NID).
				 * \param type The type of the cipher_algorithm to create.
				 * \warning If no such type is found, a std::invalid_argument is thrown.
				 */
				explicit cipher_algorithm(int type);

				/**
				 * \brief Create a new cipher_algorithm from its name.
				 * \param name The name of the cipher_algorithm to create.
				 * \warning If no such name is found, a std::invalid_argument is thrown.
				 */
				explicit cipher_algorithm(const std::string& name);

				/**
				 * \brief Get the raw const EVP_CIPHER pointer.
				 * \return The raw pointer.
				 */
				const EVP_CIPHER* raw() const;

				/**
				 * \brief Get the type (NID) of the cipher_algorithm.
				 * \return The type of the cipher_algorithm.
				 */
				int type() const;

				/**
				 * \brief Get the name of the cipher_algorithm.
				 * \return The name of the cipher_algorithm.
				 */
				std::string name() const;

				/**
				 * \brief Get the size of a block.
				 * \return The size of a block.
				 */
				size_t block_size() const;

				/**
				 * \brief Get the key length.
				 * \return The key length.
				 */
				size_t key_length() const;

				/**
				 * \brief Get the iv length.
				 * \return The iv length.
				 */
				size_t iv_length() const;

				/**
				 * \brief Get the flags.
				 * \return The flags.
				 */
				unsigned long flags() const;

				/**
				 * \brief Get the mode.
				 * \return The mode.
				 */
				unsigned long mode() const;

				/**
				 * \brief Check that the cipher_algorithm is null.
				 * \return true if the cipher_algorithm is null.
				 */
				bool operator!() const;

				/**
				 * \brief Check that the cipher_algorithm is not null.
				 * \return true if the cipher_algorithm is not null.
				 */
				bool boolean_test() const;

			private:

				const EVP_CIPHER* m_cipher;
		};

		inline cipher_algorithm::cipher_algorithm(const EVP_CIPHER* cipher) :
			m_cipher(cipher)
		{
		}

		inline const EVP_CIPHER* cipher_algorithm::raw() const
		{
			return m_cipher;
		}

		inline int cipher_algorithm::type() const
		{
			assert(m_cipher);

			return EVP_CIPHER_nid(m_cipher);
		}

		inline std::string cipher_algorithm::name() const
		{
			return std::string(OBJ_nid2sn(type()));
		}

		inline size_t cipher_algorithm::block_size() const
		{
			assert(m_cipher);

			return EVP_CIPHER_block_size(m_cipher);
		}

		inline size_t cipher_algorithm::key_length() const
		{
			assert(m_cipher);

			return EVP_CIPHER_key_length(m_cipher);
		}

		inline size_t cipher_algorithm::iv_length() const
		{
			assert(m_cipher);

			return EVP_CIPHER_iv_length(m_cipher);
		}

		inline unsigned long cipher_algorithm::flags() const
		{
			assert(m_cipher);

			return EVP_CIPHER_flags(m_cipher);
		}

		inline unsigned long cipher_algorithm::mode() const
		{
			assert(m_cipher);

			return EVP_CIPHER_mode(m_cipher);
		}

		inline bool cipher_algorithm::operator!() const
		{
			return !boolean_test();
		}

		inline bool cipher_algorithm::boolean_test() const
		{
			return (m_cipher != NULL);
		}
	}
}

#endif /* CRYPTOPLUS_CIPHER_CIPHER_ALGORITHM_HPP */


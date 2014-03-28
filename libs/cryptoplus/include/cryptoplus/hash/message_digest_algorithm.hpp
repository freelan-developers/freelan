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
 * \file message_digest_algorithm.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A message digest algorithm wrapper class.
 */

#ifndef CRYPTOPLUS_HASH_MESSAGE_DIGEST_ALGORITHM_HPP
#define CRYPTOPLUS_HASH_MESSAGE_DIGEST_ALGORITHM_HPP

#include <openssl/evp.h>

#include <string>

namespace cryptoplus
{
	namespace hash
	{
		/**
		 * \brief A message digest algorithm.
		 */
		class message_digest_algorithm
		{
			public:

				/**
				 * \brief Create a new message_digest_algorithm from a const EVP_MD pointer.
				 * \param md The raw const EVP_MD pointer. If md is NULL, the behavior is undefined.
				 */
				message_digest_algorithm(const EVP_MD* md);

				/**
				 * \brief Create a new message_digest_algorithm from its type (NID).
				 * \param type The type of the message_digest_algorithm to create.
				 * \warning If no such type is found, a std::invalid_argument is thrown.
				 */
				explicit message_digest_algorithm(int type);

				/**
				 * \brief Create a new message_digest_algorithm from its name.
				 * \param name The name of the message_digest_algorithm to create.
				 * \warning If no such name is found, a std::invalid_argument is thrown.
				 */
				explicit message_digest_algorithm(const std::string& name);

				/**
				 * \brief Get the raw const EVP_MD pointer.
				 * \return The raw pointer.
				 */
				const EVP_MD* raw() const;

				/**
				 * \brief Get the type (NID) of the message_digest_algorithm.
				 * \return The type of the message_digest_algorithm.
				 */
				int type() const;

				/**
				 * \brief Get the name of the message_digest_algorithm.
				 * \return The name of the message_digest_algorithm.
				 */
				std::string name() const;

				/**
				 * \brief Get the size of the generated digest message.
				 * \return The size of the generated digest message.
				 */
				size_t result_size() const;

				/**
				 * \brief Get the size of a block.
				 * \return The size of a block.
				 */
				size_t block_size() const;

			private:

				const EVP_MD* m_md;
		};

		inline message_digest_algorithm::message_digest_algorithm(const EVP_MD* md) :
			m_md(md)
		{
		}

		inline const EVP_MD* message_digest_algorithm::raw() const
		{
			return m_md;
		}

		inline int message_digest_algorithm::type() const
		{
			return EVP_MD_type(m_md);
		}

		inline std::string message_digest_algorithm::name() const
		{
			return std::string(OBJ_nid2sn(type()));
		}

		inline size_t message_digest_algorithm::result_size() const
		{
			return EVP_MD_size(m_md);
		}

		inline size_t message_digest_algorithm::block_size() const
		{
			return EVP_MD_block_size(m_md);
		}
	}
}

#endif /* CRYPTOPLUS_HASH_MESSAGE_DIGEST_ALGORITHM_HPP */


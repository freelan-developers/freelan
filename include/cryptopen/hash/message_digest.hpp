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
 * \file message_digest.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Message digest helper functions.
 */

#ifndef CRYPTOPEN_HASH_MESSAGE_DIGEST_HPP
#define CRYPTOPEN_HASH_MESSAGE_DIGEST_HPP

#include <openssl/evp.h>

#include <vector>
#include <string>

namespace cryptopen
{
	namespace hash
	{
		/**
		 * \brief Get a message digest by its name.
		 * \param name The message digest name.
		 * \return The message digest or NULL if no matching message digest is found.
		 * \see message_digest_name
		 * \warning An initializer must exist for this function to succeed.
		 * \see cryptopen::hash::message_digest_initializer
		 */
		const EVP_MD* get_message_digest_by_name(const std::string& name);

		/**
		 * \brief Get a message digest by its NID.
		 * \param nid The message digest NID.
		 * \return The message digest or NULL if no matching message digest is found.
		 * \see message_digest_type
		 */
		const EVP_MD* get_message_digest_by_type(int nid);

		/**
		 * \brief Compute a message digest for the given buffer, using the given digest method.
		 * \param out The output buffer. Must be at least message_digest_size(md) bytes long.
		 * \param out_len The output buffer length. 
		 * \param data The buffer.
		 * \param len The buffer length.
		 * \param md The digest method.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The count of bytes written to out. Should be equal to message_digest_size(md).
		 */
		size_t message_digest(void* out, size_t out_len, const void* data, size_t len, const EVP_MD* md, ENGINE* impl = NULL);

		/**
		 * \brief Compute a message digest for the given buffer, using the given digest method.
		 * \param data The buffer.
		 * \param len The buffer length.
		 * \param md The digest method.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The message digest.
		 */
		template <typename T>
			std::vector<T> message_digest(const void* data, size_t len, const EVP_MD* md, ENGINE* impl = NULL);

		/**
		 * \brief Get the size of a HMAC generated with the specified hash method.
		 * \param md The digest method.
		 * \return The size of a HMAC generated with md.
		 */
		size_t message_digest_size(const EVP_MD* md);

		inline const EVP_MD* get_message_digest_by_name(const std::string& name)
		{
			return EVP_get_digestbyname(name.c_str());
		}

		inline const EVP_MD* get_message_digest_by_type(int nid)
		{
			return EVP_get_digestbynid(nid);
		}

		template <typename T>
			inline std::vector<T> message_digest(const void* data, size_t len, const EVP_MD* md, ENGINE* impl)
			{
				std::vector<T> result(message_digest_size(md));

				message_digest(&result[0], result.size(), data, len, md, impl);

				return result;
			}

		inline size_t message_digest_size(const EVP_MD* md)
		{
			return EVP_MD_size(md);
		}
	}
}

#endif /* CRYPTOPEN_HASH_MESSAGE_DIGEST_HPP */


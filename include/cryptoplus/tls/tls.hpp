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
 * \file tls.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief TLS utility functions.
 */

#ifndef CRYPTOPLUS_TLS_TLS_HPP
#define CRYPTOPLUS_TLS_TLS_HPP

#include <string>

#include "../buffer.hpp"
#include "../hash/message_digest_algorithm.hpp"

namespace cryptoplus
{
	namespace tls
	{
		/**
		 * \brief Computes the P_hash as defined in TLS RFC 5246.
		 * \param out The output buffer. Must be at least as big as the message digest algorithm result size.
		 * \param out_len The output buffer length. Also the count of bytes requested.
		 * \param key The key to use.
		 * \param key_len The key length.
		 * \param data The first buffer of data.
		 * \param data_len The data length.
		 * \param data2 The second buffer of data.
		 * \param data2_len The data2 length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The count of bytes written to out. Should be equal to out_len.
		 */
		size_t p_hash(void* out, size_t out_len, const void* key, size_t key_len, const void* data, size_t data_len, const void* data2, size_t data2_len, const hash::message_digest_algorithm& algorithm, ENGINE* impl = NULL);

		/**
		 * \brief Computes the P_hash as defined in TLS RFC 5246.
		 * \param out_len The count of bytes requested.
		 * \param key The key to use.
		 * \param key_len The key length.
		 * \param data The buffer.
		 * \param data_llen The buffer length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The hash.
		 */
		inline buffer p_hash(size_t out_len, const void* key, size_t key_len, const void* data, size_t data_len, const void* data2, size_t data2_len, const hash::message_digest_algorithm& algorithm, ENGINE* impl = NULL)
		{
			buffer result(out_len);

			p_hash(buffer_cast<uint8_t*>(result), buffer_size(result), key, key_len, data, data_len, data2, data2_len, algorithm, impl);

			return result;
		}

		/**
		 * \brief TLS PRF function as defined in RFC 5246.
		 * \param out The output buffer. Must be at least as big as the message digest algorithm result size.
		 * \param out_len The output buffer length. Also the count of bytes requested.
		 * \param key The key to use.
		 * \param key_len The key length.
		 * \param label The label to use.
		 * \param label_len The label length.
		 * \param seed The seed. Typically the concatenation of the client and server randoms.
		 * \param seed_len The seed length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The count of bytes written to out. Should be equal to out_len.
		 */
		inline size_t prf(void* out, size_t out_len, const void* key, size_t key_len, const void* label, size_t label_len, const void* seed, size_t seed_len, const hash::message_digest_algorithm& algorithm, ENGINE* impl = NULL)
		{
			return p_hash(out, out_len, key, key_len, label, label_len, seed, seed_len, algorithm, impl);
		}

		/**
		 * \brief TLS PRF function as defined in RFC 5246.
		 * \param out_len The count of bytes requested.
		 * \param key The key to use.
		 * \param key_len The key length.
		 * \param label The label to use.
		 * \param seed The seed. Typically the concatenation of the client and server randoms.
		 * \param seed_len The seed length.
		 * \param algorithm The message digest algorithm to use.
		 * \param impl The engine to use. The NULL default value indicate that no engine should be used.
		 * \return The hash.
		 */
		inline buffer prf(size_t out_len, const void* key, size_t key_len, const std::string& label, const void* seed, size_t seed_len, const hash::message_digest_algorithm& algorithm, ENGINE* impl = NULL)
		{
			buffer result(out_len);

			prf(buffer_cast<uint8_t*>(result), buffer_size(result), key, key_len, label.c_str(), label.size(), seed, seed_len, algorithm, impl);

			return result;
		}

	}
}

#endif /* CRYPTOPLUS_TLS_TLS_HPP */

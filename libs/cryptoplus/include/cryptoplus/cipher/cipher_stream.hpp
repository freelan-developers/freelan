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
 * \file cipher_stream.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A cipher stream class.
 */

#ifndef CRYPTOPLUS_CIPHER_CIPHER_STREAM_HPP
#define CRYPTOPLUS_CIPHER_CIPHER_STREAM_HPP

#include "../buffer.hpp"

#include "cipher_context.hpp"

#include <cstring>

namespace cryptoplus
{
	namespace cipher
	{
		/**
		 * \brief A cipher stream class.
		 *
		 * The cipher_stream class ease the computation of a cipher, using a stream oriented interface.
		 *
		 * To work with cipher_stream, call the initialize() method like you would do on a cipher_context, call append() or operator<<() as long as you have data to cipher then call finalize(). The resulting ciphered buffer can be get by calling result().
		 *
		 * The list of the available cipher methods depends on the version of OpenSSL and can be found on the man page of EVP_EncryptInit().
		 *
		 * cipher_stream is noncopyable by design.
		 */
		class cipher_stream : private cipher_context
		{
			public:

				using cipher_context::cipher_direction;
				using cipher_context::unchanged;
				using cipher_context::decrypt;
				using cipher_context::encrypt;
				using cipher_context::initialize;
				using cipher_context::set_padding;
				using cipher_context::set_key_length;
				using cipher_context::key_length;
				using cipher_context::ctrl_get;
				using cipher_context::ctrl_set;
				using cipher_context::algorithm;

				/**
				 * \brief Create a new cipher stream.
				 * \param alloc The minimum number of bytes to pre-allocate. A good value here is the count of bytes to cipher + cipher algorithm block size.
				 * \see initalize()
				 *
				 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_EncryptInit().
				 */
				explicit cipher_stream(size_t alloc);

				/**
				 * \brief Append data to the stream.
				 * \param buf The data to append to the stream.
				 * \param buf_len The length of buf.
				 * \return The cipher stream.
				 */
				cipher_stream& append(const void* buf, size_t buf_len);

				/**
				 * \brief Append data to the stream.
				 * \param buf The data to append to the stream.
				 * \return The cipher stream.
				 */
				cipher_stream& append(const buffer& buf);

				/**
				 * \brief Append a C-string to the stream.
				 * \param cstr The C-string to add. No terminal null-characater is appended.
				 * \return The cipher stream.
				 */
				cipher_stream& append(const char* cstr);

				/**
				 * \brief Append a string to the stream.
				 * \param str The string to add. No terminal null-characater is appended.
				 * \return The cipher stream.
				 */
				cipher_stream& append(const std::string& str);

				/**
				 * \brief Finalize the stream input.
				 * \see result()
				 */
				void finalize();

				/**
				 * \brief Reallocate the internal buffer.
				 * \param alloc The minimum number of bytes to pre-allocate. A good value here is the count of bytes to cipher + cipher algorithm block size.
				 * \warning Avoid reallocations while a encrypt/decrypt sequence is pending. At any time, if the streams needs more output space, it will automatically reallocate() the underlying buffer.
				 */
				void reallocate(size_t alloc);

				/**
				 * \brief Get the result buffer.
				 * \return The result buffer.
				 * \warning Be sure to call finalize() before calling this method.
				 * \see finalize()
				 */
				const buffer& result() const;

			private:

				using cipher_context::update;
				using cipher_context::finalize;

				buffer m_buffer;
				size_t m_offset;
		};

		/**
		 * \brief Append a value to a cipher_stream.
		 * \param cs The cipher_stream.
		 * \param value The value.
		 * \return The cipher_stream.
		 */
		template <typename T>
		cipher_stream& operator<<(cipher_stream& cs, const T& value);

		inline cipher_stream::cipher_stream(size_t alloc) :
			m_buffer(alloc), m_offset(0)
		{
		}

		inline cipher_stream& cipher_stream::append(const buffer& buf)
		{
			return append(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}

		inline cipher_stream& cipher_stream::append(const char* cstr)
		{
			return append(cstr, std::strlen(cstr));
		}

		inline cipher_stream& cipher_stream::append(const std::string& str)
		{
			return append(str.c_str(), str.size());
		}

		inline void cipher_stream::reallocate(size_t alloc)
		{
			m_buffer.data().resize(alloc);
		}

		inline const buffer& cipher_stream::result() const
		{
			return m_buffer;
		}

		template <typename T>
		inline cipher_stream& operator<<(cipher_stream& cs, const T& value)
		{
			return cs.append(value);
		}
	}
}

#endif /* CRYPTOPLUS_CIPHER_CIPHER_STREAM_HPP */


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
 * \file string.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_STRING pointer class.
 */

#ifndef CRYPTOPLUS_ASN1_STRING_HPP
#define CRYPTOPLUS_ASN1_STRING_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"

#include <openssl/crypto.h>
#include <openssl/asn1.h>

#include <vector>
#include <string>

namespace cryptoplus
{
	namespace asn1
	{
		/**
		 * \brief An OpenSSL ASN1_STRING pointer.
		 *
		 * The string class is a wrapper for an OpenSSL ASN1_STRING* pointer.
		 *
		 * A string instance has the same semantic as a ASN1_STRING* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the string not to be NULL before calling any of its method. Calling any method (except raw()) on a null string has undefined behavior.
		 */
		class string : public pointer_wrapper<ASN1_STRING>
		{
			public:

				/**
				 * \brief Create a new string.
				 * \return The string.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static string create();

				/**
				 * \brief Take ownership of a specified ASN1_STRING pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An string.
				 */
				static string take_ownership(pointer ptr);

				/**
				 * \brief Create an ASN1 string from some data.
				 * \param buf The data buffer.
				 * \param buf_len The length of buf.
				 * \return The ASN1 string.
				 */
				static string from_data(const void* buf, size_t buf_len);

				/**
				 * \brief Create an ASN1 string from some data.
				 * \param buf The data buffer.
				 * \return The ASN1 string.
				 */
				static string from_data(const buffer& buf);

				/**
				 * \brief Create an ASN1 string from a C-string.
				 * \param str The string.
				 * \return The ASN1 string.
				 */
				static string from_string(const char* str);

				/**
				 * \brief Create an ASN1 string from a std::string.
				 * \param str The string.
				 * \return The ASN1 string.
				 */
				static string from_string(const std::string& str);

				/**
				 * \brief Create a new empty string.
				 */
				string();

				/**
				 * \brief Create an string by *NOT* taking ownership of an existing ASN1_STRING* pointer.
				 * \param ptr The ASN1_STRING* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				string(pointer ptr);

				/**
				 * \brief Get the size of the string.
				 * \return The size of the string.
				 */
				size_t size() const;

				/**
				 * \brief Get the string data.
				 * \return The string data.
				 */
				const unsigned char* data();

				/**
				 * \brief Set the internal data.
				 * \param data The data.
				 * \param data_len The length of data.
				 */
				void set_data(const void* data, size_t data_len) const;

				/**
				 * \brief Set the internal data.
				 * \param data The data.
				 */
				void set_data(const buffer& data) const;

				/**
				 * \brief Set the internal data.
				 * \param data The data, as a null terminated ASCII C-string.
				 */
				void set_data(const char* data) const;

				/**
				 * \brief Set the internal data.
				 * \param data The data, as an ASCII string.
				 */
				void set_data(const std::string& data) const;

				/**
				 * \brief Get the type of the string.
				 * \return The type.
				 */
				int type() const;

				/**
				 * \brief Build a string from data() and size().
				 * \return A string built from data() and that will be size() bytes long.
				 */
				std::string str();

				/**
				 * \brief Get the content as an UTF-8 string.
				 * \return The UTF-8 content.
				 */
				std::string to_utf8() const;

			private:

				explicit string(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two string instance share the same underlying pointer.
		 */
		bool operator==(const string& lhs, const string& rhs);

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two string instance do not share the same underlying pointer.
		 */
		bool operator!=(const string& lhs, const string& rhs);

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return 0 if the two ASN1 strings are identical.
		 */
		int compare(const string& lhs, const string& rhs);

		inline string string::create()
		{
			return take_ownership(ASN1_STRING_new());
		}
		inline string string::from_data(const void* buf, size_t buf_len)
		{
			string result = create();
			result.set_data(buf, buf_len);

			return result;
		}
		inline string string::from_data(const buffer& buf)
		{
			string result = create();
			result.set_data(buf);

			return result;
		}
		inline string string::from_string(const char* str)
		{
			string result = create();
			result.set_data(str);

			return result;
		}
		inline string string::from_string(const std::string& str)
		{
			string result = create();
			result.set_data(str);

			return result;
		}
		inline string::string()
		{
		}
		inline string::string(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline size_t string::size() const
		{
			return ASN1_STRING_length(ptr().get());
		}
		inline const unsigned char* string::data()
		{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
			return ASN1_STRING_get0_data(ptr().get());
#else
			return ASN1_STRING_data(ptr().get());
#endif
		}
		inline void string::set_data(const void* _data, size_t data_len) const
		{
			throw_error_if_not(ASN1_STRING_set(ptr().get(), _data, static_cast<int>(data_len)) != 0);
		}
		inline void string::set_data(const buffer& _data) const
		{
			set_data(buffer_cast<const uint8_t*>(_data), buffer_size(_data));
		}
		inline void string::set_data(const char* _data) const
		{
			throw_error_if_not(ASN1_STRING_set(ptr().get(), _data, -1) != 0);
		}
		inline void string::set_data(const std::string& _data) const
		{
			set_data(_data.c_str());
		}
		inline int string::type() const
		{
			return ASN1_STRING_type(ptr().get());
		}
		inline std::string string::str()
		{
			return std::string(reinterpret_cast<const char*>(data()), size());
		}
		inline string::string(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const string& lhs, const string& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const string& lhs, const string& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline int compare(const string& lhs, const string& rhs)
		{
#if OPENSSL_VERSION_NUMBER >= 0x01000000 && !defined(LIBRESSL_VERSION_NUMBER)
			return ASN1_STRING_cmp(lhs.raw(), rhs.raw());
#else
			return ASN1_STRING_cmp(const_cast<string::pointer>(lhs.raw()), const_cast<string::pointer>(rhs.raw()));
#endif
		}
	}
}

#endif /* CRYPTOPLUS_ASN1_STRING_HPP */

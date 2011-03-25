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
 * \file string_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_STRING pointer class.
 */

#ifndef CRYPTOPEN_ASN1_STRING_PTR_HPP
#define CRYPTOPEN_ASN1_STRING_PTR_HPP

#include "../error/cryptographic_exception.hpp"
#include "../nullable.hpp"

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
		 * The string_ptr class is a wrapper for an OpenSSL ASN1_STRING* pointer.
		 *
		 * A string_ptr instance has the same semantic as a ASN1_STRING* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A string_ptr *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a string_ptr always points to a valid ASN1_STRING structure.
		 *
		 * \warning Always check for the string_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset_ptr()) on a null string_ptr has undefined behavior.
		 */
		class string_ptr : public nullable<string_ptr>
		{
			public:

				/**
				 * \brief Create a new string_ptr.
				 * \param string The ASN1_STRING to point to.
				 */
				string_ptr(ASN1_STRING* string = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param string The ASN1_STRING to point to.
				 */
				void reset_ptr(ASN1_STRING* string = NULL);

				/**
				 * \brief Get the raw ASN1_string pointer.
				 * \return The raw ASN1_STRING pointer.
				 */
				ASN1_STRING* raw();

				/**
				 * \brief Get the raw ASN1_string pointer.
				 * \return The raw ASN1_STRING pointer.
				 */
				const ASN1_STRING* raw() const;

				/**
				 * \brief Get the size of the string.
				 * \return The size of the string.
				 */
				size_t size();

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
				void set_data(const void* data, size_t data_len);

				/**
				 * \brief Set the internal data.
				 * \param data The data, as a null terminated ASCII C-string.
				 */
				void set_data(const char* data);

				/**
				 * \brief Set the internal data.
				 * \param data The data, as an ASCII string.
				 */
				void set_data(const std::string& data);

				/**
				 * \brief Get the type of the string.
				 * \return The type.
				 */
				int type();

				/**
				 * \brief Build a string from data() and size().
				 * \return A string built from data() and that will be size() bytes long.
				 */
				std::string str();

				/**
				 * \brief Get the content as an UTF-8 string.
				 * \return The UTF-8 content.
				 */
				std::vector<unsigned char> to_utf8();

			private:

				bool boolean_test() const;

				ASN1_STRING* m_string;
		};

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two string_ptr instance share the same underlying pointer.
		 */
		bool operator==(const string_ptr& lhs, const string_ptr& rhs);

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two string_ptr instance do not share the same underlying pointer.
		 */
		bool operator!=(const string_ptr& lhs, const string_ptr& rhs);

		/**
		 * \brief Compare two ASN1 string pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return 0 if the two ASN1 strings are identical.
		 */
		int compare(const string_ptr& lhs, const string_ptr& rhs);

		inline string_ptr::string_ptr(ASN1_STRING* _string) : m_string(_string)
		{
		}
		inline void string_ptr::reset_ptr(ASN1_STRING* _string)
		{
			m_string = _string;
		}
		inline ASN1_STRING* string_ptr::raw()
		{
			return m_string;
		}
		inline const ASN1_STRING* string_ptr::raw() const
		{
			return m_string;
		}
		inline size_t string_ptr::size()
		{
			return ASN1_STRING_length(m_string);
		}
		inline const unsigned char* string_ptr::data()
		{
			return ASN1_STRING_data(m_string);
		}
		inline void string_ptr::set_data(const void* _data, size_t data_len)
		{
			error::throw_error_if_not(ASN1_STRING_set(m_string, _data, data_len));
		}
		inline void string_ptr::set_data(const char* _data)
		{
			error::throw_error_if_not(ASN1_STRING_set(m_string, _data, -1));
		}
		inline void string_ptr::set_data(const std::string& _data)
		{
			set_data(_data.c_str());
		}
		inline int string_ptr::type()
		{
			return ASN1_STRING_type(m_string);
		}
		inline std::string string_ptr::str()
		{
			return std::string(reinterpret_cast<const char*>(data()), size());
		}
		inline bool string_ptr::boolean_test() const
		{
			return (m_string != NULL);
		}
		inline bool operator==(const string_ptr& lhs, const string_ptr& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const string_ptr& lhs, const string_ptr& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline int compare(const string_ptr& lhs, const string_ptr& rhs)
		{
			return ASN1_STRING_cmp(lhs.raw(), rhs.raw());
		}
	}
}

#endif /* CRYPTOPEN_ASN1_STRING_PTR_HPP */


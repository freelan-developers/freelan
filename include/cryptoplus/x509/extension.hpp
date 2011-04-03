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
 * \file extension.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 extension class.
 */

#ifndef CRYPTOPEN_X509_EXTENSION_HPP
#define CRYPTOPEN_X509_EXTENSION_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../asn1/object.hpp"
#include "../asn1/string.hpp"

#include <openssl/x509.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 extension.
		 *
		 * The extension class represents a X509 extension.
		 *
		 * A extension instance has the same semantic as a X509_EXTENSION* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class extension : public pointer_wrapper<X509_EXTENSION>
		{
			public:

				/**
				 * \brief Create a new extension.
				 * \return The extension.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static extension create();

				/**
				 * \brief Take ownership of a specified X509_EXTENSION pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A extension_entry.
				 */
				static extension take_ownership(pointer ptr);

				/**
				 * \brief Create an extension from a nid and its value.
				 * \param nid The nid.
				 * \param critical The critical flag.
				 * \param value The value.
				 */
				static extension from_nid(int nid, bool critical, asn1::string value);

				/**
				 * \brief Create an extension from an ASN1 object and its value.
				 * \param obj The ASN1 object.
				 * \param critical The critical flag.
				 * \param value The value.
				 */
				static extension from_obj(asn1::object obj, bool critical, asn1::string value);

				/**
				 * \brief Create a new empty extension.
				 */
				extension();

				/**
				 * \brief Create a X509 extension by *NOT* taking ownership of an existing X509_EXTENSION* pointer.
				 * \param ptr The X509_EXTENSION* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				extension(pointer ptr);

				/**
				 * \brief Clone the extension instance.
				 * \return The clone.
				 */
				extension clone() const;

			private:

				extension(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two extension instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two extension instances share the same underlying pointer.
		 */
		bool operator==(const extension& lhs, const extension& rhs);

		/**
		 * \brief Compare two extension instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two extension instances do not share the same underlying pointer.
		 */
		bool operator!=(const extension& lhs, const extension& rhs);

		inline extension extension::create()
		{
			pointer _ptr = X509_EXTENSION_new();

			error::throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline extension extension::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return extension(_ptr, deleter);
		}
		inline extension extension::from_nid(int nid, bool critical, asn1::string value)
		{
			return take_ownership(X509_EXTENSION_create_by_NID(NULL, nid, critical ? 1 : 0, value.raw()));
		}
		inline extension extension::from_obj(asn1::object obj, bool critical, asn1::string value)
		{
			return take_ownership(X509_EXTENSION_create_by_OBJ(NULL, obj.raw(), critical ? 1 : 0, value.raw()));
		}
		inline extension::extension()
		{
		}
		inline extension::extension(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline extension extension::clone() const
		{
			return extension(X509_EXTENSION_dup(ptr().get()));
		}
		inline extension::extension(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const extension& lhs, const extension& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const extension& lhs, const extension& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_X509_EXTENSION_HPP */


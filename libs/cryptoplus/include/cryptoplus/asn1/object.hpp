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
 * \file object.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_OBJECT pointer class.
 */

#ifndef CRYPTOPLUS_ASN1_OBJECT_HPP
#define CRYPTOPLUS_ASN1_OBJECT_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"

#include <openssl/objects.h>

#include <vector>
#include <string>

namespace cryptoplus
{
	namespace asn1
	{
		/**
		 * \brief An OpenSSL ASN1_OBJECT pointer.
		 *
		 * The object class is a wrapper for an OpenSSL ASN1_OBJECT* pointer.
		 *
		 * A object instance has the same semantic as a ASN1_OBJECT* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class object : public pointer_wrapper<ASN1_OBJECT>
		{
			public:

				/**
				 * \brief Create a new object.
				 * \return The object.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static object create();

				/**
				 * \brief Take ownership of a specified ASN1_OBJECT pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An object.
				 */
				static object take_ownership(pointer ptr);

				/**
				 * \brief Create an ASN1 object pointer from a nid.
				 * \param nid The nid.
				 * \return The ASN1 object pointer.
				 */
				static object from_nid(int nid);

				/**
				 * \brief Create a new empty object.
				 */
				object();

				/**
				 * \brief Create an object by *NOT* taking ownership of an existing ASN1_OBJECT* pointer.
				 * \param ptr The ASN1_OBJECT* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				object(pointer ptr);

				/**
				 * \brief Get the nid associated to the specified object.
				 * \return The nid.
				 */
				int to_nid() const;

			private:

				explicit object(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two ASN1 object pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two object instance share the same underlying pointer.
		 */
		bool operator==(const object& lhs, const object& rhs);

		/**
		 * \brief Compare two ASN1 object pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two object instance do not share the same underlying pointer.
		 */
		bool operator!=(const object& lhs, const object& rhs);

		/**
		 * \brief Compare two ASN1 object pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return 0 if the two ASN1 objects are identical.
		 */
		int compare(const object& lhs, const object& rhs);

		inline object object::create()
		{
			return take_ownership(ASN1_OBJECT_new());
		}
		inline object object::from_nid(int nid)
		{
			return OBJ_nid2obj(nid);
		}
		inline object::object()
		{
		}
		inline object::object(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline int object::to_nid() const
		{
			return OBJ_obj2nid(ptr().get());
		}
		inline object::object(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const object& lhs, const object& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const object& lhs, const object& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline int compare(const object& lhs, const object& rhs)
		{
			return OBJ_cmp(lhs.raw(), rhs.raw());
		}
	}
}

#endif /* CRYPTOPLUS_ASN1_OBJECT_HPP */


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
 * \file object_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_OBJECT pointer class.
 */

#ifndef CRYPTOPEN_ASN1_OBJECT_PTR_HPP
#define CRYPTOPEN_ASN1_OBJECT_PTR_HPP

#include "../nullable.hpp"

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
		 * The object_ptr class is a wrapper for an OpenSSL ASN1_OBJECT* pointer.
		 *
		 * A object_ptr instance has the same semantic as a ASN1_OBJECT* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A object_ptr *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a object_ptr always points to a valid ASN1_OBJECT structure.
		 *
		 * \warning Always check for the object_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset_ptr()) on a null object_ptr has undefined behavior.
		 */
		class object_ptr : public nullable<object_ptr>
		{
			public:

				/**
				 * \brief Create an ASN1 object pointer from a nid.
				 * \param nid The nid.
				 * \return The ASN1 object pointer.
				 */
				static object_ptr from_nid(int nid);

				/**
				 * \brief Create a new object_ptr.
				 * \param object The ASN1_OBJECT to point to.
				 */
				object_ptr(ASN1_OBJECT* object = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param object The ASN1_OBJECT to point to.
				 */
				void reset_ptr(ASN1_OBJECT* object = NULL);

				/**
				 * \brief Get the raw ASN1_OBJECT pointer.
				 * \return The raw ASN1_OBJECT pointer.
				 */
				ASN1_OBJECT* raw();

				/**
				 * \brief Get the raw ASN1_OBJECT pointer.
				 * \return The raw ASN1_OBJECT pointer.
				 */
				const ASN1_OBJECT* raw() const;

				/**
				 * \brief Get the nid associated to the specified object.
				 * \return The nid.
				 */
				int to_nid() const;

			private:

				bool boolean_test() const;

				ASN1_OBJECT* m_object;
		};

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two object_ptr instance share the same underlying pointer.
		 */
		bool operator==(const object_ptr& lhs, const object_ptr& rhs);

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two object_ptr instance do not share the same underlying pointer.
		 */
		bool operator!=(const object_ptr& lhs, const object_ptr& rhs);

		inline object_ptr object_ptr::from_nid(int nid)
		{
			return OBJ_nid2obj(nid);
		}
		inline object_ptr::object_ptr(ASN1_OBJECT* _object) : m_object(_object)
		{
		}
		inline void object_ptr::reset_ptr(ASN1_OBJECT* _object)
		{
			m_object = _object;
		}
		inline ASN1_OBJECT* object_ptr::raw()
		{
			return m_object;
		}
		inline const ASN1_OBJECT* object_ptr::raw() const
		{
			return m_object;
		}
		inline int object_ptr::to_nid() const
		{
			return OBJ_obj2nid(m_object);
		}
		inline bool object_ptr::boolean_test() const
		{
			return (m_object != NULL);
		}
		inline bool operator==(const object_ptr& lhs, const object_ptr& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const object_ptr& lhs, const object_ptr& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_ASN1_OBJECT_PTR_HPP */


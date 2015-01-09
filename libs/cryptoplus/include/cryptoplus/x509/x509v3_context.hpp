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
 * \file x509v3_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 V3 context class.
 */

#ifndef CRYPTOPLUS_X509_X509V3_CONTEXT_HPP
#define CRYPTOPLUS_X509_X509V3_CONTEXT_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"

#include <openssl/x509v3.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 V3 context.
		 *
		 * The x509v3_context class represents a X509 V3 context.
		 *
		 * A x509v3_context instance has the same semantic as a X509V3_CTX* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class x509v3_context : public pointer_wrapper<X509V3_CTX>
		{
			public:

				/**
				 * \brief Create a new x509v3_context.
				 * \return The x509v3_context.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static x509v3_context create();

				/**
				 * \brief Take ownership of a specified X509_X509V3_CONTEXT pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A x509v3_context_entry.
				 */
				static x509v3_context take_ownership(pointer ptr);

				/**
				 * \brief Create a new empty x509v3_context.
				 */
				x509v3_context();

				/**
				 * \brief Create a X509 x509v3_context by *NOT* taking ownership of an existing X509_X509V3_CONTEXT* pointer.
				 * \param ptr The X509_X509V3_CONTEXT* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				x509v3_context(pointer ptr);

			private:

				x509v3_context(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two x509v3_context instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two x509v3_context instances share the same underlying pointer.
		 */
		bool operator==(const x509v3_context& lhs, const x509v3_context& rhs);

		/**
		 * \brief Compare two x509v3_context instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two x509v3_context instances do not share the same underlying pointer.
		 */
		bool operator!=(const x509v3_context& lhs, const x509v3_context& rhs);

		inline x509v3_context x509v3_context::create()
		{
			pointer _ptr = new X509V3_CTX();

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline x509v3_context::x509v3_context()
		{
		}
		inline x509v3_context::x509v3_context(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline x509v3_context::x509v3_context(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const x509v3_context& lhs, const x509v3_context& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const x509v3_context& lhs, const x509v3_context& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_X509V3_CONTEXT_HPP */


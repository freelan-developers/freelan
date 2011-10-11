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
 * \file store.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 store class.
 */

#ifndef CRYPTOPLUS_X509_STORE_HPP
#define CRYPTOPLUS_X509_STORE_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 certificate store.
		 *
		 * The store class represents a X509 certificate store.
		 *
		 * A store instance has the same semantic as a X509_STORE* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class store: public pointer_wrapper<X509_STORE>
		{
			public:

				/**
				 * \brief A verification callback type.
				 */
				typedef int (*verification_callback_type)(int, X509_STORE_CTX*);

				/**
				 * \brief Create a new store.
				 * \return The store.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static store create();

				/**
				 * \brief Take ownership of a specified X509_STORE pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A store.
				 */
				static store take_ownership(pointer ptr);

				/**
				 * \brief Create a new empty X509 store.
				 */
				store();

				/**
				 * \brief Create a X509 store by *NOT* taking ownership of an existing X509_STORE* pointer.
				 * \param ptr The X509_STORE* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				store(pointer ptr);

				/**
				 * \brief Set the verification callback.
				 * \param callback The verification callback.
				 */
				void set_verification_callback(verification_callback_type callback);

			private:

				store(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two store instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two store instances share the same underlying pointer.
		 */
		bool operator==(const store& lhs, const store& rhs);

		/**
		 * \brief Compare two store instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two store instances do not share the same underlying pointer.
		 */
		bool operator!=(const store& lhs, const store& rhs);

		inline store store::create()
		{
			pointer _ptr = X509_STORE_new();

			error::throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline store store::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return store(_ptr, deleter);
		}
		inline store::store()
		{
		}
		inline store::store(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void store::set_verification_callback(verification_callback_type callback)
		{
			X509_STORE_set_verify_cb(raw(), callback);
		}
		inline store::store(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const store& lhs, const store& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const store& lhs, const store& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_STORE_HPP */


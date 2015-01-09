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
#include "../error/helpers.hpp"
#include "certificate.hpp"
#include "certificate_revocation_list.hpp"
#include "verify_param.hpp"

#include <openssl/x509.h>
#include <openssl/x509v3.h>

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
				 * If allocation fails, an exception is thrown.
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

#ifdef X509_STORE_set_verify_cb
				/**
				 * \brief Set the verification callback.
				 * \param callback The verification callback.
				 */
				void set_verification_callback(verification_callback_type callback);
#endif

				/**
				 * \brief Set the verification parameters.
				 * \param vp The verification parameters.
				 */
				void set_verification_parameters(verify_param vp);

				/**
				 * \brief Set the verification flags.
				 * \param flags The verification flags.
				 */
				void set_verification_flags(int flags);

				/**
				 * \brief Add a lookup method to the store.
				 * \param lookup_method The lookup method to add.
				 * \return The new created X509_LOOKUP. Return value is never NULL.
				 */
				X509_LOOKUP* add_lookup_method(X509_LOOKUP_METHOD* lookup_method);

				/**
				 * \brief Add a file and a directory to the loaded locations for certificates and CRLs.
				 * \param file A certificate or CRL file.
				 * \param dir A directory containing hash named certificates or CRLs.
				 */
				void load_locations(const char* file, const char* dir);

				/**
				 * \brief Add a certificate to the store.
				 * \param cert The certificate.
				 */
				void add_certificate(certificate cert);

				/**
				 * \brief Add a CRL to the store.
				 * \param crl The CRL.
				 */
				void add_certificate_revocation_list(certificate_revocation_list crl);

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

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline store::store()
		{
		}
		inline store::store(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
#ifdef X509_STORE_set_verify_cb
		inline void store::set_verification_callback(verification_callback_type callback)
		{
			X509_STORE_set_verify_cb(raw(), callback);
		}
#endif
		inline void store::set_verification_parameters(verify_param vp)
		{
			X509_STORE_set1_param(raw(), vp.raw());
		}
		inline void store::set_verification_flags(int flags)
		{
			X509_STORE_set_flags(raw(), flags);
		}
		inline X509_LOOKUP* store::add_lookup_method(X509_LOOKUP_METHOD* lookup_method)
		{
			X509_LOOKUP* lookup = X509_STORE_add_lookup(raw(), lookup_method);
			throw_error_if_not(lookup != NULL);
			return lookup;
		}
		inline void store::load_locations(const char* file, const char* dir)
		{
			throw_error_if_not(X509_STORE_load_locations(raw(), const_cast<char*>(file), const_cast<char*>(dir)) != 0);
		}
		inline void store::add_certificate(certificate cert)
		{
			throw_error_if_not(X509_STORE_add_cert(raw(), cert.raw()) != 0);
		}
		inline void store::add_certificate_revocation_list(certificate_revocation_list crl)
		{
			throw_error_if_not(X509_STORE_add_crl(raw(), crl.raw()) != 0);
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


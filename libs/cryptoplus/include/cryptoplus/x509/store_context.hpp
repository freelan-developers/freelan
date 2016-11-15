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
 * \file store_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 store context class.
 */

#ifndef CRYPTOPLUS_X509_STORE_CONTEXT_HPP
#define CRYPTOPLUS_X509_STORE_CONTEXT_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"

#include "store.hpp"
#include "certificate.hpp"
#include "verify_param.hpp"

#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 certificate store context.
		 *
		 * The store class represents a X509 certificate store context.
		 *
		 * A store instance has the same semantic as a X509_STORE_CTX* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class store_context: public pointer_wrapper<X509_STORE_CTX>
		{
			public:

				/**
				 * \brief Register a new index for external data.
				 * \param argl A long argument to pass to the functions.
				 * \param argp A pointer argument to pass to the functions.
				 * \param new_func The function to call when a new store_context is allocated.
				 * \param dup_func The function to call when a store_context is duplicated.
				 * \param free_func The function to call when a store_context is deleted.
				 * \return The new index.
				 */
				static int register_index(long argl, void* argp, CRYPTO_EX_new* new_func, CRYPTO_EX_dup* dup_func, CRYPTO_EX_free* free_func);

				/**
				 * \brief Register a new index for external data.
				 * \return The new index.
				 *
				 * This method is equivalent to store_context::register_index(0, NULL, NULL, NULL, NULL).
				 */
				static int register_index();

				/**
				 * \brief A verification callback type.
				 */
				typedef int (*verification_callback_type)(int, X509_STORE_CTX*);

				/**
				 * \brief Create a new store context.
				 * \return The store context.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static store_context create();

				/**
				 * \brief Take ownership of a specified X509_STORE_CTX pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A store.
				 */
				static store_context take_ownership(pointer ptr);

				/**
				 * \brief Create a new empty X509 store context.
				 */
				store_context();

				/**
				 * \brief Create a X509 store context by *NOT* taking ownership of an existing X509_STORE_CTX* pointer.
				 * \param ptr The X509_STORE_CTX* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				store_context(pointer ptr);

				/**
				 * \brief Set the verification callback.
				 * \param callback The verification callback.
				 */
				void set_verification_callback(verification_callback_type callback);

				/**
				 * \brief Initialize the store context.
				 * \param _store The store to use, if any. Can be NULL.
				 * \param cert The certificate to be verified. Can be NULL.
				 * \param chain Additional untrusted certificates that might be used to build the chain. Can be NULL.
				 */
				void initialize(store _store, certificate cert, STACK_OF(X509)* chain);

				/**
				 * \brief Cleanup the store context.
				 *
				 * The context can then be reused and you can call initialize() again.
				 */
				void cleanup();

				/**
				 * \brief Set the external data.
				 * \param index The index, as returned by register_index().
				 * \param data The data.
				 */
				void set_external_data(int index, void* data);

				/**
				 * \brief Get the external data.
				 * \param index The index, as returned by register_index().
				 * \return The data.
				 */
				void* get_external_data(int index);

				/**
				 * \brief Set a trusted stack of certificates.
				 * \param certs The trusted stack of certificates to set.
				 *
				 * This is an alternative way of specifying trusted certificates instead of using a x509::store.
				 */
				void set_trusted_certificates(STACK_OF(X509)* certs);

				/**
				 * \brief Set the certificate to be verified.
				 * \param cert The certificate to be verified.
				 */
				void set_certificate(certificate cert);

				/**
				 * \brief Set the additional untrusted certificates that might be used to build the chain.
				 * \param chain The certificates.
				 */
				void set_chain(STACK_OF(X509)* chain);

				/**
				 * \brief Set the CRL to use to aid certificate verification. These CRLs will only be used if CRL verification is enable in the associated x509::verify_param structure.
				 * \param crls The CRLs.
				 */
				void set_crls(STACK_OF(X509_CRL)* crls);

				/**
				 * \brief Get the associated verification parameters.
				 * \return The associated verification parameters.
				 */
				verify_param verification_parameters() const;

				/**
				 * \brief Set the verification parameters.
				 * \param vp The verification parameters. vp should not be used after this method was called.
				 *
				 * \warning The store_context instance takes ownership on vp and is responsible for his deletion.
				 */
				void set_verification_parameters(verify_param vp);

				/**
				 * \brief Lookups and sets the default verification method.
				 * \param name The verification method name.
				 */
				void set_default(const char* name);

				/**
				 * \brief Lookups and sets the default verification method.
				 * \param name The verification method name.
				 */
				void set_default(const std::string& name);

				/**
				 * \brief Get the error.
				 * \return The error.
				 */
				long get_error() const;

				/**
				 * \brief Get the error string.
				 * \return The error string.
				 */
				const char* get_error_string() const;

				/**
				 * \brief Get the error depth.
				 * \return The error depth.
				 */
				int get_error_depth() const;

				/**
				 * \brief Get the certificate where the error occurred.
				 * \return The certificate where the error occurred. May be null.
				 */
				certificate get_current_certificate() const;

				/**
				 * \brief Verifies the certificate.
				 * \return true on success, false if the certificate could not be verified.
				 */
				bool verify();

			private:

				store_context(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two store_context instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two store_context instances share the same underlying pointer.
		 */
		bool operator==(const store_context& lhs, const store_context& rhs);

		/**
		 * \brief Compare two store_context instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two store_context instances do not share the same underlying pointer.
		 */
		bool operator!=(const store_context& lhs, const store_context& rhs);

		inline int store_context::register_index(long argl, void* argp, CRYPTO_EX_new* new_func, CRYPTO_EX_dup* dup_func, CRYPTO_EX_free* free_func)
		{
			int index = X509_STORE_CTX_get_ex_new_index(argl, argp, new_func, dup_func, free_func);

			throw_error_if(index < 0);

			return index;
		}
		inline int store_context::register_index()
		{
			return register_index(0, NULL, NULL, NULL, NULL);
		}
		inline store_context store_context::create()
		{
			pointer _ptr = X509_STORE_CTX_new();

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline store_context::store_context()
		{
		}
		inline store_context::store_context(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void store_context::set_verification_callback(verification_callback_type callback)
		{
			X509_STORE_CTX_set_verify_cb(raw(), callback);
		}
		inline void store_context::initialize(store _store, certificate cert, STACK_OF(X509)* chain)
		{
			throw_error_if_not(X509_STORE_CTX_init(raw(), _store.raw(), cert.raw(), chain) != 0);
		}
		inline void store_context::cleanup()
		{
			X509_STORE_CTX_cleanup(raw());
		}
		inline void store_context::set_external_data(int index, void* data)
		{
			throw_error_if(X509_STORE_CTX_set_ex_data(raw(), index, data) == 0);
		}
		inline void* store_context::get_external_data(int index)
		{
			// This call can fail but we cannot know for sure when it happens since NULL as a return value could also be a valid value...
			return X509_STORE_CTX_get_ex_data(raw(), index);
		}
		inline void store_context::set_trusted_certificates(STACK_OF(X509)* certs)
		{
			X509_STORE_CTX_trusted_stack(raw(), certs);
		}
		inline void store_context::set_certificate(certificate cert)
		{
			X509_STORE_CTX_set_cert(raw(), cert.raw());
		}
		inline void store_context::set_chain(STACK_OF(X509)* chain)
		{
			X509_STORE_CTX_set_chain(raw(), chain);
		}
		inline void store_context::set_crls(STACK_OF(X509_CRL)* crls)
		{
			X509_STORE_CTX_set0_crls(raw(), crls);
		}
		inline verify_param store_context::verification_parameters() const
		{
			return X509_STORE_CTX_get0_param(const_cast<pointer>(raw()));
		}
		inline void store_context::set_verification_parameters(verify_param vp)
		{
			X509_STORE_CTX_set0_param(raw(), vp.raw());
		}
		inline void store_context::set_default(const char* name)
		{
			throw_error_if_not(X509_STORE_CTX_set_default(raw(), name) != 0);
		}
		inline void store_context::set_default(const std::string& name)
		{
			throw_error_if_not(X509_STORE_CTX_set_default(raw(), name.c_str()) != 0);
		}
		inline long store_context::get_error() const
		{
			return X509_STORE_CTX_get_error(const_cast<pointer>(raw()));
		}
		inline const char* store_context::get_error_string() const
		{
			return X509_verify_cert_error_string(get_error());
		}
		inline int store_context::get_error_depth() const
		{
			return X509_STORE_CTX_get_error_depth(const_cast<pointer>(raw()));
		}
		inline certificate store_context::get_current_certificate() const
		{
			return X509_STORE_CTX_get_current_cert(const_cast<pointer>(raw()));
		}
		inline bool store_context::verify()
		{
			return (X509_verify_cert(raw()) > 0);
		}
		inline store_context::store_context(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const store_context& lhs, const store_context& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const store_context& lhs, const store_context& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_STORE_CONTEXT_HPP */


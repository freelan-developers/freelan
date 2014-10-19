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
 * \file certificate_request.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 certificate request class.
 */

#ifndef CRYPTOPLUS_X509_CERTIFICATE_REQUEST_HPP
#define CRYPTOPLUS_X509_CERTIFICATE_REQUEST_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "../bio/bio_ptr.hpp"
#include "../pkey/pkey.hpp"
#include "../asn1/integer.hpp"
#include "../asn1/utctime.hpp"
#include "../file.hpp"
#include "name.hpp"
#include "extension.hpp"

#include <openssl/x509.h>
#include <openssl/pem.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 certificate request.
		 *
		 * The certificate_request class represents a X509 certificate request.
		 *
		 * A certificate_request instance has the same semantic as a X509* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class certificate_request : public pointer_wrapper<X509_REQ>
		{
			public:

				/**
				 * \brief A PEM passphrase callback type.
				 */
				typedef int (*pem_passphrase_callback_type)(char*, int, int, void*);

				/**
				 * \brief Create a new certificate_request.
				 * \return The certificate_request.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static certificate_request create();

				/**
				 * \brief Take ownership of a specified X509 pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A certificate_request.
				 */
				static certificate_request take_ownership(pointer ptr);

				/**
				 * \brief Load a X509 certificate_request in DER format.
				 * \param bio The BIO.
				 * \return The certificate_request.
				 */
				static certificate_request from_der(bio::bio_ptr bio);

				/**
				 * \brief Load a X509 certificate_request from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_request.
				 *
				 * This function will also load a trusted certificate_request but without its 'trust' information.
				 */
				static certificate_request from_certificate_request(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 certificate_request in DER format.
				 * \param file The file.
				 * \return The certificate_request.
				 */
				static certificate_request from_der(file file);

				/**
				 * \brief Load a X509 certificate_request from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_request.
				 *
				 * This function will also load a trusted certificate_request but without its 'trust' information.
				 */
				static certificate_request from_certificate_request(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 certificate_request in DER format.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \return The certificate_request.
				 */
				static certificate_request from_der(const void* buf, size_t buf_len);

				/**
				 * \brief Load a X509 certificate_request in DER format.
				 * \param buf The buffer.
				 * \return The certificate_request.
				 */
				static certificate_request from_der(const buffer& buf);

				/**
				 * \brief Load a X509 certificate_request from a buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_request.
				 */
				static certificate_request from_certificate_request(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty X509 certificate_request.
				 */
				certificate_request();

				/**
				 * \brief Create a X509 certificate_request by *NOT* taking ownership of an existing X509* pointer.
				 * \param ptr The X509* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				certificate_request(pointer ptr);

				/**
				 * \brief Write the certificate_request in DER format to a BIO.
				 * \param bio The BIO.
				 */
				void write_der(bio::bio_ptr bio) const;

				/**
				 * \brief Write the certificate_request to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_request(bio::bio_ptr bio) const;

				/**
				 * \brief Write the certificate_request in DER format to a file.
				 * \param file The file.
				 */
				void write_der(file file) const;

				/**
				 * \brief Write the certificate_request to a file.
				 * \param file The file.
				 */
				void write_certificate_request(file file) const;

				/**
				 * \brief Write the certificate_request in DER format to a buffer.
				 * \param buf The buffer to write too. If NULL is specified, only the needed size is returned.
				 * \return The size written or to be written.
				 */
				size_t write_der(void* buf) const;

				/**
				 * \brief Write the certificate_request in DER format to a buffer.
				 * \return The buffer.
				 */
				buffer write_der() const;

				/**
				 * \brief Clone the certificate_request instance.
				 * \return The clone.
				 */
				certificate_request clone() const;

				/**
				 * \brief Print a X509 to a BIO.
				 * \param bio The BIO.
				 */
				void print(bio::bio_ptr bio) const;

				/**
				 * \brief Get the public key.
				 * \return The public key.
				 */
				pkey::pkey public_key() const;

				/**
				 * \brief Set the public key.
				 * \param pkey The public key.
				 */
				void set_public_key(pkey::pkey pkey) const;

				/**
				 * \brief Get the subject name.
				 * \return The subject name.
				 * \warning The returned name depends on the certificate_request instance and will be invalidated as soon as the underlying pointer is changed (or freed).
				 */
				name subject() const;

				/**
				 * \brief Set the subject name.
				 * \param aname The subject name that will be copied.
				 */
				void set_subject(name aname) const;

				/**
				 * \brief Get the certificate_request version.
				 * \return The version.
				 */
				long version() const;

				/**
				 * \brief Set the certificate_request version.
				 * \param version The version.
				 */
				void set_version(long version) const;

				/**
				 * \brief Verify the certificate_request against a specified public key.
				 * \param pkey The public pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_public_key(pkey::pkey pkey) const;

				/**
				 * \brief Sign the certificate_request.
				 * \param pkey The private key.
				 * \param algorithm The message digest to use.
				 */
				void sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm) const;

				/**
				 * \brief Verify the certificate_request against a specified private key.
				 * \param pkey The private pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_private_key(pkey::pkey pkey) const;

			private:

				certificate_request(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two certificate_request instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_request instances share the same underlying pointer.
		 */
		bool operator==(const certificate_request& lhs, const certificate_request& rhs);

		/**
		 * \brief Compare two certificate_request instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_request instances do not share the same underlying pointer.
		 */
		bool operator!=(const certificate_request& lhs, const certificate_request& rhs);

		inline certificate_request certificate_request::create()
		{
			pointer _ptr = X509_REQ_new();

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline certificate_request certificate_request::from_der(bio::bio_ptr bio)
		{
			return take_ownership(d2i_X509_REQ_bio(bio.raw(), NULL));
		}
		inline certificate_request certificate_request::from_certificate_request(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_X509_REQ(bio.raw(), NULL, callback, callback_arg));
		}
		inline certificate_request certificate_request::from_der(file _file)
		{
			return take_ownership(d2i_X509_REQ_fp(_file.raw(), NULL));
		}
		inline certificate_request certificate_request::from_certificate_request(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509_REQ(_file.raw(), NULL, callback, callback_arg));
		}
		inline certificate_request certificate_request::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509_REQ(NULL, &pbuf, static_cast<long>(buf_len)));
		}
		inline certificate_request certificate_request::from_der(const buffer& buf)
		{
			return from_der(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}
		inline certificate_request::certificate_request()
		{
		}
		inline certificate_request::certificate_request(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void certificate_request::write_der(bio::bio_ptr bio) const
		{
			throw_error_if_not(i2d_X509_REQ_bio(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate_request::write_certificate_request(bio::bio_ptr bio) const
		{
			throw_error_if_not(PEM_write_bio_X509_REQ(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate_request::write_der(file _file) const
		{
			throw_error_if_not(i2d_X509_REQ_fp(_file.raw(), ptr().get()) != 0);
		}
		inline void certificate_request::write_certificate_request(file _file) const
		{
			throw_error_if_not(PEM_write_X509_REQ(_file.raw(), ptr().get()) != 0);
		}
		inline size_t certificate_request::write_der(void* buf) const
		{
			unsigned char* out = static_cast<unsigned char*>(buf);
			unsigned char** pout = out != NULL ? &out : NULL;

			int result = i2d_X509_REQ(ptr().get(), pout);

			throw_error_if(result < 0);

			return result;
		}
		inline buffer certificate_request::write_der() const
		{
			buffer result(write_der(static_cast<void*>(NULL)));

			write_der(buffer_cast<uint8_t*>(result));

			return result;
		}
		inline certificate_request certificate_request::clone() const
		{
			return certificate_request(X509_REQ_dup(ptr().get()));
		}
		inline void certificate_request::print(bio::bio_ptr bio) const
		{
			throw_error_if_not(X509_REQ_print(bio.raw(), ptr().get()) != 0);
		}
		inline pkey::pkey certificate_request::public_key() const
		{
			return pkey::pkey(X509_REQ_get_pubkey(ptr().get()));
		}
		inline void certificate_request::set_public_key(pkey::pkey pkey) const
		{
			throw_error_if_not(X509_REQ_set_pubkey(ptr().get(), pkey.raw()) != 0);
		}
		inline name certificate_request::subject() const
		{
			return X509_REQ_get_subject_name(ptr().get());
		}
		inline void certificate_request::set_subject(name _name) const
		{
			throw_error_if_not(X509_REQ_set_subject_name(ptr().get(), _name.raw()) != 0);
		}
		inline long certificate_request::version() const
		{
			return X509_REQ_get_version(ptr().get());
		}
		inline void certificate_request::set_version(long _version) const
		{
			throw_error_if_not(X509_REQ_set_version(ptr().get(), _version) != 0);
		}
		inline bool certificate_request::verify_public_key(pkey::pkey pkey) const
		{
			return X509_REQ_verify(ptr().get(), pkey.raw()) == 1;
		}
		inline void certificate_request::sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm) const
		{
			throw_error_if_not(X509_REQ_sign(ptr().get(), pkey.raw(), algorithm.raw()) != 0);
		}
		inline bool certificate_request::verify_private_key(pkey::pkey pkey) const
		{
			return X509_REQ_check_private_key(ptr().get(), pkey.raw()) == 1;
		}
		inline certificate_request::certificate_request(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const certificate_request& lhs, const certificate_request& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const certificate_request& lhs, const certificate_request& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_CERTIFICATE_request_REQUEST_HPP */


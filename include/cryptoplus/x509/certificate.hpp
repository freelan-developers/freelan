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
 * \file certificate.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 certificate class.
 */

#ifndef CRYPTOPEN_X509_CERTIFICATE_HPP
#define CRYPTOPEN_X509_CERTIFICATE_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../pkey/pkey.hpp"
#include "../asn1/integer.hpp"
#include "../asn1/utctime.hpp"
#include "name.hpp"

#include <openssl/x509.h>
#include <openssl/pem.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 certificate.
		 *
		 * The certificate class represents a X509 certificate.
		 *
		 * A certificate instance has the same semantic as a X509* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class certificate : public pointer_wrapper<X509>
		{
			public:

				/**
				 * \brief A PEM passphrase callback type.
				 */
				typedef int (*pem_passphrase_callback_type)(char*, int, int, void*);

				/**
				 * \brief Create a new certificate.
				 * \return The certificate.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static certificate create();

				/**
				 * \brief Take ownership of a specified X509 pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A certificate.
				 */
				static certificate take_ownership(pointer ptr);

				/**
				 * \brief Load a X509 certificate in DER format.
				 * \param bio The BIO.
				 * \return The certificate.
				 */
				static certificate from_der(bio::bio_ptr bio);

				/**
				 * \brief Load a X509 certificate from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 *
				 * This function will also load a trusted certificate but without its 'trust' information.
				 */
				static certificate from_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 trusted certificate from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 */
				static certificate from_trusted_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 certificate in DER format.
				 * \param file The file.
				 * \return The certificate.
				 */
				static certificate from_der(FILE* file);

				/**
				 * \brief Load a X509 certificate from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 *
				 * This function will also load a trusted certificate but without its 'trust' information.
				 */
				static certificate from_certificate(FILE* file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 trusted certificate from a BIO.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 */
				static certificate from_trusted_certificate(FILE* file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 certificate in DER format.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \return The certificate.
				 */
				static certificate from_der(const void* buf, size_t buf_len);

				/**
				 * \brief Load a X509 certificate from a buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 */
				static certificate from_certificate(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 trusted certificate from a buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 */
				static certificate from_trusted_certificate(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty X509 certificate.
				 */
				certificate();

				/**
				 * \brief Create a X509 certificate by *NOT* taking ownership of an existing X509* pointer.
				 * \param x509 The X509* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				certificate(X509* x509);

				/**
				 * \brief Write the certificate in DER format to a BIO.
				 * \param bio The BIO.
				 */
				void write_der(bio::bio_ptr bio);

				/**
				 * \brief Write the certificate to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate(bio::bio_ptr bio);

				/**
				 * \brief Write the trusted certificate to a BIO.
				 * \param bio The BIO.
				 */
				void write_trusted_certificate(bio::bio_ptr bio);

				/**
				 * \brief Write the certificate in DER format to a file.
				 * \param file The file.
				 */
				void write_der(FILE* file);

				/**
				 * \brief Write the certificate to a file.
				 * \param file The file.
				 */
				void write_certificate(FILE* file);

				/**
				 * \brief Write the trusted certificate to a file.
				 * \param file The file.
				 */
				void write_trusted_certificate(FILE* file);

				/**
				 * \brief Write the certificate in DER format to a buffer.
				 * \param buf The buffer to write too. If NULL is specified, only the needed size is returned.
				 * \return The size written or to be written.
				 */
				size_t write_der(void* buf);

				/**
				 * \brief Write the certificate in DER format to a buffer.
				 * \return The buffer.
				 */
				std::vector<unsigned char> write_der();

				/**
				 * \brief Clone the certificate instance.
				 * \return The clone.
				 */
				certificate clone() const;

				/**
				 * \brief Print a X509 to a BIO.
				 * \param bio The BIO.
				 */
				void print(bio::bio_ptr bio);

				/**
				 * \brief Get the public key.
				 * \return The public key.
				 */
				pkey::pkey public_key();

				/**
				 * \brief Set the public key.
				 * \param pkey The public key.
				 */
				void set_public_key(pkey::pkey pkey);

				/**
				 * \brief Get the subject name.
				 * \return The subject name.
				 * \warning The returned name depends on the certificate instance and will be invalidated as soon as the underlying pointer is changed (or freed).
				 */
				name subject();

				/**
				 * \brief Set the subject name.
				 * \param aname The subject name that will be copied.
				 */
				void set_subject(name aname);

				/**
				 * \brief Get the issuer name.
				 * \return The issuer name.
				 * \warning The returned name depends on the certificate instance and will be invalidated as soon as the underlying pointer is changed (or freed).
				 */
				name issuer();

				/**
				 * \brief Set the issuer name.
				 * \param aname The issuer name that will be copied.
				 */
				void set_issuer(name aname);

				/**
				 * \brief Get the certificate version.
				 * \return The version.
				 */
				long version();

				/**
				 * \brief Set the certificate version.
				 * \param version The version.
				 */
				void set_version(long version);

				/**
				 * \brief Get the certificate serial number.
				 * \return The serial number.
				 */
				asn1::integer serial_number();

				/**
				 * \brief Set the certificate serial number.
				 * \param serial_number The serial number.
				 */
				void set_serial_number(asn1::integer serial_number);

				/**
				 * \brief Get the certificate not before date.
				 * \return The not before date.
				 */
				asn1::utctime not_before();

				/**
				 * \brief Set the certificate not before date.
				 * \param not_before The not before date.
				 */
				void set_not_before(asn1::utctime not_before);

				/**
				 * \brief Get the certificate not after date.
				 * \return The not after date.
				 */
				asn1::utctime not_after();

				/**
				 * \brief Set the certificate not after date.
				 * \param not_after The not after date.
				 */
				void set_not_after(asn1::utctime not_after);

				/**
				 * \brief Verify the certificate against a specified public key.
				 * \param pkey The public pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_public_key(pkey::pkey pkey);

				/**
				 * \brief Sign the certificate.
				 * \param pkey The private key.
				 * \param algorithm The message digest to use.
				 */
				void sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm);

				/**
				 * \brief Verify the certificate against a specified private key.
				 * \param pkey The private pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_private_key(pkey::pkey pkey);

			private:

				certificate(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two certificate instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate instances share the same underlying pointer.
		 */
		bool operator==(const certificate& lhs, const certificate& rhs);

		/**
		 * \brief Compare two certificate instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate instances do not share the same underlying pointer.
		 */
		bool operator!=(const certificate& lhs, const certificate& rhs);

		inline certificate certificate::create()
		{
			pointer _ptr = X509_new();

			error::throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline certificate certificate::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return certificate(_ptr, deleter);
		}
		inline certificate certificate::from_der(bio::bio_ptr bio)
		{
			return take_ownership(d2i_X509_bio(bio.raw(), NULL));
		}
		inline certificate certificate::from_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_X509(bio.raw(), NULL, callback, callback_arg));
		}
		inline certificate certificate::from_trusted_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_X509_AUX(bio.raw(), NULL, callback, callback_arg));
		}
		inline certificate certificate::from_der(FILE* file)
		{
			return take_ownership(d2i_X509_fp(file, NULL));
		}
		inline certificate certificate::from_certificate(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509(file, NULL, callback, callback_arg));
		}
		inline certificate certificate::from_trusted_certificate(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509_AUX(file, NULL, callback, callback_arg));
		}
		inline certificate certificate::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509(NULL, &pbuf, buf_len));
		}
		inline certificate::certificate()
		{
		}
		inline certificate::certificate(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void certificate::write_der(bio::bio_ptr bio)
		{
			error::throw_error_if_not(i2d_X509_bio(bio.raw(), ptr().get()));
		}
		inline void certificate::write_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509(bio.raw(), ptr().get()));
		}
		inline void certificate::write_trusted_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509_AUX(bio.raw(), ptr().get()));
		}
		inline void certificate::write_der(FILE* file)
		{
			error::throw_error_if_not(i2d_X509_fp(file, ptr().get()));
		}
		inline void certificate::write_certificate(FILE* file)
		{
			error::throw_error_if_not(PEM_write_X509(file, ptr().get()));
		}
		inline void certificate::write_trusted_certificate(FILE* file)
		{
			error::throw_error_if_not(PEM_write_X509_AUX(file, ptr().get()));
		}
		inline size_t certificate::write_der(void* buf)
		{
			unsigned char* out = static_cast<unsigned char*>(buf);

			int result = i2d_X509(ptr().get(), &out);

			error::throw_error_if(result < 0);

			return result;
		}
		inline std::vector<unsigned char> certificate::write_der()
		{
			std::vector<unsigned char> result(write_der(static_cast<void*>(NULL)));

			write_der(&result[0]);

			return result;
		}
		inline certificate certificate::clone() const
		{
			return certificate(X509_dup(ptr().get()));
		}
		inline void certificate::print(bio::bio_ptr bio)
		{
			error::throw_error_if_not(X509_print(bio.raw(), ptr().get()));
		}
		inline pkey::pkey certificate::public_key()
		{
			return pkey::pkey(X509_get_pubkey(ptr().get()));
		}
		inline void certificate::set_public_key(pkey::pkey pkey)
		{
			error::throw_error_if_not(X509_set_pubkey(ptr().get(), pkey.raw()));
		}
		inline name certificate::subject()
		{
			return X509_get_subject_name(ptr().get());
		}
		inline void certificate::set_subject(name _name)
		{
			error::throw_error_if_not(X509_set_subject_name(ptr().get(), _name.raw()));
		}
		inline name certificate::issuer()
		{
			return X509_get_issuer_name(ptr().get());
		}
		inline void certificate::set_issuer(name _name)
		{
			error::throw_error_if_not(X509_set_issuer_name(ptr().get(), _name.raw()));
		}
		inline long certificate::version()
		{
			return X509_get_version(ptr().get());
		}
		inline void certificate::set_version(long _version)
		{
			error::throw_error_if_not(X509_set_version(ptr().get(), _version));
		}
		inline asn1::integer certificate::serial_number()
		{
			return X509_get_serialNumber(ptr().get());
		}
		inline void certificate::set_serial_number(asn1::integer _serial_number)
		{
			error::throw_error_if_not(X509_set_serialNumber(ptr().get(), _serial_number.raw()));
		}
		inline asn1::utctime certificate::not_before()
		{
			return X509_get_notBefore(ptr().get());
		}
		inline void certificate::set_not_before(asn1::utctime _not_before)
		{
			error::throw_error_if_not(X509_set_notBefore(ptr().get(), _not_before.raw()));
		}
		inline asn1::utctime certificate::not_after()
		{
			return X509_get_notAfter(ptr().get());
		}
		inline void certificate::set_not_after(asn1::utctime _not_after)
		{
			error::throw_error_if_not(X509_set_notAfter(ptr().get(), _not_after.raw()));
		}
		inline bool certificate::verify_public_key(pkey::pkey pkey)
		{
			return X509_verify(ptr().get(), pkey.raw()) == 1;
		}
		inline void certificate::sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm)
		{
			error::throw_error_if_not(X509_sign(ptr().get(), pkey.raw(), algorithm.raw()));
		}
		inline bool certificate::verify_private_key(pkey::pkey pkey)
		{
			return X509_check_private_key(ptr().get(), pkey.raw()) == 1;
		}
		inline certificate::certificate(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const certificate& lhs, const certificate& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const certificate& lhs, const certificate& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_X509_CERTIFICATE_HPP */


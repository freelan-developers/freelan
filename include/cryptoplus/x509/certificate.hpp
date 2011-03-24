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

#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../pkey/pkey.hpp"
#include "name.hpp"

#include <openssl/x509.h>
#include <openssl/pem.h>

#include <boost/shared_ptr.hpp>

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
		 */
		class certificate
		{
			public:

				/**
				 * \brief A PEM passphrase callback type.
				 */
				typedef int (*pem_passphrase_callback_type)(char*, int, int, void*);

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
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				certificate();

				/**
				 * \brief Create a X509 certificate by taking ownership of an existing X509* pointer.
				 * \param x509 The X509* pointer. Cannot be NULL.
				 */
				explicit certificate(X509* x509);

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
				 * \brief Get the raw X509 pointer.
				 * \return The raw X509 pointer.
				 * \warning The instance has ownership of the return pointer. Calling X509_free() on the returned value will result in undefined behavior.
				 */
				const X509* raw() const;

				/**
				 * \brief Get the raw X509 pointer.
				 * \return The raw X509 pointer.
				 * \warning The instance has ownership of the return pointer. Calling X509_free() on the returned value will result in undefined behavior.
				 */
				X509* raw();

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
				ASN1_INTEGER* serial_number();

				/**
				 * \brief Set the certificate serial number.
				 * \param serial_number The serial number.
				 */
				void set_serial_number(ASN1_INTEGER* serial_number);

				/**
				 * \brief Verify the certificate against a specified public key.
				 * \param pkey The public pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_public_key(pkey::pkey pkey);

				/**
				 * \brief Verify the certificate against a specified private key.
				 * \param pkey The private pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_private_key(pkey::pkey pkey);

			private:

				explicit certificate(boost::shared_ptr<X509> x509);

				boost::shared_ptr<X509> m_x509;
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

		inline certificate certificate::from_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return certificate(boost::shared_ptr<X509>(PEM_read_bio_X509(bio.raw(), NULL, callback, callback_arg), X509_free));
		}
		inline certificate certificate::from_trusted_certificate(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return certificate(boost::shared_ptr<X509>(PEM_read_bio_X509_AUX(bio.raw(), NULL, callback, callback_arg), X509_free));
		}
		inline certificate certificate::from_certificate(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return certificate(boost::shared_ptr<X509>(PEM_read_X509(file, NULL, callback, callback_arg), X509_free));
		}
		inline certificate certificate::from_trusted_certificate(FILE* file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return certificate(boost::shared_ptr<X509>(PEM_read_X509_AUX(file, NULL, callback, callback_arg), X509_free));
		}
		inline certificate::certificate() : m_x509(X509_new(), X509_free)
		{
			error::throw_error_if_not(m_x509);
		}
		inline certificate::certificate(X509* x509) : m_x509(x509, X509_free)
		{
			if (!m_x509)
			{
				throw std::invalid_argument("certificate");
			}
		}
		inline void certificate::write_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509(bio.raw(), m_x509.get()));
		}
		inline void certificate::write_trusted_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509_AUX(bio.raw(), m_x509.get()));
		}
		inline void certificate::write_certificate(FILE* file)
		{
			error::throw_error_if_not(PEM_write_X509(file, m_x509.get()));
		}
		inline void certificate::write_trusted_certificate(FILE* file)
		{
			error::throw_error_if_not(PEM_write_X509_AUX(file, m_x509.get()));
		}
		inline const X509* certificate::raw() const
		{
			return m_x509.get();
		}
		inline X509* certificate::raw()
		{
			return m_x509.get();
		}
		inline certificate certificate::clone() const
		{
			return certificate(X509_dup(m_x509.get()));
		}
		inline void certificate::print(bio::bio_ptr bio)
		{
			error::throw_error_if_not(X509_print(bio.raw(), m_x509.get()));
		}
		inline pkey::pkey certificate::public_key()
		{
			return pkey::pkey(X509_get_pubkey(m_x509.get()));
		}
		inline name certificate::subject()
		{
			return name(boost::shared_ptr<X509_NAME>(X509_get_subject_name(m_x509.get()), name::null_deleter));
		}
		inline void certificate::set_subject(name aname)
		{
			error::throw_error_if_not(X509_set_subject_name(m_x509.get(), aname.raw()));
		}
		inline name certificate::issuer()
		{
			return name(boost::shared_ptr<X509_NAME>(X509_get_issuer_name(m_x509.get()), name::null_deleter));
		}
		inline void certificate::set_issuer(name aname)
		{
			error::throw_error_if_not(X509_set_issuer_name(m_x509.get(), aname.raw()));
		}
		inline long certificate::version()
		{
			return X509_get_version(m_x509.get());
		}
		inline void certificate::set_serial_number(ASN1_INTEGER* _serial_number)
		{
			error::throw_error_if_not(X509_set_serialNumber(m_x509.get(), _serial_number));
		}
		inline ASN1_INTEGER* certificate::serial_number()
		{
			return X509_get_serialNumber(m_x509.get());
		}
		inline void certificate::set_version(long _version)
		{
			error::throw_error_if_not(X509_set_version(m_x509.get(), _version));
		}
		inline bool certificate::verify_public_key(pkey::pkey pkey)
		{
			return X509_verify(m_x509.get(), pkey.raw()) == 1;
		}
		inline bool certificate::verify_private_key(pkey::pkey pkey)
		{
			return X509_check_private_key(m_x509.get(), pkey.raw()) == 1;
		}
		inline certificate::certificate(boost::shared_ptr<X509> x509) : m_x509(x509)
		{
			error::throw_error_if_not(m_x509);
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


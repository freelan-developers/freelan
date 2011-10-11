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

#ifndef CRYPTOPLUS_X509_CERTIFICATE_HPP
#define CRYPTOPLUS_X509_CERTIFICATE_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../pkey/pkey.hpp"
#include "../asn1/integer.hpp"
#include "../asn1/utctime.hpp"
#include "../file.hpp"
#include "name.hpp"
#include "extension.hpp"
#include "certificate_request.hpp"

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
				 * \brief The wrapped value type.
				 */
				typedef extension wrapped_value_type;

				/**
				 * \brief The wrapper pointer type.
				 */
				typedef wrapped_value_type* wrapped_pointer;

				/**
				 * \brief An iterator class.
				 */
				class iterator : public std::iterator<std::random_access_iterator_tag, wrapped_value_type>
				{
					public:

						/**
						 * \brief Create an empty iterator.
						 */
						iterator();

						/**
						 * \brief Dereference operator.
						 * \return The value.
						 */
						reference operator*();

						/**
						 * \brief Dereference operator.
						 * \return The value.
						 */
						pointer operator->();

						/**
						 * \brief Dereference operator.
						 * \param index The index to add or substract.
						 * \return An iterator.
						 */
						reference operator[](int index);

						/**
						 * \brief Increment the iterator.
						 * \return A reference to this.
						 */
						iterator& operator++();

						/**
						 * \brief Increment the iterator.
						 * \return The old value.
						 */
						iterator operator++(int);

						/**
						 * \brief Decrement the iterator.
						 * \return A reference to this.
						 */
						iterator& operator--();

						/**
						 * \brief Decrement the iterator.
						 * \return The old value.
						 */
						iterator operator--(int);

						/**
						 * \brief Increment the iterator.
						 * \param cnt The number to add to the iterator.
						 * \return A reference to this.
						 */
						iterator& operator+=(int cnt);

						/**
						 * \brief Decrement the iterator.
						 * \param cnt The number to substract from the iterator.
						 * \return A reference to this.
						 */
						iterator& operator-=(int cnt);

					private:

						iterator(certificate*, int);

						certificate* m_owner;
						int m_index;
						value_type m_cache;

						friend class certificate;
						friend bool operator==(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend bool operator!=(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend bool operator<(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend bool operator<=(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend bool operator>(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend bool operator>=(const certificate::iterator& lhs, const certificate::iterator& rhs);
						friend certificate::iterator operator+(const certificate::iterator& lhs, int rhs);
						friend certificate::iterator operator+(int lhs, const certificate::iterator& rhs);
						friend certificate::iterator operator-(const certificate::iterator& lhs, int rhs);
						friend certificate::iterator operator-(int lhs, const certificate::iterator& rhs);
						friend certificate::iterator::difference_type operator-(const certificate::iterator& lhs, const certificate::iterator& rhs);
				};

				/**
				* \brief Reverse iterator type.
				*/
				typedef std::reverse_iterator<iterator> reverse_iterator;

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
				static certificate from_der(file file);

				/**
				 * \brief Load a X509 certificate from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 *
				 * This function will also load a trusted certificate but without its 'trust' information.
				 */
				static certificate from_certificate(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509 trusted certificate from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate.
				 */
				static certificate from_trusted_certificate(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

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
				 * \param ptr The X509* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				certificate(pointer ptr);

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
				void write_der(file file);

				/**
				 * \brief Write the certificate to a file.
				 * \param file The file.
				 */
				void write_certificate(file file);

				/**
				 * \brief Write the trusted certificate to a file.
				 * \param file The file.
				 */
				void write_trusted_certificate(file file);

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
				 * \brief Get the count of entries.
				 * \return The count of entries.
				 */
				int count();

				/**
				 * \brief Get the entry at the specified position.
				 * \param index The index. Must be a valid index position or the behavior is undefined. See count().
				 * \return The name entry.
				 * \see count().
				 */
				wrapped_value_type operator[](int index);

				/**
				 * \brief Get the begin iterator.
				 * \return The begin iterator.
				 */
				iterator begin();

				/**
				 * \brief Get the end iterator.
				 * \return The end iterator.
				 */
				iterator end();

				/**
				 * \brief Get the reverse begin iterator.
				 * \return The reverse begin iterator.
				 */
				reverse_iterator rbegin();

				/**
				 * \brief Get the reverse end iterator.
				 * \return The reverse end iterator.
				 */
				reverse_iterator rend();

				/**
				 * \brief Erase the given entry.
				 * \param it An iterator to the entry to erase.
				 * \return The next iterator.
				 */
				iterator erase(iterator it);

				/**
				 * \brief Erase the given entries.
				 * \param first The first iterator.
				 * \param last The last iterator.
				 * \return last.
				 */
				iterator erase(iterator first, iterator last);

				/**
				 * \brief Find an extension by its NID.
				 * \param nid The nid.
				 * \return An iterator to the first extension that matches, or end() if none is found.
				 */
				iterator find(int nid);

				/**
				 * \brief Find an extension by its NID.
				 * \param nid The nid.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an extension that matches, or end() if none is found.
				 */
				iterator find(int nid, iterator lastpos);

				/**
				 * \brief Find an extension by its ASN1 object.
				 * \param object The ASN1 object.
				 * \return An iterator to the first extension that matches, or end() if none is found.
				 */
				iterator find(asn1::object object);

				/**
				 * \brief Find an extension by its ASN1 object.
				 * \param object The ASN1 object.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an extension that matches, or end() if none is found.
				 */
				iterator find(asn1::object object, iterator lastpos);

				/**
				 * \brief Find an extension by its critical flag.
				 * \param critical The critical flag.
				 * \return An iterator to the first extension that matches, or end() if none is found.
				 */
				iterator find_by_critical(bool critical);

				/**
				 * \brief Find an extension by its critical flag.
				 * \param critical The critical flag.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an extension that matches, or end() if none is found.
				 */
				iterator find_by_critical(bool critical, iterator lastpos);

				/**
				 * \brief Clear all extensions.
				 */
				void clear();

				/**
				 * \brief Push a copy of the specified extension at the end of the extension table.
				 * \param ext The extension
				 */
				void push_back(wrapped_value_type ext);

				/**
				 * \brief Insert a copy of the specified extension in the extension table.
				 * \param position The position to insert the extension at.
				 * \param ext The extension.
				 * \return An iterator to the extension that was added.
				 */
				iterator insert(iterator position, wrapped_value_type ext);

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

				/**
				 * \brief Generate a X509 certificate request from this certificate.
				 * \param pkey The private key to use to sign the request.
				 * \param algorithm The message digest algorithm to use to sign the request.
				 * \return The certificate request.
				 */
				certificate_request to_certificate_request(pkey::pkey pkey, hash::message_digest_algorithm algorithm);

			private:

				certificate(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate::iterator instances point to the same element.
		 */
		bool operator==(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate::iterator instances do not point to the same element.
		 */
		bool operator!=(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than rhs.
		 */
		bool operator<(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than or equal to rhs.
		 */
		bool operator<=(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than rhs.
		 */
		bool operator>(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Compare two certificate::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than or equal to rhs.
		 */
		bool operator>=(const certificate::iterator& lhs, const certificate::iterator& rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate::iterator operator+(const certificate::iterator& lhs, int rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate::iterator operator+(int lhs, const certificate::iterator& rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate::iterator operator-(const certificate::iterator& lhs, int rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate::iterator operator-(int lhs, const certificate::iterator& rhs);

		/**
		 * \brief Substract a iterator from another iterator and gets the index distance.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The distance.
		 */
		certificate::iterator::difference_type operator-(const certificate::iterator& lhs, const certificate::iterator& rhs);

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

		inline certificate::iterator::iterator() : m_owner(NULL), m_index(0)
		{
		}
		inline certificate::iterator::reference certificate::iterator::operator*()
		{
			return (m_cache = (*m_owner)[m_index]);
		}
		inline certificate::iterator::pointer certificate::iterator::operator->()
		{
			return &operator*();
		}
		inline certificate::iterator::reference certificate::iterator::operator[](int index)
		{
			return *iterator(m_owner, m_index + index);
		}
		inline certificate::iterator& certificate::iterator::operator++()
		{
			++m_index;

			return *this;
		}
		inline certificate::iterator certificate::iterator::operator++(int)
		{
			iterator old = *this;

			++m_index;

			return old;
		}
		inline certificate::iterator& certificate::iterator::operator--()
		{
			--m_index;

			return *this;
		}
		inline certificate::iterator certificate::iterator::operator--(int)
		{
			iterator old = *this;

			--m_index;

			return old;
		}
		inline certificate::iterator& certificate::iterator::operator+=(int cnt)
		{
			m_index += cnt;

			return *this;
		}
		inline certificate::iterator& certificate::iterator::operator-=(int cnt)
		{
			m_index -= cnt;

			return *this;
		}
		inline certificate::iterator::iterator(certificate* _certificate, int index) : m_owner(_certificate), m_index(index)
		{
		}
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
		inline certificate certificate::from_der(file _file)
		{
			return take_ownership(d2i_X509_fp(_file.raw(), NULL));
		}
		inline certificate certificate::from_certificate(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509(_file.raw(), NULL, callback, callback_arg));
		}
		inline certificate certificate::from_trusted_certificate(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509_AUX(_file.raw(), NULL, callback, callback_arg));
		}
		inline certificate certificate::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509(NULL, &pbuf, static_cast<long>(buf_len)));
		}
		inline certificate::certificate()
		{
		}
		inline certificate::certificate(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void certificate::write_der(bio::bio_ptr bio)
		{
			error::throw_error_if_not(i2d_X509_bio(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate::write_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate::write_trusted_certificate(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509_AUX(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate::write_der(file _file)
		{
			error::throw_error_if_not(i2d_X509_fp(_file.raw(), ptr().get()) != 0);
		}
		inline void certificate::write_certificate(file _file)
		{
			error::throw_error_if_not(PEM_write_X509(_file.raw(), ptr().get()) != 0);
		}
		inline void certificate::write_trusted_certificate(file _file)
		{
			error::throw_error_if_not(PEM_write_X509_AUX(_file.raw(), ptr().get()) != 0);
		}
		inline size_t certificate::write_der(void* buf)
		{
			unsigned char* out = static_cast<unsigned char*>(buf);
			unsigned char** pout = out != NULL ? &out : NULL;

			int result = i2d_X509(ptr().get(), pout);

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
			error::throw_error_if_not(X509_print(bio.raw(), ptr().get()) != 0);
		}
		inline int certificate::count()
		{
			return X509_get_ext_count(ptr().get());
		}
		inline certificate::wrapped_value_type certificate::operator[](int index)
		{
			return wrapped_value_type(X509_get_ext(ptr().get(), index));
		}
		inline certificate::iterator certificate::begin()
		{
			return iterator(this, 0);
		}
		inline certificate::iterator certificate::end()
		{
			return iterator(this, count());
		}
		inline certificate::reverse_iterator certificate::rbegin()
		{
			return reverse_iterator(end());
		}
		inline certificate::reverse_iterator certificate::rend()
		{
			return reverse_iterator(begin());
		}
		inline certificate::iterator certificate::erase(iterator it)
		{
			wrapped_value_type::take_ownership(X509_delete_ext(it.m_owner->ptr().get(), it.m_index));

			return it;
		}
		inline certificate::iterator certificate::erase(iterator first, iterator last)
		{
			while (first != last)
				first = erase(first);

			return first;
		}
		inline certificate::iterator certificate::find(int nid)
		{
			int index = X509_get_ext_by_NID(ptr().get(), nid, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate::iterator certificate::find(int nid, iterator lastpos)
		{
			int index = X509_get_ext_by_NID(ptr().get(), nid, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate::iterator certificate::find(asn1::object object)
		{
			int index = X509_get_ext_by_OBJ(ptr().get(), object.raw(), -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate::iterator certificate::find(asn1::object object, iterator lastpos)
		{
			int index = X509_get_ext_by_OBJ(ptr().get(), object.raw(), lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate::iterator certificate::find_by_critical(bool critical)
		{
			int index = X509_get_ext_by_critical(ptr().get(), critical ? 1 : 0, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate::iterator certificate::find_by_critical(bool critical, iterator lastpos)
		{
			int index = X509_get_ext_by_critical(ptr().get(), critical ? 1 : 0, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline void certificate::clear()
		{
			erase(begin(), end());
		}
		inline void certificate::push_back(wrapped_value_type ext)
		{
			error::throw_error_if_not(X509_add_ext(ptr().get(), ext.raw(), -1) != 0);
		}
		inline certificate::iterator certificate::insert(iterator position, wrapped_value_type ext)
		{
			assert(position.m_owner == this);

			error::throw_error_if_not(X509_add_ext(ptr().get(), ext.raw(), position.m_index) != 0);

			return position;
		}
		inline pkey::pkey certificate::public_key()
		{
			return pkey::pkey::take_ownership(X509_get_pubkey(ptr().get()));
		}
		inline void certificate::set_public_key(pkey::pkey pkey)
		{
			error::throw_error_if_not(X509_set_pubkey(ptr().get(), pkey.raw()) != 0);
		}
		inline name certificate::subject()
		{
			return X509_get_subject_name(ptr().get());
		}
		inline void certificate::set_subject(name _name)
		{
			error::throw_error_if_not(X509_set_subject_name(ptr().get(), _name.raw()) != 0);
		}
		inline name certificate::issuer()
		{
			return X509_get_issuer_name(ptr().get());
		}
		inline void certificate::set_issuer(name _name)
		{
			error::throw_error_if_not(X509_set_issuer_name(ptr().get(), _name.raw()) != 0);
		}
		inline long certificate::version()
		{
			return X509_get_version(ptr().get());
		}
		inline void certificate::set_version(long _version)
		{
			error::throw_error_if_not(X509_set_version(ptr().get(), _version) != 0);
		}
		inline asn1::integer certificate::serial_number()
		{
			return X509_get_serialNumber(ptr().get());
		}
		inline void certificate::set_serial_number(asn1::integer _serial_number)
		{
			error::throw_error_if_not(X509_set_serialNumber(ptr().get(), _serial_number.raw()) != 0);
		}
		inline asn1::utctime certificate::not_before()
		{
			return X509_get_notBefore(ptr().get());
		}
		inline void certificate::set_not_before(asn1::utctime _not_before)
		{
			error::throw_error_if_not(X509_set_notBefore(ptr().get(), _not_before.raw()) != 0);
		}
		inline asn1::utctime certificate::not_after()
		{
			return X509_get_notAfter(ptr().get());
		}
		inline void certificate::set_not_after(asn1::utctime _not_after)
		{
			error::throw_error_if_not(X509_set_notAfter(ptr().get(), _not_after.raw()) != 0);
		}
		inline bool certificate::verify_public_key(pkey::pkey pkey)
		{
			return X509_verify(ptr().get(), pkey.raw()) == 1;
		}
		inline void certificate::sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm)
		{
			error::throw_error_if_not(X509_sign(ptr().get(), pkey.raw(), algorithm.raw()) != 0);
		}
		inline bool certificate::verify_private_key(pkey::pkey pkey)
		{
			return X509_check_private_key(ptr().get(), pkey.raw()) == 1;
		}
		inline certificate_request certificate::to_certificate_request(pkey::pkey pkey, hash::message_digest_algorithm algorithm)
		{
			return certificate_request::take_ownership(X509_to_X509_REQ(ptr().get(), pkey.raw(), algorithm.raw()));
		}
		inline certificate::certificate(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index == rhs.m_index);
		}
		inline bool operator!=(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index != rhs.m_index);
		}
		inline bool operator<(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index < rhs.m_index);
		}
		inline bool operator<=(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index <= rhs.m_index);
		}
		inline bool operator>(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index > rhs.m_index);
		}
		inline bool operator>=(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index >= rhs.m_index);
		}
		inline certificate::iterator operator+(const certificate::iterator& lhs, int rhs)
		{
			return certificate::iterator(lhs.m_owner, lhs.m_index + rhs);
		}
		inline certificate::iterator operator+(int lhs, const certificate::iterator& rhs)
		{
			return certificate::iterator(rhs.m_owner, rhs.m_index + lhs);
		}
		inline certificate::iterator operator-(const certificate::iterator& lhs, int rhs)
		{
			return certificate::iterator(lhs.m_owner, lhs.m_index - rhs);
		}
		inline certificate::iterator operator-(int lhs, const certificate::iterator& rhs)
		{
			return certificate::iterator(rhs.m_owner, rhs.m_index - lhs);
		}
		inline certificate::iterator::difference_type operator-(const certificate::iterator& lhs, const certificate::iterator& rhs)
		{
			return lhs.m_index - rhs.m_index;
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

#endif /* CRYPTOPLUS_X509_CERTIFICATE_HPP */


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
 * \file certificate_revocation_list.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 certificate revocation list class.
 */

#ifndef CRYPTOPLUS_X509_CRL_CERTIFICATE_REVOCATION_LIST_HPP
#define CRYPTOPLUS_X509_CRL_CERTIFICATE_REVOCATION_LIST_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../pkey/pkey.hpp"
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
		 * \brief A X509 certificate revocation list.
		 *
		 * The certificate_revocation_list class represents a X509 certificate revocation list.
		 *
		 * A certificate_revocation_list instance has the same semantic as a X509_CRL* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class certificate_revocation_list : public pointer_wrapper<X509_CRL>
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

						iterator(certificate_revocation_list*, int);

						certificate_revocation_list* m_owner;
						int m_index;
						value_type m_cache;

						friend class certificate_revocation_list;
						friend bool operator==(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend bool operator!=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend bool operator<(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend bool operator<=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend bool operator>(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend bool operator>=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
						friend certificate_revocation_list::iterator operator+(const certificate_revocation_list::iterator& lhs, int rhs);
						friend certificate_revocation_list::iterator operator+(int lhs, const certificate_revocation_list::iterator& rhs);
						friend certificate_revocation_list::iterator operator-(const certificate_revocation_list::iterator& lhs, int rhs);
						friend certificate_revocation_list::iterator operator-(int lhs, const certificate_revocation_list::iterator& rhs);
						friend certificate_revocation_list::iterator::difference_type operator-(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);
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
				 * \brief Create a new certificate_revocation_list.
				 * \return The certificate_revocation_list.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static certificate_revocation_list create();

				/**
				 * \brief Take ownership of a specified X509_CRL pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A certificate_revocation_list.
				 */
				static certificate_revocation_list take_ownership(pointer ptr);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list in DER format.
				 * \param bio The BIO.
				 * \return The certificate_revocation_list.
				 */
				static certificate_revocation_list from_der(bio::bio_ptr bio);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list from a BIO.
				 * \param bio The BIO.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_revocation_list.
				 *
				 * This function will also load a trusted certificate_revocation_list but without its 'trust' information.
				 */
				static certificate_revocation_list from_certificate_revocation_list(bio::bio_ptr bio, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list in DER format.
				 * \param file The file.
				 * \return The certificate_revocation_list.
				 */
				static certificate_revocation_list from_der(file file);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list from a file.
				 * \param file The file.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_revocation_list.
				 *
				 * This function will also load a trusted certificate_revocation_list but without its 'trust' information.
				 */
				static certificate_revocation_list from_certificate_revocation_list(file file, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list in DER format.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \return The certificate_revocation_list.
				 */
				static certificate_revocation_list from_der(const void* buf, size_t buf_len);

				/**
				 * \brief Load a X509_CRL certificate_revocation_list from a buffer.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \param callback A callback that will get called whenever a passphrase is needed. Can be NULL, in such case no passphrase is used.
				 * \param callback_arg An argument that will be passed to callback, if needed.
				 * \return The certificate_revocation_list.
				 */
				static certificate_revocation_list from_certificate_revocation_list(const void* buf, size_t buf_len, pem_passphrase_callback_type callback = NULL, void* callback_arg = NULL);

				/**
				 * \brief Create a new empty X509_CRL certificate_revocation_list.
				 */
				certificate_revocation_list();

				/**
				 * \brief Create a X509_CRL certificate_revocation_list by *NOT* taking ownership of an existing X509_CRL* pointer.
				 * \param ptr The X509_CRL* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				certificate_revocation_list(pointer ptr);

				/**
				 * \brief Write the certificate_revocation_list in DER format to a BIO.
				 * \param bio The BIO.
				 */
				void write_der(bio::bio_ptr bio);

				/**
				 * \brief Write the certificate_revocation_list to a BIO.
				 * \param bio The BIO.
				 */
				void write_certificate_revocation_list(bio::bio_ptr bio);

				/**
				 * \brief Write the certificate_revocation_list in DER format to a file.
				 * \param file The file.
				 */
				void write_der(file file);

				/**
				 * \brief Write the certificate_revocation_list to a file.
				 * \param file The file.
				 */
				void write_certificate_revocation_list(file file);

				/**
				 * \brief Write the certificate_revocation_list in DER format to a buffer.
				 * \param buf The buffer to write too. If NULL is specified, only the needed size is returned.
				 * \return The size written or to be written.
				 */
				size_t write_der(void* buf);

				/**
				 * \brief Write the certificate_revocation_list in DER format to a buffer.
				 * \return The buffer.
				 */
				std::string write_der();

				/**
				 * \brief Clone the certificate_revocation_list instance.
				 * \return The clone.
				 */
				certificate_revocation_list clone() const;

				/**
				 * \brief Print a X509_CRL to a BIO.
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
				 * \brief Set the issuer name.
				 * \param aname The issuer name that will be copied.
				 */
				void set_issuer(name aname);

				/**
				 * \brief Get the certificate_revocation_list version.
				 * \return The version.
				 */
				long version();

				/**
				 * \brief Set the certificate_revocation_list version.
				 * \param version The version.
				 */
				void set_version(long version);

				/**
				 * \brief Verify the certificate_revocation_list against a specified public key.
				 * \param pkey The public pkey.
				 * \return true if the verification succeeds.
				 */
				bool verify_public_key(pkey::pkey pkey);

				/**
				 * \brief Sign the certificate_revocation_list.
				 * \param pkey The private key.
				 * \param algorithm The message digest to use.
				 */
				void sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm);

			private:

				certificate_revocation_list(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_revocation_list::iterator instances point to the same element.
		 */
		bool operator==(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_revocation_list::iterator instances do not point to the same element.
		 */
		bool operator!=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than rhs.
		 */
		bool operator<(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than or equal to rhs.
		 */
		bool operator<=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than rhs.
		 */
		bool operator>(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than or equal to rhs.
		 */
		bool operator>=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate_revocation_list::iterator operator+(const certificate_revocation_list::iterator& lhs, int rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate_revocation_list::iterator operator+(int lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate_revocation_list::iterator operator-(const certificate_revocation_list::iterator& lhs, int rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		certificate_revocation_list::iterator operator-(int lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Substract a iterator from another iterator and gets the index distance.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The distance.
		 */
		certificate_revocation_list::iterator::difference_type operator-(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs);

		/**
		 * \brief Compare two certificate_revocation_list instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_revocation_list instances share the same underlying pointer.
		 */
		bool operator==(const certificate_revocation_list& lhs, const certificate_revocation_list& rhs);

		/**
		 * \brief Compare two certificate_revocation_list instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two certificate_revocation_list instances do not share the same underlying pointer.
		 */
		bool operator!=(const certificate_revocation_list& lhs, const certificate_revocation_list& rhs);

		inline certificate_revocation_list::iterator::iterator() : m_owner(NULL), m_index(0)
		{
		}
		inline certificate_revocation_list::iterator::reference certificate_revocation_list::iterator::operator*()
		{
			return (m_cache = (*m_owner)[m_index]);
		}
		inline certificate_revocation_list::iterator::pointer certificate_revocation_list::iterator::operator->()
		{
			return &operator*();
		}
		inline certificate_revocation_list::iterator::reference certificate_revocation_list::iterator::operator[](int index)
		{
			return *iterator(m_owner, m_index + index);
		}
		inline certificate_revocation_list::iterator& certificate_revocation_list::iterator::operator++()
		{
			++m_index;

			return *this;
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::iterator::operator++(int)
		{
			iterator old = *this;

			++m_index;

			return old;
		}
		inline certificate_revocation_list::iterator& certificate_revocation_list::iterator::operator--()
		{
			--m_index;

			return *this;
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::iterator::operator--(int)
		{
			iterator old = *this;

			--m_index;

			return old;
		}
		inline certificate_revocation_list::iterator& certificate_revocation_list::iterator::operator+=(int cnt)
		{
			m_index += cnt;

			return *this;
		}
		inline certificate_revocation_list::iterator& certificate_revocation_list::iterator::operator-=(int cnt)
		{
			m_index -= cnt;

			return *this;
		}
		inline certificate_revocation_list::iterator::iterator(certificate_revocation_list* _certificate_revocation_list, int index) : m_owner(_certificate_revocation_list), m_index(index)
		{
		}
		inline certificate_revocation_list certificate_revocation_list::create()
		{
			pointer _ptr = X509_CRL_new();

			error::throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline certificate_revocation_list certificate_revocation_list::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return certificate_revocation_list(_ptr, deleter);
		}
		inline certificate_revocation_list certificate_revocation_list::from_der(bio::bio_ptr bio)
		{
			return take_ownership(d2i_X509_CRL_bio(bio.raw(), NULL));
		}
		inline certificate_revocation_list certificate_revocation_list::from_certificate_revocation_list(bio::bio_ptr bio, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_bio_X509_CRL(bio.raw(), NULL, callback, callback_arg));
		}
		inline certificate_revocation_list certificate_revocation_list::from_der(file _file)
		{
			return take_ownership(d2i_X509_CRL_fp(_file.raw(), NULL));
		}
		inline certificate_revocation_list certificate_revocation_list::from_certificate_revocation_list(file _file, pem_passphrase_callback_type callback, void* callback_arg)
		{
			return take_ownership(PEM_read_X509_CRL(_file.raw(), NULL, callback, callback_arg));
		}
		inline certificate_revocation_list certificate_revocation_list::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509_CRL(NULL, &pbuf, static_cast<long>(buf_len)));
		}
		inline certificate_revocation_list::certificate_revocation_list()
		{
		}
		inline certificate_revocation_list::certificate_revocation_list(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void certificate_revocation_list::write_der(bio::bio_ptr bio)
		{
			error::throw_error_if_not(i2d_X509_CRL_bio(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate_revocation_list::write_certificate_revocation_list(bio::bio_ptr bio)
		{
			error::throw_error_if_not(PEM_write_bio_X509_CRL(bio.raw(), ptr().get()) != 0);
		}
		inline void certificate_revocation_list::write_der(file _file)
		{
			error::throw_error_if_not(i2d_X509_CRL_fp(_file.raw(), ptr().get()) != 0);
		}
		inline void certificate_revocation_list::write_certificate_revocation_list(file _file)
		{
			error::throw_error_if_not(PEM_write_X509_CRL(_file.raw(), ptr().get()) != 0);
		}
		inline size_t certificate_revocation_list::write_der(void* buf)
		{
			unsigned char* out = static_cast<unsigned char*>(buf);
			unsigned char** pout = out != NULL ? &out : NULL;

			int result = i2d_X509_CRL(ptr().get(), pout);

			error::throw_error_if(result < 0);

			return result;
		}
		inline std::string certificate_revocation_list::write_der()
		{
			std::string result(write_der(static_cast<void*>(NULL)), char());

			write_der(&result[0]);

			return result;
		}
		inline certificate_revocation_list certificate_revocation_list::clone() const
		{
			return certificate_revocation_list(X509_CRL_dup(ptr().get()));
		}
		inline void certificate_revocation_list::print(bio::bio_ptr bio)
		{
			error::throw_error_if_not(X509_CRL_print(bio.raw(), ptr().get()) != 0);
		}
		inline int certificate_revocation_list::count()
		{
			return X509_CRL_get_ext_count(ptr().get());
		}
		inline certificate_revocation_list::wrapped_value_type certificate_revocation_list::operator[](int index)
		{
			return wrapped_value_type(X509_CRL_get_ext(ptr().get(), index));
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::begin()
		{
			return iterator(this, 0);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::end()
		{
			return iterator(this, count());
		}
		inline certificate_revocation_list::reverse_iterator certificate_revocation_list::rbegin()
		{
			return reverse_iterator(end());
		}
		inline certificate_revocation_list::reverse_iterator certificate_revocation_list::rend()
		{
			return reverse_iterator(begin());
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::erase(iterator it)
		{
			wrapped_value_type::take_ownership(X509_CRL_delete_ext(it.m_owner->ptr().get(), it.m_index));

			return it;
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::erase(iterator first, iterator last)
		{
			while (first != last)
				first = erase(first);

			return first;
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find(int nid)
		{
			int index = X509_CRL_get_ext_by_NID(ptr().get(), nid, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find(int nid, iterator lastpos)
		{
			int index = X509_CRL_get_ext_by_NID(ptr().get(), nid, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find(asn1::object object)
		{
			int index = X509_CRL_get_ext_by_OBJ(ptr().get(), object.raw(), -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find(asn1::object object, iterator lastpos)
		{
			int index = X509_CRL_get_ext_by_OBJ(ptr().get(), object.raw(), lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find_by_critical(bool critical)
		{
			int index = X509_CRL_get_ext_by_critical(ptr().get(), critical ? 1 : 0, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::find_by_critical(bool critical, iterator lastpos)
		{
			int index = X509_CRL_get_ext_by_critical(ptr().get(), critical ? 1 : 0, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline void certificate_revocation_list::clear()
		{
			erase(begin(), end());
		}
		inline void certificate_revocation_list::push_back(wrapped_value_type ext)
		{
			error::throw_error_if_not(X509_CRL_add_ext(ptr().get(), ext.raw(), -1) != 0);
		}
		inline certificate_revocation_list::iterator certificate_revocation_list::insert(iterator position, wrapped_value_type ext)
		{
			assert(position.m_owner == this);

			error::throw_error_if_not(X509_CRL_add_ext(ptr().get(), ext.raw(), position.m_index) != 0);

			return position;
		}
		inline void certificate_revocation_list::set_issuer(name _name)
		{
			error::throw_error_if_not(X509_CRL_set_issuer_name(ptr().get(), _name.raw()) != 0);
		}
		inline long certificate_revocation_list::version()
		{
			return X509_CRL_get_version(ptr().get());
		}
		inline void certificate_revocation_list::set_version(long _version)
		{
			error::throw_error_if_not(X509_CRL_set_version(ptr().get(), _version) != 0);
		}
		inline bool certificate_revocation_list::verify_public_key(pkey::pkey pkey)
		{
			return X509_CRL_verify(ptr().get(), pkey.raw()) == 1;
		}
		inline void certificate_revocation_list::sign(pkey::pkey pkey, hash::message_digest_algorithm algorithm)
		{
			error::throw_error_if_not(X509_CRL_sign(ptr().get(), pkey.raw(), algorithm.raw()) != 0);
		}
		inline certificate_revocation_list::certificate_revocation_list(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index == rhs.m_index);
		}
		inline bool operator!=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index != rhs.m_index);
		}
		inline bool operator<(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index < rhs.m_index);
		}
		inline bool operator<=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index <= rhs.m_index);
		}
		inline bool operator>(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index > rhs.m_index);
		}
		inline bool operator>=(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index >= rhs.m_index);
		}
		inline certificate_revocation_list::iterator operator+(const certificate_revocation_list::iterator& lhs, int rhs)
		{
			return certificate_revocation_list::iterator(lhs.m_owner, lhs.m_index + rhs);
		}
		inline certificate_revocation_list::iterator operator+(int lhs, const certificate_revocation_list::iterator& rhs)
		{
			return certificate_revocation_list::iterator(rhs.m_owner, rhs.m_index + lhs);
		}
		inline certificate_revocation_list::iterator operator-(const certificate_revocation_list::iterator& lhs, int rhs)
		{
			return certificate_revocation_list::iterator(lhs.m_owner, lhs.m_index - rhs);
		}
		inline certificate_revocation_list::iterator operator-(int lhs, const certificate_revocation_list::iterator& rhs)
		{
			return certificate_revocation_list::iterator(rhs.m_owner, rhs.m_index - lhs);
		}
		inline certificate_revocation_list::iterator::difference_type operator-(const certificate_revocation_list::iterator& lhs, const certificate_revocation_list::iterator& rhs)
		{
			return lhs.m_index - rhs.m_index;
		}
		inline bool operator==(const certificate_revocation_list& lhs, const certificate_revocation_list& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const certificate_revocation_list& lhs, const certificate_revocation_list& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_CRL_CERTIFICATE_REVOCATION_LIST_HPP */


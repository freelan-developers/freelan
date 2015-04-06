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
 * \file name.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 name class.
 */

#ifndef CRYPTOPLUS_X509_NAME_HPP
#define CRYPTOPLUS_X509_NAME_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "../bio/bio_ptr.hpp"
#include "name_entry.hpp"

// Windows defines a X509_NAME structure as well...
#ifdef X509_NAME
#undef X509_NAME
#endif

#include <openssl/x509.h>

#include <cstring>
#include <string>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 name.
		 *
		 * The name class represents a X509 name.
		 *
		 * A name instance has the same semantic as a X509_NAME* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class name : public pointer_wrapper<X509_NAME>
		{
			public:

				/**
				 * \brief The wrapped value type.
				 */
				typedef name_entry wrapped_value_type;

				/**
				 * \brief The wrapper pointer type.
				 */
				typedef wrapped_value_type* wrapped_pointer;

				/**
				 * \brief An iterator class.
				 */
				template <typename IteratorValueType>
				class base_iterator : public std::iterator<std::random_access_iterator_tag, typename IteratorValueType::wrapped_value_type>
				{
					public:

						/**
						 * \brief Create an empty iterator.
						 */
						base_iterator();

						/**
						 * \brief Dereference operator.
						 * \return The value.
						 */
						typename base_iterator::reference operator*() const;

						/**
						 * \brief Dereference operator.
						 * \return The value.
						 */
						typename base_iterator::pointer operator->() const;

						/**
						 * \brief Dereference operator.
						 * \param index The index to add or substract.
						 * \return An iterator.
						 */
						typename base_iterator::reference operator[](int index) const;

						/**
						 * \brief Increment the iterator.
						 * \return A reference to this.
						 */
						base_iterator& operator++();

						/**
						 * \brief Increment the iterator.
						 * \return The old value.
						 */
						base_iterator operator++(int);

						/**
						 * \brief Decrement the iterator.
						 * \return A reference to this.
						 */
						base_iterator& operator--();

						/**
						 * \brief Decrement the iterator.
						 * \return The old value.
						 */
						base_iterator operator--(int);

						/**
						 * \brief Increment the iterator.
						 * \param cnt The number to add to the iterator.
						 * \return A reference to this.
						 */
						base_iterator& operator+=(int cnt);

						/**
						 * \brief Decrement the iterator.
						 * \param cnt The number to substract from the iterator.
						 * \return A reference to this.
						 */
						base_iterator& operator-=(int cnt);

					private:

						base_iterator(IteratorValueType*, int);

						IteratorValueType* m_owner;
						int m_index;
						mutable typename IteratorValueType::wrapped_value_type m_cache;

						friend class name;
						template <typename _IteratorValueType>
						friend bool operator==(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend bool operator!=(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend bool operator<(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend bool operator<=(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend bool operator>(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend bool operator>=(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend name::base_iterator<_IteratorValueType> operator+(const name::base_iterator<_IteratorValueType>& lhs, int rhs);
						template <typename _IteratorValueType>
						friend name::base_iterator<_IteratorValueType> operator+(int lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend name::base_iterator<_IteratorValueType> operator-(const name::base_iterator<_IteratorValueType>& lhs, int rhs);
						template <typename _IteratorValueType>
						friend name::base_iterator<_IteratorValueType> operator-(int lhs, const name::base_iterator<_IteratorValueType>& rhs);
						template <typename _IteratorValueType>
						friend typename name::base_iterator<IteratorValueType>::difference_type operator-(const name::base_iterator<_IteratorValueType>& lhs, const name::base_iterator<_IteratorValueType>& rhs);
				};

				/**
				 * \brief The iterator type.
				 */
				typedef base_iterator<name> iterator;

				/**
				 * \brief The iterator type.
				 */
				typedef base_iterator<const name> const_iterator;

				/**
				* \brief Reverse iterator type.
				*/
				typedef std::reverse_iterator<iterator> reverse_iterator;

				/**
				* \brief Const reverse iterator type.
				*/
				typedef std::reverse_iterator<const_iterator> reverse_const_iterator;

				/**
				 * \brief Create a new name.
				 * \return The name.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static name create();

				/**
				 * \brief Take ownership of a specified X509_NAME pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A name_entry.
				 */
				static name take_ownership(pointer ptr);

				/**
				 * \brief Load a X509 name in DER format.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \return The name.
				 */
				static name from_der(const void* buf, size_t buf_len);

				/**
				 * \brief Load a X509 name in DER format.
				 * \param buf The buffer.
				 * \return The name.
				 */
				static name from_der(const buffer& buf);

				/**
				 * \brief Create a new empty name.
				 */
				name();

				/**
				 * \brief Create a X509 name by *NOT* taking ownership of an existing X509_NAME* pointer.
				 * \param ptr The X509_NAME* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				name(pointer ptr);

				/**
				 * \brief Write the name in DER format to a buffer.
				 * \param buf The buffer to write too. If NULL is specified, only the needed size is returned.
				 * \return The size written or to be written.
				 */
				size_t write_der(void* buf) const;

				/**
				 * \brief Write the name in DER format to a buffer.
				 * \return The buffer.
				 */
				buffer write_der() const;

				/**
				 * \brief Clone the name instance.
				 * \return The clone.
				 */
				name clone() const;

				/**
				 * \brief Get the hash of the name.
				 * \return The hash.
				 */
				unsigned int hash() const;

				/**
				 * \brief Get a one-line human readable representation of the X509 name.
				 * \param max_size The maximum size of the result. Default is 256.
				 * \return A string.
				 */
				std::string oneline(size_t max_size = 256) const;

				/**
				 * \brief Print a X509 name to a BIO.
				 * \param bio The BIO.
				 * \param obase An undocumented parameter. Defaulted to 0.
				 */
				void print(bio::bio_ptr bio, int obase = 0) const;

				/**
				 * \brief Get a complete string representation of the name.
				 * \return The string representation.
				 */
				std::string to_string() const;

				/**
				 * \brief Get the count of entries.
				 * \return The count of entries.
				 */
				int count() const;

				/**
				 * \brief Get the entry at the specified position.
				 * \param index The index. Must be a valid index position or the behavior is undefined. See count().
				 * \return The name entry.
				 * \see count().
				 */
				wrapped_value_type operator[](int index) const;

				/**
				 * \brief Get the begin iterator.
				 * \return The begin iterator.
				 */
				const_iterator begin() const;

				/**
				 * \brief Get the begin iterator.
				 * \return The begin iterator.
				 */
				iterator begin();

				/**
				 * \brief Get the end iterator.
				 * \return The end iterator.
				 */
				const_iterator end() const;

				/**
				 * \brief Get the end iterator.
				 * \return The end iterator.
				 */
				iterator end();

				/**
				 * \brief Get the reverse begin iterator.
				 * \return The reverse begin iterator.
				 */
				reverse_const_iterator rbegin() const;

				/**
				 * \brief Get the reverse begin iterator.
				 * \return The reverse begin iterator.
				 */
				reverse_iterator rbegin();

				/**
				 * \brief Get the reverse end iterator.
				 * \return The reverse end iterator.
				 */
				reverse_const_iterator rend() const;

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
				const_iterator erase(const_iterator it) const;

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
				const_iterator erase(const_iterator first, const_iterator last) const;

				/**
				 * \brief Erase the given entries.
				 * \param first The first iterator.
				 * \param last The last iterator.
				 * \return last.
				 */
				iterator erase(iterator first, iterator last);

				/**
				 * \brief Find an entry by its NID.
				 * \param nid The nid.
				 * \return An iterator to the first entry that matches, or end() if none is found.
				 */
				const_iterator find(int nid) const;

				/**
				 * \brief Find an entry by its NID.
				 * \param nid The nid.
				 * \return An iterator to the first entry that matches, or end() if none is found.
				 */
				iterator find(int nid);

				/**
				 * \brief Find an entry by its NID.
				 * \param nid The nid.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an entry that matches, or end() if none is found.
				 */
				const_iterator find(int nid, const_iterator lastpos) const;

				/**
				 * \brief Find an entry by its NID.
				 * \param nid The nid.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an entry that matches, or end() if none is found.
				 */
				iterator find(int nid, iterator lastpos);

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \return An iterator to the first entry that matches, or end() if none is found.
				 */
				const_iterator find(asn1::object object) const;

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \return An iterator to the first entry that matches, or end() if none is found.
				 */
				iterator find(asn1::object object);

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an entry that matches, or end() if none is found.
				 */
				const_iterator find(asn1::object object, const_iterator lastpos) const;

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an entry that matches, or end() if none is found.
				 */
				iterator find(asn1::object object, iterator lastpos);

				/**
				 * \brief Clear all entries.
				 */
				void clear() const;

				/**
				 * \brief Push a copy of the specified name_entry at the end of the entry table.
				 * \param entry The name entry.
				 */
				void push_back(wrapped_value_type entry) const;

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param field The field.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(const std::string& field, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				* \brief Push a new entry at the end of the entry table.
				* \param field The field.
				* \param type The type.
				* \param data The data.
				* \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				*/
				void push_back(const std::string& field, int type, const std::string& data, int set = 0) const
				{
					push_back(field, type, data.empty() ? nullptr : &data[0], data.size(), set);
				}

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param object The ASN1 object.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(asn1::object object, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				* \brief Push a new entry at the end of the entry table.
				* \param object The ASN1 object.
				* \param type The type.
				* \param data The data.
				* \param data_len The length of data.
				* \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				*/
				void push_back(asn1::object object, int type, const std::string& data, int set = 0) const
				{
					push_back(object, type, data.empty() ? nullptr : &data[0], data.size(), set);
				}

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param nid The NID.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(int nid, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				* \brief Push a new entry at the end of the entry table.
				* \param nid The NID.
				* \param type The type.
				* \param data The data string.
				* \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				*/
				void push_back(int nid, int type, const std::string& data, int set = 0) const
				{
					push_back(nid, type, data.empty() ? nullptr : &data[0], data.size(), set);
				}

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \return An iterator to the entry that was added.
				 */
				const_iterator insert(const_iterator position, wrapped_value_type entry) const;

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \return An iterator to the entry that was added.
				 */
				iterator insert(iterator position, wrapped_value_type entry);

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the call is equivalent to insert(position, entry) without a return value.
				 */
				void insert(const_iterator position, wrapped_value_type entry, int set) const;

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the call is equivalent to insert(position, entry) without a return value.
				 */
				void insert(iterator position, wrapped_value_type entry, int set);

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param field The field.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(const_iterator position, const std::string& field, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param field The field.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(iterator position, const std::string& field, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param object The ASN1 object.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(const_iterator position, asn1::object object, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param object The ASN1 object.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(iterator position, asn1::object object, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param nid The NID.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(const_iterator position, int nid, int type, const void* data, size_t data_len, int set = 0) const;

				/**
				 * \brief Insert a new entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param nid The NID.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void insert(iterator position, int nid, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Insert a copy of the specified name entries in the entry table.
				 * \param position The position to insert the first entry at.
				 * \param first An iterator to the first entry to insert.
				 * \param last An iterator past the last entry to insert.
				 */
				void insert(const_iterator position, const_iterator first, const_iterator last) const;

				/**
				 * \brief Insert a copy of the specified name entries in the entry table.
				 * \param position The position to insert the first entry at.
				 * \param first An iterator to the first entry to insert.
				 * \param last An iterator past the last entry to insert.
				 */
				void insert(iterator position, iterator first, iterator last);

			private:

				name(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name::iterator instances point to the same element.
		 */
		template <typename IteratorValueType>
		bool operator==(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name::iterator instances do not point to the same element.
		 */
		template <typename IteratorValueType>
		bool operator!=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than rhs.
		 */
		template <typename IteratorValueType>
		bool operator<(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than or equal to rhs.
		 */
		template <typename IteratorValueType>
		bool operator<=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than rhs.
		 */
		template <typename IteratorValueType>
		bool operator>(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than or equal to rhs.
		 */
		template <typename IteratorValueType>
		bool operator>=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		template <typename IteratorValueType>
		name::base_iterator<IteratorValueType> operator+(const name::base_iterator<IteratorValueType>& lhs, int rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		template <typename IteratorValueType>
		name::base_iterator<IteratorValueType> operator+(int lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		template <typename IteratorValueType>
		name::base_iterator<IteratorValueType> operator-(const name::base_iterator<IteratorValueType>& lhs, int rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		template <typename IteratorValueType>
		name::base_iterator<IteratorValueType> operator-(int lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Substract a iterator from another iterator and gets the index distance.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The distance.
		 */
		template <typename IteratorValueType>
		typename name::base_iterator<IteratorValueType>::difference_type operator-(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs);

		/**
		 * \brief Compare two name instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name instances share the same underlying pointer.
		 */
		bool operator==(const name& lhs, const name& rhs);

		/**
		 * \brief Compare two name instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name instances do not share the same underlying pointer.
		 */
		bool operator!=(const name& lhs, const name& rhs);

		/**
		 * \brief Compare two names, by value.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The result of the comparison, which is 0 if the two arguments are identical.
		 */
		int compare(const name& lhs, const name& rhs);

		/**
		 * \brief Output the name to an output stream.
		 * \param os The output stream.
		 * \param value The value instance..
		 * \return The output stream.
		 */
		std::ostream& operator<<(std::ostream& os, const name& value);

		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>::base_iterator() : m_owner(NULL), m_index(0)
		{
		}
		template <typename IteratorValueType>
		inline typename name::base_iterator<IteratorValueType>::reference name::base_iterator<IteratorValueType>::operator*() const
		{
			return (m_cache = (*m_owner)[m_index]);
		}
		template <typename IteratorValueType>
		inline typename name::base_iterator<IteratorValueType>::pointer name::base_iterator<IteratorValueType>::operator->() const
		{
			return &operator*();
		}
		template <typename IteratorValueType>
		inline typename name::base_iterator<IteratorValueType>::reference name::base_iterator<IteratorValueType>::operator[](int index) const
		{
			return *iterator(m_owner, m_index + index);
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>& name::base_iterator<IteratorValueType>::operator++()
		{
			++m_index;

			return *this;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> name::base_iterator<IteratorValueType>::operator++(int)
		{
			base_iterator<IteratorValueType> old = *this;

			++m_index;

			return old;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>& name::base_iterator<IteratorValueType>::operator--()
		{
			--m_index;

			return *this;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> name::base_iterator<IteratorValueType>::operator--(int)
		{
			base_iterator<IteratorValueType> old = *this;

			--m_index;

			return old;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>& name::base_iterator<IteratorValueType>::operator+=(int cnt)
		{
			m_index += cnt;

			return *this;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>& name::base_iterator<IteratorValueType>::operator-=(int cnt)
		{
			m_index -= cnt;

			return *this;
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType>::base_iterator(IteratorValueType* _name, int index) : m_owner(_name), m_index(index)
		{
		}
		inline name name::create()
		{
			pointer _ptr = X509_NAME_new();

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline name name::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509_NAME(NULL, &pbuf, static_cast<long>(buf_len)));
		}
		inline name name::from_der(const buffer& buf)
		{
			return from_der(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}
		inline name::name()
		{
		}
		inline name::name(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline size_t name::write_der(void* buf) const
		{
			unsigned char* out = static_cast<unsigned char*>(buf);
			unsigned char** pout = out != NULL ? &out : NULL;

			int result = i2d_X509_NAME(ptr().get(), pout);

			throw_error_if(result < 0);

			return result;
		}
		inline buffer name::write_der() const
		{
			buffer result(write_der(static_cast<void*>(NULL)));

			write_der(buffer_cast<uint8_t*>(result));

			return result;
		}
		inline name name::clone() const
		{
			return name(X509_NAME_dup(ptr().get()));
		}
		inline unsigned int name::hash() const
		{
			return X509_NAME_hash(ptr().get());
		}
		inline std::string name::oneline(size_t max_size) const
		{
			std::string result(max_size + 1, ' ');

			char* c = X509_NAME_oneline(ptr().get(), &result[0], static_cast<int>(result.size() - 1));

			throw_error_if_not(c);

			result.resize(std::strlen(c));

			return result;
		}
		inline void name::print(bio::bio_ptr bio, int obase) const
		{
			throw_error_if_not(X509_NAME_print(bio.raw(), ptr().get(), obase) != 0);
		}
		inline int name::count() const
		{
			return X509_NAME_entry_count(ptr().get());
		}
		inline name::wrapped_value_type name::operator[](int index) const
		{
			return wrapped_value_type(X509_NAME_get_entry(ptr().get(), index));
		}
		inline name::const_iterator name::begin() const
		{
			return const_iterator(this, 0);
		}
		inline name::iterator name::begin()
		{
			return iterator(this, 0);
		}
		inline name::const_iterator name::end() const
		{
			return const_iterator(this, count());
		}
		inline name::iterator name::end()
		{
			return iterator(this, count());
		}
		inline name::reverse_const_iterator name::rbegin() const
		{
			return reverse_const_iterator(end());
		}
		inline name::reverse_iterator name::rbegin()
		{
			return reverse_iterator(end());
		}
		inline name::reverse_const_iterator name::rend() const
		{
			return reverse_const_iterator(begin());
		}
		inline name::reverse_iterator name::rend()
		{
			return reverse_iterator(begin());
		}
		inline name::const_iterator name::erase(const_iterator it) const
		{
			wrapped_value_type::take_ownership(X509_NAME_delete_entry(it.m_owner->ptr().get(), it.m_index));

			return it;
		}
		inline name::iterator name::erase(iterator it)
		{
			wrapped_value_type::take_ownership(X509_NAME_delete_entry(it.m_owner->ptr().get(), it.m_index));

			return it;
		}
		inline name::const_iterator name::erase(const_iterator first, const_iterator last) const
		{
			while (first != last)
				first = erase(first);

			return first;
		}
		inline name::iterator name::erase(iterator first, iterator last)
		{
			while (first != last)
				first = erase(first);

			return first;
		}
		inline name::const_iterator name::find(int nid) const
		{
			int index = X509_NAME_get_index_by_NID(ptr().get(), nid, -1);

			return (index < 0) ? end() : const_iterator(this, index);
		}
		inline name::iterator name::find(int nid)
		{
			int index = X509_NAME_get_index_by_NID(ptr().get(), nid, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::const_iterator name::find(int nid, const_iterator lastpos) const
		{
			int index = X509_NAME_get_index_by_NID(ptr().get(), nid, lastpos.m_index);

			return (index < 0) ? end() : const_iterator(this, index);
		}
		inline name::iterator name::find(int nid, iterator lastpos)
		{
			int index = X509_NAME_get_index_by_NID(ptr().get(), nid, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::const_iterator name::find(asn1::object object) const
		{
			int index = X509_NAME_get_index_by_OBJ(ptr().get(), object.raw(), -1);

			return (index < 0) ? end() : const_iterator(this, index);
		}
		inline name::iterator name::find(asn1::object object)
		{
			int index = X509_NAME_get_index_by_OBJ(ptr().get(), object.raw(), -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::const_iterator name::find(asn1::object object, const_iterator lastpos) const
		{
			int index = X509_NAME_get_index_by_OBJ(ptr().get(), object.raw(), lastpos.m_index);

			return (index < 0) ? end() : const_iterator(this, index);
		}
		inline name::iterator name::find(asn1::object object, iterator lastpos)
		{
			int index = X509_NAME_get_index_by_OBJ(ptr().get(), object.raw(), lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline void name::clear() const
		{
			erase(begin(), end());
		}
		inline void name::push_back(wrapped_value_type entry) const
		{
			throw_error_if_not(X509_NAME_add_entry(ptr().get(), entry.raw(), -1, 0) != 0);
		}
		inline void name::push_back(const std::string& field, int type, const void* data, size_t data_len, int set) const
		{
			throw_error_if_not(X509_NAME_add_entry_by_txt(ptr().get(), field.c_str(), type, static_cast<const unsigned char*>(data), static_cast<int>(data_len), -1, set) != 0);
		}
		inline void name::push_back(asn1::object object, int type, const void* data, size_t data_len, int set) const
		{
			throw_error_if_not(X509_NAME_add_entry_by_OBJ(ptr().get(), object.raw(), type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), -1, set) != 0);
		}
		inline void name::push_back(int nid, int type, const void* data, size_t data_len, int set) const
		{
			throw_error_if_not(X509_NAME_add_entry_by_NID(ptr().get(), nid, type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), -1, set) != 0);
		}
		inline name::const_iterator name::insert(const_iterator position, wrapped_value_type entry) const
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry(ptr().get(), entry.raw(), position.m_index, 0) != 0);

			return position;
		}
		inline name::iterator name::insert(iterator position, wrapped_value_type entry)
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry(ptr().get(), entry.raw(), position.m_index, 0) != 0);

			return position;
		}
		inline void name::insert(const_iterator position, wrapped_value_type entry, int set) const
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry(ptr().get(), entry.raw(), position.m_index, set) != 0);
		}
		inline void name::insert(iterator position, wrapped_value_type entry, int set)
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry(ptr().get(), entry.raw(), position.m_index, set) != 0);
		}
		inline void name::insert(const_iterator position, const std::string& field, int type, const void* data, size_t data_len, int set) const
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_txt(ptr().get(), field.c_str(), type, static_cast<const unsigned char*>(data), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(iterator position, const std::string& field, int type, const void* data, size_t data_len, int set)
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_txt(ptr().get(), field.c_str(), type, static_cast<const unsigned char*>(data), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(const_iterator position, asn1::object object, int type, const void* data, size_t data_len, int set) const
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_OBJ(ptr().get(), object.raw(), type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(iterator position, asn1::object object, int type, const void* data, size_t data_len, int set)
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_OBJ(ptr().get(), object.raw(), type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(const_iterator position, int nid, int type, const void* data, size_t data_len, int set) const
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_NID(ptr().get(),nid, type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(iterator position, int nid, int type, const void* data, size_t data_len, int set)
		{
			assert(position.m_owner == this);

			throw_error_if_not(X509_NAME_add_entry_by_NID(ptr().get(),nid, type, static_cast<unsigned char*>(const_cast<void*>(data)), static_cast<int>(data_len), position.m_index, set) != 0);
		}
		inline void name::insert(const_iterator position, const_iterator first, const_iterator last) const
		{
			assert(position.m_owner == this);
			assert(first.m_owner == last.m_owner);

			for(; first != last; ++first)
			{
				position = insert(position, *first) + 1;
			}
		}
		inline void name::insert(iterator position, iterator first, iterator last)
		{
			assert(position.m_owner == this);
			assert(first.m_owner == last.m_owner);

			for(; first != last; ++first)
			{
				position = insert(position, *first) + 1;
			}
		}
		inline name::name(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		template <typename IteratorValueType>
		inline bool operator==(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index == rhs.m_index);
		}
		template <typename IteratorValueType>
		inline bool operator!=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index != rhs.m_index);
		}
		template <typename IteratorValueType>
		inline bool operator<(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index < rhs.m_index);
		}
		template <typename IteratorValueType>
		inline bool operator<=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index <= rhs.m_index);
		}
		template <typename IteratorValueType>
		inline bool operator>(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index > rhs.m_index);
		}
		template <typename IteratorValueType>
		inline bool operator>=(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			assert(lhs.m_owner == rhs.m_owner);

			return (lhs.m_index >= rhs.m_index);
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> operator+(const name::base_iterator<IteratorValueType>& lhs, int rhs)
		{
			return name::base_iterator<IteratorValueType>(lhs.m_owner, lhs.m_index + rhs);
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> operator+(int lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			return name::base_iterator<IteratorValueType>(rhs.m_owner, rhs.m_index + lhs);
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> operator-(const name::base_iterator<IteratorValueType>& lhs, int rhs)
		{
			return name::base_iterator<IteratorValueType>(lhs.m_owner, lhs.m_index - rhs);
		}
		template <typename IteratorValueType>
		inline name::base_iterator<IteratorValueType> operator-(int lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			return name::base_iterator<IteratorValueType>(rhs.m_owner, rhs.m_index - lhs);
		}
		template <typename IteratorValueType>
		inline typename name::base_iterator<IteratorValueType>::difference_type operator-(const name::base_iterator<IteratorValueType>& lhs, const name::base_iterator<IteratorValueType>& rhs)
		{
			return lhs.m_index - rhs.m_index;
		}
		inline bool operator==(const name& lhs, const name& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const name& lhs, const name& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline int compare(const name& lhs, const name& rhs)
		{
			return X509_NAME_cmp(lhs.raw(), rhs.raw());
		}
		inline std::ostream& operator<<(std::ostream& os, const name& value)
		{
			return os << value.to_string();
		}
	}
}

#endif /* CRYPTOPLUS_X509_NAME_HPP */


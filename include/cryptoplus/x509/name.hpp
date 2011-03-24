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

#ifndef CRYPTOPEN_X509_NAME_HPP
#define CRYPTOPEN_X509_NAME_HPP

#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "name_entry.hpp"

// Windows defines a X509_NAME structure as well...
#ifdef X509_NAME
#undef X509_NAME
#endif

#include <openssl/x509.h>

#include <boost/shared_ptr.hpp>

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
		 */
		class name
		{
			public:

				/**
				* \brief The type.
				*/
				typedef name_entry value_type;

				/**
				* \brief he difference type.
				*/
				typedef int difference_type;

				/**
				* \brief The reference type.
				*/
				typedef value_type& reference;

				/**
				* \brief The pointer type.
				*/
				typedef value_type* pointer;

				/**
				 * \brief An iterator class.
				 */
				class iterator : public std::iterator<std::random_access_iterator_tag, value_type>
				{
					public:

						/**
						 * \brief Create an empty iterator.
						 */
						iterator();

						/**
						 * \brief Dereference operator.
						 * \return The reference.
						 */
						value_type operator*();

						/**
						 * \brief Dereference operator.
						 * \return The reference.
						 */
						value_type operator->();

						/**
						 * \brief Dereference operator.
						 * \param index The index to add or substract.
						 * \return An iterator.
						 */
						value_type operator[](int index);

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

						iterator(name*, int);

						name* m_name;
						int m_index;

						friend class name;
						friend bool operator==(const name::iterator& lhs, const name::iterator& rhs);
						friend bool operator!=(const name::iterator& lhs, const name::iterator& rhs);
						friend bool operator<(const name::iterator& lhs, const name::iterator& rhs);
						friend bool operator<=(const name::iterator& lhs, const name::iterator& rhs);
						friend bool operator>(const name::iterator& lhs, const name::iterator& rhs);
						friend bool operator>=(const name::iterator& lhs, const name::iterator& rhs);
						friend name::iterator operator+(const name::iterator& lhs, int rhs);
						friend name::iterator operator+(int lhs, const name::iterator& rhs);
						friend name::iterator operator-(const name::iterator& lhs, int rhs);
						friend name::iterator operator-(int lhs, const name::iterator& rhs);
				};

				/**
				* \brief Reverse iterator type.
				*/
				typedef std::reverse_iterator<iterator> reverse_iterator;

				/**
				 * \brief Create a new empty X509 name.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				name();

				/**
				 * \brief Create a X509 name by taking ownership of an existing X509_NAME* pointer.
				 * \param x509_name The X509_name* pointer. Cannot be NULL.
				 */
				explicit name(X509_NAME* x509_name);

				/**
				 * \brief Get the raw X509_NAME pointer.
				 * \return The raw X509_NAME pointer.
				 * \warning The instance has ownership of the returned pointer. Calling X509_NAME_free() on the returned value will result in undefined behavior.
				 */
				const X509_NAME* raw() const;

				/**
				 * \brief Get the raw X509_NAME pointer.
				 * \return The raw X509_NAME pointer.
				 * \warning The instance has ownership of the returned pointer. Calling X509_NAME_free() on the returned value will result in undefined behavior.
				 */
				X509_NAME* raw();

				/**
				 * \brief Clone the name instance.
				 * \return The clone.
				 */
				name clone() const;

				/**
				 * \brief Get the hash of the name.
				 * \return The hash.
				 */
				unsigned int hash();

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
				void print(bio::bio_ptr bio, int obase = 0);

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
				value_type operator[](int index);

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
				iterator find(int nid, iterator lastpos);

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \return An iterator to the first entry that matches, or end() if none is found.
				 */
				iterator find(asn1::object_ptr object);

				/**
				 * \brief Find an entry by its ASN1 object.
				 * \param object The ASN1 object.
				 * \param lastpos The iterator to start the search after.
				 * \return An iterator to an entry that matches, or end() if none is found.
				 */
				iterator find(asn1::object_ptr object, iterator lastpos);

				/**
				 * \brief Clear all entries.
				 */
				void clear();

				/**
				 * \brief Push a copy of the specified name_entry at the end of the entry table.
				 * \param entry The name entry.
				 */
				void push_back(value_type entry);

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param field The field.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(const std::string& field, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param object The ASN1 object.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(asn1::object_ptr object, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Push a new entry at the end of the entry table.
				 * \param nid The NID.
				 * \param type The type.
				 * \param data The data.
				 * \param data_len The length of data.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the default, a new RDN is created.
				 */
				void push_back(int nid, int type, const void* data, size_t data_len, int set = 0);

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \return An iterator to the entry that was added.
				 */
				iterator insert(iterator position, value_type entry);

				/**
				 * \brief Insert a copy of the specified name_entry in the entry table.
				 * \param position The position to insert the entry at.
				 * \param entry The name entry.
				 * \param set If set is -1 or 1, the entry will be added to the previous or next RDN structure respectively. If set is 0, the call is equivalent to insert(position, entry) without a return value.
				 */
				void insert(iterator position, value_type entry, int set);

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
				void insert(iterator position, asn1::object_ptr object, int type, const void* data, size_t data_len, int set = 0);

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
				void insert(iterator position, iterator first, iterator last);

			private:

				static void null_deleter(X509_NAME*);

				explicit name(boost::shared_ptr<X509_NAME> x509_name);

				boost::shared_ptr<X509_NAME> m_x509_name;

				friend class certificate;
		};

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name::iterator instances point to the same element.
		 */
		bool operator==(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name::iterator instances do not point to the same element.
		 */
		bool operator!=(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than rhs.
		 */
		bool operator<(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is smaller than or equal to rhs.
		 */
		bool operator<=(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than rhs.
		 */
		bool operator>(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Compare two name::iterator instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is greater than or equal to rhs.
		 */
		bool operator>=(const name::iterator& lhs, const name::iterator& rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		name::iterator operator+(const name::iterator& lhs, int rhs);

		/**
		 * \brief Add an integer value to an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		name::iterator operator+(int lhs, const name::iterator& rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		name::iterator operator-(const name::iterator& lhs, int rhs);

		/**
		 * \brief Substract an integer value from an iterator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The new iterator.
		 */
		name::iterator operator-(int lhs, const name::iterator& rhs);

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

		inline name::iterator::iterator() : m_name(NULL), m_index(0)
		{
		}
		inline name::iterator::value_type name::iterator::operator*()
		{
			return (*m_name)[m_index];
		}
		inline name::iterator::value_type name::iterator::operator->()
		{
			return operator*();
		}
		inline name::iterator::value_type name::iterator::operator[](int index)
		{
			return *iterator(m_name, m_index + index);
		}
		inline name::iterator& name::iterator::operator++()
		{
			++m_index;

			return *this;
		}
		inline name::iterator name::iterator::operator++(int)
		{
			iterator old = *this;

			++m_index;

			return old;
		}
		inline name::iterator& name::iterator::operator--()
		{
			--m_index;

			return *this;
		}
		inline name::iterator name::iterator::operator--(int)
		{
			iterator old = *this;

			--m_index;

			return old;
		}
		inline name::iterator& name::iterator::operator+=(int cnt)
		{
			m_index += cnt;

			return *this;
		}
		inline name::iterator& name::iterator::operator-=(int cnt)
		{
			m_index -= cnt;

			return *this;
		}
		inline name::iterator::iterator(name* _name, int index) : m_name(_name), m_index(index)
		{
		}
		inline name::name() : m_x509_name(X509_NAME_new(), X509_NAME_free)
		{
			error::throw_error_if_not(m_x509_name);
		}
		inline name::name(X509_NAME* x509_name) : m_x509_name(x509_name, X509_NAME_free)
		{
			if (!m_x509_name)
			{
				throw std::invalid_argument("x509_name");
			}
		}
		inline const X509_NAME* name::raw() const
		{
			return m_x509_name.get();
		}
		inline X509_NAME* name::raw()
		{
			return m_x509_name.get();
		}
		inline name name::clone() const
		{
			return name(X509_NAME_dup(m_x509_name.get()));
		}
		inline unsigned int name::hash()
		{
			return X509_NAME_hash(m_x509_name.get());
		}
		inline std::string name::oneline(size_t max_size) const
		{
			std::string result(' ', max_size + 1);

			char* c = X509_NAME_oneline(m_x509_name.get(), &result[0], result.size() - 1);

			error::throw_error_if_not(c);

			result.resize(std::strlen(c));

			return result;
		}
		inline void name::print(bio::bio_ptr bio, int obase)
		{
			error::throw_error_if_not(X509_NAME_print(bio.raw(), m_x509_name.get(), obase));
		}
		inline int name::count()
		{
			return X509_NAME_entry_count(m_x509_name.get());
		}
		inline name::value_type name::operator[](int index)
		{
			boost::shared_ptr<X509_NAME_ENTRY> name_entry_ptr(X509_NAME_get_entry(m_x509_name.get(), index), name_entry::null_deleter);

			return value_type(name_entry_ptr);
		}
		inline name::iterator name::begin()
		{
			return iterator(this, 0);
		}
		inline name::iterator name::end()
		{
			return iterator(this, count());
		}
		inline name::reverse_iterator name::rbegin()
		{
			return reverse_iterator(end());
		}
		inline name::reverse_iterator name::rend()
		{
			return reverse_iterator(begin());
		}
		inline name::iterator name::erase(iterator it)
		{
			X509_NAME_ENTRY* entry = X509_NAME_delete_entry(it.m_name->m_x509_name.get(), it.m_index);

			assert(entry);
			error::throw_error_if_not(entry);

			X509_NAME_ENTRY_free(entry);

			return it;
		}
		inline name::iterator name::erase(iterator first, iterator last)
		{
			while (first != last)
				first = erase(first);

			return first;
		}
		inline name::iterator name::find(int nid)
		{
			int index = X509_NAME_get_index_by_NID(m_x509_name.get(), nid, -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::iterator name::find(int nid, iterator lastpos)
		{
			int index = X509_NAME_get_index_by_NID(m_x509_name.get(), nid, lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::iterator name::find(asn1::object_ptr object)
		{
			int index = X509_NAME_get_index_by_OBJ(m_x509_name.get(), object.raw(), -1);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline name::iterator name::find(asn1::object_ptr object, iterator lastpos)
		{
			int index = X509_NAME_get_index_by_OBJ(m_x509_name.get(), object.raw(), lastpos.m_index);

			return (index < 0) ? end() : iterator(this, index);
		}
		inline void name::clear()
		{
			erase(begin(), end());
		}
		inline void name::push_back(value_type entry)
		{
			error::throw_error_if_not(X509_NAME_add_entry(m_x509_name.get(), entry.raw(), -1, 0));
		}
		inline void name::push_back(const std::string& field, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_txt(m_x509_name.get(), field.c_str(), type, static_cast<const unsigned char*>(data), data_len, -1, set));
		}
		inline void name::push_back(asn1::object_ptr object, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_OBJ(m_x509_name.get(), object.raw(), type, static_cast<unsigned char*>(const_cast<void*>(data)), data_len, -1, set));
		}
		inline void name::push_back(int nid, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_NID(m_x509_name.get(),nid, type, static_cast<unsigned char*>(const_cast<void*>(data)), data_len, -1, set));
		}
		inline name::iterator name::insert(iterator position, value_type entry)
		{
			error::throw_error_if_not(X509_NAME_add_entry(m_x509_name.get(), entry.raw(), position.m_index, 0));

			return position;
		}
		inline void name::insert(iterator position, value_type entry, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry(m_x509_name.get(), entry.raw(), position.m_index, set));
		}
		inline void name::insert(iterator position, const std::string& field, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_txt(m_x509_name.get(), field.c_str(), type, static_cast<const unsigned char*>(data), data_len, position.m_index, set));
		}
		inline void name::insert(iterator position, asn1::object_ptr object, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_OBJ(m_x509_name.get(), object.raw(), type, static_cast<unsigned char*>(const_cast<void*>(data)), data_len, position.m_index, set));
		}
		inline void name::insert(iterator position, int nid, int type, const void* data, size_t data_len, int set)
		{
			error::throw_error_if_not(X509_NAME_add_entry_by_NID(m_x509_name.get(),nid, type, static_cast<unsigned char*>(const_cast<void*>(data)), data_len, position.m_index, set));
		}
		inline void name::insert(iterator position, iterator first, iterator last)
		{
			for(; first != last; ++first, ++position)
			{
				insert(position, *first);
			}
		}
		inline void name::null_deleter(X509_NAME*)
		{
		}
		inline name::name(boost::shared_ptr<X509_NAME> x509_name) : m_x509_name(x509_name)
		{
			error::throw_error_if_not(m_x509_name);
		}
		inline bool operator==(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name == rhs.m_name) && (lhs.m_index == rhs.m_index);
		}
		inline bool operator!=(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name != rhs.m_name) || (lhs.m_index != rhs.m_index);
		}
		inline bool operator<(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name == rhs.m_name) && (lhs.m_index < rhs.m_index);
		}
		inline bool operator<=(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name == rhs.m_name) && (lhs.m_index <= rhs.m_index);
		}
		inline bool operator>(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name == rhs.m_name) && (lhs.m_index > rhs.m_index);
		}
		inline bool operator>=(const name::iterator& lhs, const name::iterator& rhs)
		{
			return (lhs.m_name == rhs.m_name) && (lhs.m_index >= rhs.m_index);
		}
		inline name::iterator operator+(const name::iterator& lhs, int rhs)
		{
			return name::iterator(lhs.m_name, lhs.m_index + rhs);
		}
		inline name::iterator operator+(int lhs, const name::iterator& rhs)
		{
			return name::iterator(rhs.m_name, rhs.m_index + lhs);
		}
		inline name::iterator operator-(const name::iterator& lhs, int rhs)
		{
			return name::iterator(lhs.m_name, lhs.m_index - rhs);
		}
		inline name::iterator operator-(int lhs, const name::iterator& rhs)
		{
			return name::iterator(rhs.m_name, rhs.m_index - lhs);
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
	}
}

#endif /* CRYPTOPEN_X509_NAME_HPP */


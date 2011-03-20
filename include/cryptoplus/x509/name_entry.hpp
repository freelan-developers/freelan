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
 * \file name_entry.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 name entry class.
 */

#ifndef CRYPTOPEN_X509_NAME_ENTRY_HPP
#define CRYPTOPEN_X509_NAME_ENTRY_HPP

#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../asn1/object_ptr.hpp"

#include <openssl/x509.h>

#include <boost/shared_ptr.hpp>

#include <cstring>
#include <string>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 name entry.
		 *
		 * The name_entry class represents a X509 name_entry.
		 *
		 * A name_entry instance has the same semantic as a X509_NAME_ENTRY* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class name_entry
		{
			public:

				/**
				 * \brief Create a X509 name entry from an ASN1 object.
				 * \param object The object.
				 * \param type The type of the data. A common value is MBSTRING_UTF8, in which case data points to an UTF8 encoded string.
				 * \param data The data to fill in the name entry.
				 * \param data_len The length of data.
				 * \return The name_entry.
				 */
				name_entry from_object(asn1::object_ptr object, int type, const void* data, size_t data_len);

				/**
				 * \brief Create a X509 name entry from a nid.
				 * \param nid The nid.
				 * \param type The type of the data. A common value is MBSTRING_UTF8, in which case data points to an UTF8 encoded string.
				 * \param data The data to fill in the name entry.
				 * \param data_len The length of data.
				 * \return The name_entry.
				 */
				name_entry from_nid(int nid, int type, const void* data, size_t data_len);

				/**
				 * \brief Create a new empty X509 name entry.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				name_entry();

				/**
				 * \brief Create a X509 name entry by taking ownership of an existing X509_NAME_ENTRY* pointer.
				 * \param x509_name_entry The X509_NAME_ENTRY* pointer. Cannot be NULL.
				 */
				explicit name_entry(X509_NAME_ENTRY* x509_name_entry);

				/**
				 * \brief Get the raw X509_NAME_ENTRY pointer.
				 * \return The raw X509_NAME_ENTRY pointer.
				 * \warning The instance has ownership of the return pointer. Calling X509_NAME_ENTRY_free() on the returned value will result in undefined behavior.
				 */
				const X509_NAME_ENTRY* raw() const;

				/**
				 * \brief Get the raw X509_NAME_ENTRY pointer.
				 * \return The raw X509_NAME_ENTRY pointer.
				 * \warning The instance has ownership of the return pointer. Calling X509_NAME_ENTRY_free() on the returned value will result in undefined behavior.
				 */
				X509_NAME_ENTRY* raw();

				/**
				 * \brief Get the ASN1 object associated to this name_entry.
				 * \return The object.
				 */
				asn1::object_ptr object();

				/**
				 * \brief Get the nid associated to this name_entry.
				 * \return The nid.
				 */
				int nid();

				/**
				 * \brief Get the name associated to this name_entry.
				 * \return The name.
				 */
				std::string name();

				/**
				 * \brief Get the long name associated to this name_entry.
				 * \return The long name.
				 */
				std::string long_name();

				/**
				 * \brief Clone the name_entry instance.
				 * \return The clone.
				 */
				name_entry clone() const;

			private:

				static void null_deleter(X509_NAME_ENTRY*);

				explicit name_entry(boost::shared_ptr<X509_NAME_ENTRY> x509_name_entry);

				boost::shared_ptr<X509_NAME_ENTRY> m_x509_name_entry;

				friend class name;
		};

		/**
		 * \brief Compare two name_entry instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name_entry instances share the same underlying pointer.
		 */
		bool operator==(const name_entry& lhs, const name_entry& rhs);

		/**
		 * \brief Compare two name_entry instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two name_entry instances do not share the same underlying pointer.
		 */
		bool operator!=(const name_entry& lhs, const name_entry& rhs);

		inline name_entry name_entry::from_nid(int _nid, int _type, const void* _data, size_t data_len)
		{
			return name_entry(boost::shared_ptr<X509_NAME_ENTRY>(X509_NAME_ENTRY_create_by_NID(NULL, _nid, _type, static_cast<unsigned char*>(const_cast<void*>(_data)), data_len), X509_NAME_ENTRY_free));
		}
		inline name_entry name_entry::from_object(asn1::object_ptr _object, int _type, const void* _data, size_t data_len)
		{
			return name_entry(boost::shared_ptr<X509_NAME_ENTRY>(X509_NAME_ENTRY_create_by_OBJ(NULL, _object.raw(), _type, static_cast<unsigned char*>(const_cast<void*>(_data)), data_len), X509_NAME_ENTRY_free));
		}
		inline name_entry::name_entry() : m_x509_name_entry(X509_NAME_ENTRY_new(), X509_NAME_ENTRY_free)
		{
			error::throw_error_if_not(m_x509_name_entry);
		}
		inline name_entry::name_entry(X509_NAME_ENTRY* x509_name_entry) : m_x509_name_entry(x509_name_entry, X509_NAME_ENTRY_free)
		{
			if (!m_x509_name_entry)
			{
				throw std::invalid_argument("x509_name_entry");
			}
		}
		inline const X509_NAME_ENTRY* name_entry::raw() const
		{
			return m_x509_name_entry.get();
		}
		inline X509_NAME_ENTRY* name_entry::raw()
		{
			return m_x509_name_entry.get();
		}
		inline asn1::object_ptr name_entry::object()
		{
			return X509_NAME_ENTRY_get_object(m_x509_name_entry.get());
		}
		inline int name_entry::nid()
		{
			return object().to_nid();
		}
		inline std::string name_entry::name()
		{
			return OBJ_nid2sn(nid());
		}
		inline std::string name_entry::long_name()
		{
			return OBJ_nid2ln(nid());
		}
		inline name_entry name_entry::clone() const
		{
			return name_entry(X509_NAME_ENTRY_dup(m_x509_name_entry.get()));
		}
		inline void name_entry::null_deleter(X509_NAME_ENTRY*)
		{
		}
		inline name_entry::name_entry(boost::shared_ptr<X509_NAME_ENTRY> x509_name_entry) : m_x509_name_entry(x509_name_entry)
		{
			error::throw_error_if_not(m_x509_name_entry);
		}
		inline bool operator==(const name_entry& lhs, const name_entry& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const name_entry& lhs, const name_entry& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_X509_NAME_ENTRY_HPP */


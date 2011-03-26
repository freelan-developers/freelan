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

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"
#include "../asn1/object.hpp"
#include "../asn1/string.hpp"

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
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class name_entry : public pointer_wrapper<X509_NAME_ENTRY>
		{
			public:

				/**
				 * \brief Create a new name_entry.
				 * \return The name_entry.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static name_entry create();

				/**
				 * \brief Take ownership of a specified X509_NAME_ENTRY pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return A name_entry.
				 */
				static name_entry take_ownership(pointer ptr);

				/**
				 * \brief Create a X509 name entry from an ASN1 object.
				 * \param object The object.
				 * \param type The type of the data. A common value is MBSTRING_UTF8, in which case data points to an UTF8 encoded string.
				 * \param data The data to fill in the name entry.
				 * \param data_len The length of data.
				 * \return The name_entry.
				 */
				static name_entry from_object(asn1::object object, int type, const void* data, size_t data_len);

				/**
				 * \brief Create a X509 name entry from a nid.
				 * \param nid The nid.
				 * \param type The type of the data. A common value is MBSTRING_UTF8, in which case data points to an UTF8 encoded string.
				 * \param data The data to fill in the name entry.
				 * \param data_len The length of data.
				 * \return The name_entry.
				 */
				static name_entry from_nid(int nid, int type, const void* data, size_t data_len);

				/**
				 * \brief Create an empty name_entry.
				 */
				name_entry();

				/**
				 * \brief Create a X509 name entry by *NOT* taking ownership of an existing X509_NAME_ENTRY* pointer.
				 * \param ptr The X509_NAME_ENTRY* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				name_entry(pointer ptr);

				/**
				 * \brief Get the ASN1 object associated to this name_entry.
				 * \return The object.
				 */
				asn1::object object();

				/**
				 * \brief Set the ASN1 object associated to this name_entry.
				 * \param object The object.
				 */
				void set_object(asn1::object object);

				/**
				 * \brief Get the data associated to this name_entry.
				 * \return The data, as an ASN1 string.
				 */
				asn1::string data();

				/**
				 * \brief Set the data associated to this name_entry.
				 * \param type The type of the data to set. Usually MBSTRING_ASC or MBSTRING_UTF8.
				 * \param data The data.
				 * \param data_len The length of data.
				 */
				void set_data(int type, const void* data, size_t data_len);

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

				name_entry(pointer _ptr, deleter_type _del);
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

		inline name_entry name_entry::create()
		{
			pointer _ptr = X509_NAME_ENTRY_new();

			error::throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline name_entry name_entry::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return name_entry(_ptr, deleter);
		}
		inline name_entry name_entry::from_nid(int _nid, int _type, const void* _data, size_t data_len)
		{
			return take_ownership(X509_NAME_ENTRY_create_by_NID(NULL, _nid, _type, static_cast<unsigned char*>(const_cast<void*>(_data)), data_len));
		}
		inline name_entry name_entry::from_object(asn1::object _object, int _type, const void* _data, size_t data_len)
		{
			return take_ownership(X509_NAME_ENTRY_create_by_OBJ(NULL, _object.raw(), _type, static_cast<unsigned char*>(const_cast<void*>(_data)), data_len));
		}
		inline name_entry::name_entry()
		{
		}
		inline name_entry::name_entry(pointer _ptr) : pointer_wrapper(_ptr, null_deleter)
		{
		}
		inline asn1::object name_entry::object()
		{
			return X509_NAME_ENTRY_get_object(ptr().get());
		}
		inline void name_entry::set_object(asn1::object _object)
		{
			error::throw_error_if_not(X509_NAME_ENTRY_set_object(ptr().get(), _object.raw()));
		}
		inline asn1::string name_entry::data()
		{
			return X509_NAME_ENTRY_get_data(ptr().get());
		}
		inline void name_entry::set_data(int type, const void* _data, size_t data_len)
		{
			error::throw_error_if_not(X509_NAME_ENTRY_set_data(ptr().get(), type, static_cast<const unsigned char*>(_data), data_len));
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
			return take_ownership(X509_NAME_ENTRY_dup(ptr().get()));
		}
		inline name_entry::name_entry(pointer _ptr, deleter_type _del) : pointer_wrapper(_ptr, _del)
		{
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


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
 * \file extension.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 extension class.
 */

#ifndef CRYPTOPLUS_X509_EXTENSION_HPP
#define CRYPTOPLUS_X509_EXTENSION_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"
#include "../asn1/object.hpp"
#include "../asn1/string.hpp"
#include "x509v3_context.hpp"

#include <openssl/x509.h>
#include <openssl/lhash.h>

namespace cryptoplus
{
	namespace x509
	{
		/**
		 * \brief A X509 extension.
		 *
		 * The extension class represents a X509 extension.
		 *
		 * A extension instance has the same semantic as a X509_EXTENSION* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the object not to be NULL before calling any of its method. Calling any method (except raw()) on a null object has undefined behavior.
		 */
		class extension : public pointer_wrapper<X509_EXTENSION>
		{
			public:

				/**
				 * \brief Create a new extension.
				 * \return An extension.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static extension create();

				/**
				 * \brief Take ownership of a specified X509_EXTENSION pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An extension.
				 */
				static extension take_ownership(pointer ptr);

				/**
				 * \brief Load a X509 extension in DER format.
				 * \param buf The buffer.
				 * \param buf_len The length of buf.
				 * \return An extension.
				 */
				static extension from_der(const void* buf, size_t buf_len);

				/**
				 * \brief Load a X509 extension in DER format.
				 * \param buf The buffer.
				 * \return An extension.
				 */
				static extension from_der(const buffer& buf);

				/**
				 * \brief Create an extension from a nid and its data.
				 * \param nid The nid.
				 * \param critical The critical flag.
				 * \param data The data.
				 * \return An extension.
				 */
				static extension from_nid(int nid, bool critical, asn1::string data);

				/**
				 * \brief Create an extension from an ASN1 object and its data.
				 * \param obj The ASN1 object.
				 * \param critical The critical flag.
				 * \param data The data.
				 * \return An extension.
				 */
				static extension from_obj(asn1::object obj, bool critical, asn1::string data);

				/**
				 * \brief Create an extension from a nid and its value, using a configuration file and a context.
				 * \param nid The nid.
				 * \param value The value.
				 * \param ctx The context. Can be NULL (defaut).
				 * \param conf The configuration file. Can be NULL (default).
				 * \return An extension.
				 */
				static extension from_nconf_nid(int nid, const char* value, x509v3_context ctx = NULL, CONF* conf = NULL);

				/**
				 * \brief Create a new empty extension.
				 */
				extension();

				/**
				 * \brief Create a X509 extension by *NOT* taking ownership of an existing X509_EXTENSION* pointer.
				 * \param ptr The X509_EXTENSION* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				extension(pointer ptr);

				/**
				 * \brief Write the extension in DER format to a buffer.
				 * \param buf The buffer to write too. If NULL is specified, only the needed size is returned.
				 * \return The size written or to be written.
				 */
				size_t write_der(void* buf) const;

				/**
				 * \brief Write the extension in DER format to a buffer.
				 * \return The buffer.
				 */
				buffer write_der() const;

				/**
				 * \brief Clone the extension instance.
				 * \return The clone.
				 */
				extension clone() const;

				/**
				 * \brief Get the object.
				 * \return The ASN1 object.
				 */
				asn1::object object() const;

				/**
				 * \brief Set the object.
				 * \param obj The ASN1 object.
				 */
				void set_object(asn1::object obj) const;

				/**
				 * \brief Get the critical flag.
				 * \return The critical flag.
				 */
				bool critical() const;

				/**
				 * \brief Set the critical flag.
				 * \param critical The critical flag.
				 */
				void set_critical(bool critical) const;

				/**
				 * \brief Get the data.
				 * \return The data.
				 */
				asn1::string data() const;

				/**
				 * \brief Set the data.
				 * \param data The data.
				 */
				void set_data(asn1::string data) const;

			private:

				extension(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two extension instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two extension instances share the same underlying pointer.
		 */
		bool operator==(const extension& lhs, const extension& rhs);

		/**
		 * \brief Compare two extension instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two extension instances do not share the same underlying pointer.
		 */
		bool operator!=(const extension& lhs, const extension& rhs);

		inline extension extension::create()
		{
			pointer _ptr = X509_EXTENSION_new();

			throw_error_if_not(_ptr);

			return take_ownership(_ptr);
		}
		inline extension extension::from_der(const void* buf, size_t buf_len)
		{
			const unsigned char* pbuf = static_cast<const unsigned char*>(buf);

			return take_ownership(d2i_X509_EXTENSION(NULL, &pbuf, static_cast<long>(buf_len)));
		}
		inline extension extension::from_der(const buffer& buf)
		{
			return from_der(buffer_cast<const uint8_t*>(buf), buffer_size(buf));
		}
		inline extension extension::from_nid(int nid, bool critical, asn1::string data)
		{
			return take_ownership(X509_EXTENSION_create_by_NID(NULL, nid, critical ? 1 : 0, data.raw()));
		}
		inline extension extension::from_obj(asn1::object obj, bool critical, asn1::string data)
		{
			return take_ownership(X509_EXTENSION_create_by_OBJ(NULL, obj.raw(), critical ? 1 : 0, data.raw()));
		}
		inline extension extension::from_nconf_nid(int nid, const char* value, x509v3_context ctx, CONF* conf)
		{
			return take_ownership(X509V3_EXT_nconf_nid(conf, ctx.raw(), nid, const_cast<char*>(value)));
		}
		inline extension::extension()
		{
		}
		inline extension::extension(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline size_t extension::write_der(void* buf) const
		{
			unsigned char* out = static_cast<unsigned char*>(buf);
			unsigned char** pout = out != NULL ? &out : NULL;

			int result = i2d_X509_EXTENSION(ptr().get(), pout);

			throw_error_if(result < 0);

			return result;
		}
		inline buffer extension::write_der() const
		{
			buffer result(write_der(static_cast<void*>(NULL)));

			write_der(buffer_cast<uint8_t*>(result));

			return result;
		}
		inline extension extension::clone() const
		{
			return extension(X509_EXTENSION_dup(ptr().get()));
		}
		inline asn1::object extension::object() const
		{
			return X509_EXTENSION_get_object(ptr().get());
		}
		inline void extension::set_object(asn1::object obj) const
		{
			throw_error_if_not(X509_EXTENSION_set_object(ptr().get(), obj.raw()) != 0);
		}
		inline bool extension::critical() const
		{
			return (X509_EXTENSION_get_critical(ptr().get()) != 0);
		}
		inline void extension::set_critical(bool _critical) const
		{
			throw_error_if_not(X509_EXTENSION_set_critical(ptr().get(), _critical ? 1 : 0) != 0);
		}
		inline asn1::string extension::data() const
		{
			return X509_EXTENSION_get_data(ptr().get());
		}
		inline void extension::set_data(asn1::string _data) const
		{
			throw_error_if_not(X509_EXTENSION_set_data(ptr().get(), _data.raw()) != 0);
		}
		inline extension::extension(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const extension& lhs, const extension& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const extension& lhs, const extension& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_X509_EXTENSION_HPP */


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

#include <openssl/x509.h>

#include <boost/shared_ptr.hpp>

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
				 * \warning The instance has ownership of the return pointer. Calling X509_NAME_free() on the returned value will result in undefined behavior.
				 */
				const X509_NAME* raw() const;

				/**
				 * \brief Get the raw X509_NAME pointer.
				 * \return The raw X509_NAME pointer.
				 * \warning The instance has ownership of the return pointer. Calling X509_NAME_free() on the returned value will result in undefined behavior.
				 */
				X509_NAME* raw();

			private:

				explicit name(boost::shared_ptr<X509_NAME> x509_name);

				boost::shared_ptr<X509_NAME> m_x509_name;
		};

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

		inline name::name() : m_x509_name(X509_NAME_new(), X509_NAME_free)
		{
			error::throw_error_if_not(m_x509_name);
		}
		inline name::name(X509_NAME* x509_name) : m_x509_name(x509_name, X509_NAME_free)
		{
			if (!m_x509_name)
			{
				throw std::invalid_argument("name");
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
		inline name::name(boost::shared_ptr<X509_NAME> x509_name) : m_x509_name(x509_name)
		{
			error::throw_error_if_not(m_x509_name);
		}
		inline bool operator==(const name& lhs, const name& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const name& lhs, const name& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_X509_NAME_HPP */


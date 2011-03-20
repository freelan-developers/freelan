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
				std::string oneline(size_t max_size) const;

				/**
				 * \brief Print a X509 name to a BIO.
				 * \param bio The BIO.
				 * \param obase An undocumented parameter. Defaulted to 0.
				 */
				void print(bio::bio_ptr bio, int obase = 0);

			private:

				static void null_deleter(X509_NAME*);

				explicit name(boost::shared_ptr<X509_NAME> x509_name);

				boost::shared_ptr<X509_NAME> m_x509_name;

				friend class certificate;
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

		/**
		 * \brief Compare two names, by value.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The result of the comparison, which is 0 if the two arguments are identical.
		 */
		int compare(const name& lhs, const name& rhs);

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
			std::string result('\0', max_size + 1);

			char* c = X509_NAME_oneline(m_x509_name.get(), &result[0], result.size() - 1);

			error::throw_error_if_not(c);

			result.resize(std::strlen(c));

			return result;
		}
		inline void name::print(bio::bio_ptr bio, int obase)
		{
			error::throw_error_if_not(X509_NAME_print(bio.raw(), m_x509_name.get(), obase));
		}
		inline void name::null_deleter(X509_NAME*)
		{
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
		inline int compare(const name& lhs, const name& rhs)
		{
			return X509_NAME_cmp(lhs.raw(), rhs.raw());
		}
	}
}

#endif /* CRYPTOPEN_X509_NAME_HPP */


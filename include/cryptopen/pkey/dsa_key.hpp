/*
 * libcryptopen - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptopen.
 *
 * libcryptopen is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptopen is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptopen in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file dsa_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DSA key class.
 */

#ifndef CRYPTOPEN_PKEY_DSA_KEY_HPP
#define CRYPTOPEN_PKEY_DSA_KEY_HPP

#include "../error/cryptographic_exception.hpp"

#include <openssl/dsa.h>
#include <openssl/engine.h>

#include <boost/shared_ptr.hpp>

namespace cryptopen
{
	namespace pkey
	{
		/**
		 * \brief A DSA key.
		 *
		 * The dsa_key class represents a DSA key (with or without a private compound).
		 * dsa_key is a low level structure that offers no mean to know whether the represented DSA key is a public or private key: it is up to the user to ensure that private key related functions will only get called on dsa_key instances that have private key information.
		 *
		 * A dsa_key instance has the same semantic as a DSA* pointer, thus two copies of the same instance share the same underlying pointer.
		 */
		class dsa_key
		{
			public:

				/**
				 * \brief Create a new DSA key.
				 */
				dsa_key();

				/**
				 * \brief Get the raw DSA pointer.
				 * \return The raw DSA pointer.
				 * \warning The instance has ownership of the return pointer. Calling DSA_free() on the returned value will result in undefined behavior.
				 */
				DSA* raw();

				/**
				 * \brief Get the raw DSA pointer.
				 * \return The raw DSA pointer.
				 * \warning The instance has ownership of the return pointer. Calling DSA_free() on the returned value will result in undefined behavior.
				 */
				const DSA* raw() const;

			private:

				boost::shared_ptr<DSA> m_dsa;
		};

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instance share the same underlying pointer.
		 */
		bool operator==(const dsa_key& lhs, const dsa_key& rhs);

		/**
		 * \brief Compare two dsa_key instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two dsa_key instance do not share the same underlying pointer.
		 */
		bool operator!=(const dsa_key& lhs, const dsa_key& rhs);
		
		inline dsa_key::dsa_key() : m_dsa(DSA_new(), DSA_free)
		{
		}
		inline DSA* dsa_key::raw()
		{
			return m_dsa.get();
		}
		inline const DSA* dsa_key::raw() const
		{
			return m_dsa.get();
		}
		inline bool operator==(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const dsa_key& lhs, const dsa_key& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_PKEY_DSA_KEY_HPP */


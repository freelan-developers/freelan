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
 * \file bio.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A bio class.
 */

#ifndef CRYPTOPEN_BIO_bio_HPP
#define CRYPTOPEN_BIO_bio_HPP

#include "../error/cryptographic_exception.hpp"

#include <openssl/bio.h>

#include <boost/shared_ptr.hpp>

namespace cryptopen
{
	namespace bio
	{
		/**
		 * \brief An OpenSSL BIO.
		 *
		 * The bio class represents a BIO.
		 *
		 * A bio instance has the same semantic as a BIO* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * Each bio owns its underlying pointer. Avoid calling BIO_free_all() on a bio.
		 */
		class bio
		{
			public:

				/**
				 * \brief Create a new bio.
				 * \param type The type of the BIO. See the man page of bio(3) to get the list of all BIOs.
				 */
				bio(BIO_METHOD* type);

				/**
				 * \brief Get the raw BIO pointer.
				 * \return The raw BIO pointer.
				 * \warning The instance has ownership of the return pointer. Calling BIO_free() on the returned value will result in undefined behavior.
				 */
				BIO* raw();

				/**
				 * \brief Get the raw BIO pointer.
				 * \return The raw BIO pointer.
				 * \warning The instance has ownership of the return pointer. Calling BIO_free() on the returned value will result in undefined behavior.
				 */
				const BIO* raw() const;

				/**
				 * \brief Get the type of the bio.
				 * \return The type.
				 *
				 * The list of possible types is available on the man page for BIO_find_type(3).
				 */
				int type() const;

			private:
				boost::shared_ptr<BIO> m_bio;
		};

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance share the same underlying pointer.
		 */
		bool operator==(const bio& lhs, const bio& rhs);

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance do not share the same underlying pointer.
		 */
		bool operator!=(const bio& lhs, const bio& rhs);

		inline bio::bio(BIO_METHOD* _type) : m_bio(BIO_new(_type), BIO_free)
		{
			error::throw_error_if_not(m_bio);
		}
		inline BIO* bio::raw()
		{
			return m_bio.get();
		}
		inline const BIO* bio::raw() const
		{
			return m_bio.get();
		}
		inline int bio::type() const
		{
			return BIO_method_type(m_bio.get());
		}
		inline bool operator==(const bio& lhs, const bio& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const bio& lhs, const bio& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_BIO_bio_HPP */


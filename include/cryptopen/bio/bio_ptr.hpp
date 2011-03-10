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
 * \file bio_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A bio pointer class.
 */

#ifndef CRYPTOPEN_BIO_BIO_PTR_HPP
#define CRYPTOPEN_BIO_BIO_PTR_HPP

#include <openssl/bio.h>

#include <stdexcept>

namespace cryptopen
{
	namespace bio
	{
		/**
		 * \brief An OpenSSL BIO pointer.
		 *
		 * The bio class is a wrapper for an OpenSSL BIO* pointer.
		 *
		 * A bio instance has the same semantic as a BIO* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A bio owns *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a bio always points to a valid BIO structure.
		 *
		 * If you require a wrapper for OpenSSL BIO with ownership semantic, see bio.
		 */
		class bio_ptr
		{
			public:

				/**
				 * \brief Create a new bio_ptr.
				 * \param bio The bio to point to. Cannot be NULL.
				 */
				bio_ptr(BIO* bio);

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

				BIO* m_bio;
		};

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance share the same underlying pointer.
		 */
		bool operator==(const bio_ptr& lhs, const bio_ptr& rhs);

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance do not share the same underlying pointer.
		 */
		bool operator!=(const bio_ptr& lhs, const bio_ptr& rhs);

		inline bio_ptr::bio_ptr(BIO* _bio) : m_bio(_bio)
		{
			throw std::invalid_argument("bio");
		}
		inline BIO* bio_ptr::raw()
		{
			return m_bio;
		}
		inline const BIO* bio_ptr::raw() const
		{
			return m_bio;
		}
		inline int bio_ptr::type() const
		{
			return BIO_method_type(m_bio);
		}
		inline bool operator==(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_BIO_BIO_PTR_HPP */


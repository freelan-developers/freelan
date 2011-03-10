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

#include "nullable.hpp"

#include <openssl/bio.h>

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
		 * A bio *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a bio_ptr always points to a valid BIO structure.
		 *
		 * If you require a wrapper for OpenSSL BIO with ownership semantic, see bio_chain.
		 *
		 * \warning Always check for the bio_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset()) on a null bio_ptr has undefined behavior.
		 */
		class bio_ptr : public nullable<bio_ptr>
		{
			public:

				/**
				 * \brief Create a new bio_ptr.
				 * \param bio The bio to point to.
				 */
				explicit bio_ptr(BIO* bio = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param bio The bio to point to.
				 */
				void reset(BIO* bio = NULL);

				/**
				 * \brief Get the raw BIO pointer.
				 * \return The raw BIO pointer.
				 */
				BIO* raw();

				/**
				 * \brief Push a bio_ptr at the bottom of the BIO chain.
				 * \param bio The bio to append to the BIO chain.
				 * \returns The current bio_ptr.
				 */
				bio_ptr push(bio_ptr& bio);

				/**
				 * \brief Remove the bio_ptr from its BIO chain.
				 * \return The next bio_ptr in the chain, or a null one if there is no next bio_ptr.
				 * 
				 * Once it is removed from its chain, the bio_ptr can be either deleted or added to another chain.
				 */
				bio_ptr pop();

				/**
				 * \brief Find a BIO in the BIO chain by its type.
				 * \param type The type of the bio_ptr.
				 * \return The first bio_ptr to match or an empty one if none is found that match the specified type.
				 */
				bio_ptr find_by_type(int type);

				/**
				 * \brief Get the next bio_ptr in the chain.
				 * \return The next bio_ptr in the chain.
				 */
				bio_ptr next();

				/**
				 * \brief Get the type of the bio.
				 * \return The type.
				 *
				 * The list of possible types is available on the man page for BIO_find_type(3).
				 */
				int type();

				/**
				 * \brief Determine if the last operation on the BIO should be retried.
				 * \return true if the last operation should be retried.
				 */
				bool should_retry();

				/**
				 * \brief Determine if the BIO should be read.
				 * \return true if the BIO should be read.
				 */
				bool should_read();

				/**
				 * \brief Determine if the BIO should be written.
				 * \return true if the BIO should be written.
				 */
				bool should_write();

				/**
				 * \brief Determine if the cause of the last failure was due to a special IO event.
				 * \return true if the cause of the last failure was due to a special IO event.
				 */
				bool should_io_special();

				/**
				 * \brief Get the retry type.
				 * \return The retry type, as specified on the man page of BIO_should_retry(3).
				 */
				int retry_type();

			private:

				bool boolean_test() const;

				BIO* m_bio;
				
				friend bool operator==(const bio_ptr& lhs, const bio_ptr& rhs);
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
		}
		inline void bio_ptr::reset(BIO* _bio)
		{
			m_bio = _bio;
		}
		inline BIO* bio_ptr::raw()
		{
			return m_bio;
		}
		inline bio_ptr bio_ptr::push(bio_ptr& bio)
		{
			return bio_ptr(BIO_push(m_bio, bio.raw()));
		}
		inline bio_ptr bio_ptr::pop()
		{
			return bio_ptr(BIO_pop(m_bio));
		}
		inline bio_ptr bio_ptr::find_by_type(int _type)
		{
			return bio_ptr(BIO_find_type(m_bio, _type));
		}
		inline bio_ptr bio_ptr::next()
		{
			return bio_ptr(BIO_next(m_bio));
		}
		inline int bio_ptr::type()
		{
			return BIO_method_type(m_bio);
		}
		inline bool bio_ptr::should_retry()
		{
			return BIO_should_retry(m_bio) != 0;
		}
		inline bool bio_ptr::should_read()
		{
			return BIO_should_read(m_bio) != 0;
		}
		inline bool bio_ptr::should_write()
		{
			return BIO_should_write(m_bio) != 0;
		}
		inline bool bio_ptr::should_io_special()
		{
			return BIO_should_io_special(m_bio) != 0;
		}
		inline int bio_ptr::retry_type()
		{
			return BIO_retry_type(m_bio);
		}
		inline bool bio_ptr::boolean_test() const
		{
			return (m_bio != NULL);
		}
		inline bool operator==(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return lhs.m_bio == rhs.m_bio;
		}
		inline bool operator!=(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return !(lhs == rhs);
		}
	}
}

#endif /* CRYPTOPEN_BIO_BIO_PTR_HPP */


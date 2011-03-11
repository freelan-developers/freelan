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
 * \file bio_chain.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A bio chain class.
 */

#ifndef CRYPTOPEN_BIO_BIO_CHAIN_HPP
#define CRYPTOPEN_BIO_BIO_CHAIN_HPP

#include "bio_ptr.hpp"
#include "../error/cryptographic_exception.hpp"

#include <boost/shared_ptr.hpp>

#include <stdexcept>

namespace cryptopen
{
	namespace bio
	{
		/**
		 * \brief A BIO chain wrapper class.
		 *
		 * bio_chain is a wrapper around OpenSSL BIOs which has ownership over its underlying pointer.
		 *
		 * It acts as a container for BIO pointers and relies on bio_ptr. The underlying BIO pointer is deleted with BIO_free_all().
		 *
		 * A bio_chain holds a shared pointer to its underlying BIO, thus any copy of the bio_chain instance shares the same pointer.
		 */
		class bio_chain
		{
			public:

				/**
				 * \brief Create a new bio_chain from a BIO_METHOD.
				 * \param type The type.
				 */
				bio_chain(BIO_METHOD* type);

				/**
				 * \brief Get the first BIO in the chain.
				 * \return The first BIO in the chain.
				 * \warning If the bio_chain instance is destroyed, the returned bio_ptr is invalidated.
				 */
				bio_ptr first() const;

			private:

				boost::shared_ptr<BIO> m_bio;
		};
		
		inline bio_chain::bio_chain(BIO_METHOD* _type) : m_bio(BIO_new(_type), BIO_free_all)
		{
			error::throw_error_if_not(m_bio);
		}
		inline bio_ptr bio_chain::first() const
		{
			return bio_ptr(m_bio.get());
		}
	}
}

#endif /* CRYPTOPEN_BIO_BIO_CHAIN_HPP */


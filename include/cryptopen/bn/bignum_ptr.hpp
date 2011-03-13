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
 * \file bignum_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A BIGNUM pointer class.
 */

#ifndef CRYPTOPEN_BN_BIGNUM_PTR_HPP
#define CRYPTOPEN_BN_BIGNUM_PTR_HPP

#include "../nullable.hpp"

#include <openssl/bn.h>

namespace cryptopen
{
	namespace bn
	{
		/**
		 * \brief An OpenSSL BIGNUM pointer.
		 *
		 * The bignum_ptr class is a wrapper for an OpenSSL BIGNUM* pointer.
		 *
		 * A bignum_ptr instance has the same semantic as a BIGNUM* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A bignum_ptr *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a bignum_ptr always points to a valid BIGNUM structure.
		 *
		 * \warning Always check for the bignum_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset_ptr()) on a null bignum_ptr has undefined behavior.
		 */
		class bignum_ptr : public nullable<bignum_ptr>
		{
			public:

				/**
				 * \brief Create a new bignum_ptr.
				 * \param bignum The BIGNUM to point to.
				 */
				bignum_ptr(BIGNUM* bignum = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param bignum The BIGNUM to point to.
				 */
				void reset_ptr(BIGNUM* bignum = NULL);

				/**
				 * \brief Get the raw BIGNUM pointer.
				 * \return The raw BIGNUM pointer.
				 */
				BIGNUM* raw();

				/**
				 * \brief Get the raw BIGNUM pointer.
				 * \return The raw BIGNUM pointer.
				 */
				const BIGNUM* raw() const;

				/**
				 * \brief Get the number of bytes needed to represent the BIGNUM.
				 * \return The number of bytes needed to represent the BIGNUM.
				 */
				size_t size() const;

			private:

				bool boolean_test() const;

				BIGNUM* m_bignum;
		};

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bignum_ptr instance share the same underlying pointer.
		 */
		bool operator==(const bignum_ptr& lhs, const bignum_ptr& rhs);

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bignum_ptr instance do not share the same underlying pointer.
		 */
		bool operator!=(const bignum_ptr& lhs, const bignum_ptr& rhs);

		inline bignum_ptr::bignum_ptr(BIGNUM* _bignum) : m_bignum(_bignum)
		{
		}
		inline void bignum_ptr::reset_ptr(BIGNUM* _bignum)
		{
			m_bignum = _bignum;
		}
		inline BIGNUM* bignum_ptr::raw()
		{
			return m_bignum;
		}
		inline const BIGNUM* bignum_ptr::raw() const
		{
			return m_bignum;
		}
		inline size_t bignum_ptr::size() const
		{
			return BN_num_bytes(m_bignum);
		}
		inline bool bignum_ptr::boolean_test() const
		{
			return (m_bignum != NULL);
		}
		inline bool operator==(const bignum_ptr& lhs, const bignum_ptr& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const bignum_ptr& lhs, const bignum_ptr& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_BN_BIGNUM_PTR_HPP */


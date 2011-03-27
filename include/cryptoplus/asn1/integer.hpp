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
 * \file integer.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_INTEGER pointer class.
 */

#ifndef CRYPTOPEN_ASN1_INTEGER_HPP
#define CRYPTOPEN_ASN1_INTEGER_HPP

#include "../pointer_wrapper.hpp"
#include "../error/cryptographic_exception.hpp"
#include "../bio/bio_ptr.hpp"

#include <openssl/crypto.h>
#include <openssl/asn1.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>

namespace cryptoplus
{
	namespace asn1
	{
		/**
		 * \brief An OpenSSL ASN1_INTEGER pointer.
		 *
		 * The integer class is a wrapper for an OpenSSL ASN1_INTEGER* pointer.
		 *
		 * A integer instance has the same semantic as a ASN1_INTEGER* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the integer not to be NULL before calling any of its method. Calling any method (except raw()) on a null integer has undefined behavior.
		 */
		class integer : public pointer_wrapper<ASN1_INTEGER>
		{
			public:

				/**
				 * \brief Create a new integer.
				 * \return The integer.
				 *
				 * If allocation fails, a cryptographic_exception is thrown.
				 */
				static integer create();

				/**
				 * \brief Take ownership of a specified ASN1_INTEGER pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An integer.
				 */
				static integer take_ownership(pointer ptr);

				/**
				 * \brief Create an integer from a long.
				 * \param l The long value.
				 * \return The integer.
				 */
				static integer from_long(long l);

				/**
				 * \brief Create a new empty integer.
				 */
				integer();

				/**
				 * \brief Create an integer by *NOT* taking ownership of an existing ASN1_INTEGER* pointer.
				 * \param ptr The ASN1_INTEGER* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				integer(pointer ptr);

				/**
				 * \brief Set the time.
				 * \param l The long.
				 */
				void set_value(long l);

				/**
				 * \brief Get a long from the ASN1_INTEGER.
				 * \return A long if the integer is not too big, 0xFFFFFFFFL otherwise.
				 */
				long to_long();

			private:

				explicit integer(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two ASN1 integer pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two integer instance share the same underlying pointer.
		 */
		bool operator==(const integer& lhs, const integer& rhs);

		/**
		 * \brief Compare two ASN1 integer pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two integer instance do not share the same underlying pointer.
		 */
		bool operator!=(const integer& lhs, const integer& rhs);

		inline integer integer::create()
		{
			return take_ownership(ASN1_INTEGER_new());
		}
		inline integer integer::take_ownership(pointer _ptr)
		{
			error::throw_error_if_not(_ptr);

			return integer(_ptr, deleter);
		}
		inline integer integer::from_long(long l)
		{
			integer result = create();
			
			result.set_value(l);

			return result;
		}
		inline integer::integer()
		{
		}
		inline integer::integer(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void integer::set_value(long l)
		{
			error::throw_error_if_not(ASN1_INTEGER_set(ptr().get(), l));
		}
		inline long integer::to_long()
		{
			return ASN1_INTEGER_get(ptr().get());
		}
		inline integer::integer(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const integer& lhs, const integer& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const integer& lhs, const integer& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_ASN1_INTEGER_HPP */


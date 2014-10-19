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

#ifndef CRYPTOPLUS_ASN1_INTEGER_HPP
#define CRYPTOPLUS_ASN1_INTEGER_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"
#include "../bio/bio_ptr.hpp"
#include "../bn/bignum.hpp"

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
				 * If allocation fails, an exception is thrown.
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
				 * \brief Create an integer from a BIGNUM.
				 * \param bn The BIGNUM.
				 * \return The integer.
				 */
				static integer from_bignum(bn::bignum bn);

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
				 * \brief Set the value.
				 * \param l The long.
				 */
				void set_value(long l) const;

				/**
				 * \brief Set the value from a BIGNUM.
				 * \param bn The BIGNUM.
				 */
				void set_value(bn::bignum bn) const;

				/**
				 * \brief Get a long from the ASN1_INTEGER.
				 * \return A long if the integer is not too big, 0xFFFFFFFFL otherwise.
				 */
				long to_long() const;

				/**
				 * \brief Get a BIGNUM from this integer.
				 * \return A BIGNUM.
				 */
				bn::bignum to_bignum() const;

				/**
				 * \brief Write the integer to a BIO.
				 * \param bio The bio to write to.
				 * \return The count of bytes written.
				 */
				size_t write(bio::bio_ptr bio) const;

				/**
				 * \brief Read the integer from a BIO.
				 * \param bio The bio to read from.
				 *
				 * This method uses an internal buffer of size 1024. If you need or want to use your own supplied buffer, see the other read() overloads.
				 */
				void read(bio::bio_ptr bio) const;

				/**
				 * \brief Read the integer from a BIO.
				 * \tparam size The size of the internal buffer to be allocated on the stack.
				 * \param bio The bio to read from.
				 *
				 * This method uses an internal buffer. If you need or want to use your own supplied buffer, see the other read() overloads.
				 */
				template <size_t size>
				void read(bio::bio_ptr bio) const;

				/**
				 * \brief Read the integer from a BIO.
				 * \param bio The bio to read from.
				 * \param buf A buffer to use for the read operation.
				 * \param buf_len The size of buf.
				 */
				void read(bio::bio_ptr bio, const void* buf, size_t buf_len) const;

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

		/**
		 * \brief Compare two ASN1 integer pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return 0 if the two ASN1 integer are identical.
		 */
		int compare(const integer& lhs, const integer& rhs);

		inline integer integer::create()
		{
			return take_ownership(ASN1_INTEGER_new());
		}
		inline integer integer::from_long(long l)
		{
			integer result = create();

			result.set_value(l);

			return result;
		}
		inline integer integer::from_bignum(bn::bignum bn)
		{
			return take_ownership(BN_to_ASN1_INTEGER(bn.raw(), NULL));
		}
		inline integer::integer()
		{
		}
		inline integer::integer(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void integer::set_value(long l) const
		{
			throw_error_if_not(ASN1_INTEGER_set(ptr().get(), l) != 0);
		}
		inline void integer::set_value(bn::bignum bn) const
		{
			throw_error_if_not(BN_to_ASN1_INTEGER(bn.raw(), ptr().get()) != 0);
		}
		inline long integer::to_long() const
		{
			return ASN1_INTEGER_get(ptr().get());
		}
		inline bn::bignum integer::to_bignum() const
		{
			return bn::bignum::from_integer(*this);
		}
		inline size_t integer::write(bio::bio_ptr bio) const
		{
			int result = i2a_ASN1_INTEGER(bio.raw(), ptr().get());

			throw_error_if_not(result >= 0);

			return result;
		}
		inline void integer::read(bio::bio_ptr bio) const
		{
			read<1024>(bio);
		}
		template <size_t size>
		inline void integer::read(bio::bio_ptr bio) const
		{
			char buf[size];

			read(bio, buf, size);
		}
		inline void integer::read(bio::bio_ptr bio, const void* buf, size_t buf_len) const
		{
			throw_error_if_not(a2i_ASN1_INTEGER(bio.raw(), ptr().get(), static_cast<char*>(const_cast<void*>(buf)), static_cast<int>(buf_len)) != 0);
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
		inline int compare(const integer& lhs, const integer& rhs)
		{
#if OPENSSL_VERSION_NUMBER >= 0x01000000
			return ASN1_INTEGER_cmp(lhs.raw(), rhs.raw());
#else
			return ASN1_INTEGER_cmp(const_cast<integer::pointer>(lhs.raw()), const_cast<integer::pointer>(rhs.raw()));
#endif
		}
	}
}

#endif /* CRYPTOPLUS_ASN1_INTEGER_HPP */

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
 * \file bignum.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A BIGNUM pointer class.
 */

#ifndef CRYPTOPLUS_BN_bignum_HPP
#define CRYPTOPLUS_BN_bignum_HPP

#include "../pointer_wrapper.hpp"
#include "../buffer.hpp"
#include "../error/helpers.hpp"

#include <openssl/bn.h>

#include <string>

namespace cryptoplus
{
	namespace asn1
	{
		class integer;
	}

	namespace bn
	{
		/**
		 * \brief An OpenSSL BIGNUM pointer.
		 *
		 * The bignum class is a wrapper for an OpenSSL BIGNUM* pointer.
		 *
		 * A bignum instance has the same semantic as a BIGNUM* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the bignum not to be NULL before calling any of its method. Calling any method (except raw()) on a null bignum has undefined behavior.
		 */
		class bignum : public pointer_wrapper<BIGNUM>
		{
			public:

				/**
				 * \brief Create a new bignum.
				 * \return The bignum.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static bignum create();

				/**
				 * \brief Take ownership of a specified BIGNUM pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An bignum.
				 */
				static bignum take_ownership(pointer ptr);

				/**
				 * \brief Create a BIGNUM from its binary representation.
				 * \param buf The buffer that holds the binary representation of the BIGNUM.
				 * \param buf_len The length of buf.
				 * \return A bignum.
				 *
				 * On error an exception is thrown.
				 */
				static bignum from_bin(const void* buf, size_t buf_len);

				/**
				 * \brief Load a BIGNUM from its hexadecimal representation.
				 * \param str The hexadecimal string representation.
				 * \return A bignum.
				 */
				static bignum from_hex(const std::string& str);

				/**
				 * \brief Load a BIGNUM from its decimal representation.
				 * \param str The decimal string representation.
				 * \return A bignum.
				 */
				static bignum from_dec(const std::string& str);

				/**
				 * \brief Load BIGNUM from an ASN1_INTEGER.
				 * \param i The integer.
				 * \return A bignum.
				 */
				static bignum from_integer(asn1::integer i);

				/**
				 * \brief Load BIGNUM from an unsigned long.
				 * \param ul The unsigned long.
				 * \return A bignum.
				 */
				static bignum from_long(unsigned long ul);

				/**
				 * \brief Create a new empty bignum.
				 */
				bignum();

				/**
				 * \brief Create an bignum by *NOT* taking ownership of an existing BIGNUM pointer.
				 * \param ptr The BIGNUM pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				bignum(pointer ptr);

				/**
				 * \brief Append another bignum to the current instance.
				 * \param bn The bignum.
				 * \return *this.
				 */
				const bignum& operator+=(const bignum& bn) const;

				/**
				 * \brief Append another bignum to the current instance.
				 * \param bn The bignum.
				 * \return *this.
				 */
				bignum& operator+=(const bignum& bn);

				/**
				 * \brief Substract another bignum from the current instance.
				 * \param bn The bignum.
				 * \return *this.
				 */
				const bignum& operator-=(const bignum& bn) const;

				/**
				 * \brief Substract another bignum from the current instance.
				 * \param bn The bignum.
				 * \return *this.
				 */
				bignum& operator-=(const bignum& bn);

				/**
				 * \brief Copy another BIGNUM.
				 * \param bn The bignum.
				 */
				void copy(const bignum& bn);

				/**
				 * \brief Get the number of bytes needed to represent the BIGNUM.
				 * \return The number of bytes needed to represent the BIGNUM.
				 */
				size_t size() const;

				/**
				 * \brief Get the binary representation of the BIGNUM.
				 * \param out The out buffer. Must be at least size() bytes long.
				 * \param out_len The length of out.
				 * \return The number of bytes written to out.
				 */
				size_t to_bin(void* out, size_t out_len) const;

				/**
				 * \brief Get the binary representation of the BIGNUM.
				 * \return The binary representation.
				 */
				buffer to_bin() const;

				/**
				 * \brief Get the hexadecimal representation of the BIGNUM.
				 * \return The hexadecimal representation of the BIGNUM.
				 */
				std::string to_hex() const;

				/**
				 * \brief Get the decimal representation of the BIGNUM.
				 * \return The decimal representation of the BIGNUM.
				 */
				std::string to_dec() const;

				/**
				 * \brief Get a ASN1_INTEGER from this BIGNUM.
				 * \return An ASN1_INTEGER.
				 */
				asn1::integer to_integer() const;

				/**
				 * \brief Get the value as a long.
				 * \return A long or 0xFFFFFFFFL if the value cannot be represented as an unsigned long.
				 */
				unsigned long to_long() const;

				/**
				 * \brief Clone the BIGNUM.
				 * \return A new bignum.
				 */
				bignum clone() const;

				/**
				 * \brief Set the value of the BIGNUM.
				 * \param ul The unsigned long value.
				 */
				void set_value(unsigned long ul) const;

			private:

				explicit bignum(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two BIGNUM pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bignum instance share the same underlying pointer.
		 */
		bool operator==(const bignum& lhs, const bignum& rhs);

		/**
		 * \brief Compare two BIGNUM pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bignum instance do not share the same underlying pointer.
		 */
		bool operator!=(const bignum& lhs, const bignum& rhs);

		/**
		 * \brief Add two BIGNUM pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The result of the operation.
		 */
		bignum operator+(const bignum& lhs, const bignum& rhs);

		/**
		 * \brief Substract two BIGNUM pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return The result of the operation.
		 */
		bignum operator-(const bignum& lhs, const bignum& rhs);

		inline bignum bignum::create()
		{
			return take_ownership(BN_new());
		}
		inline bignum bignum::from_bin(const void* buf, size_t buf_len)
		{
			return take_ownership(BN_bin2bn(static_cast<const unsigned char*>(buf), static_cast<int>(buf_len), NULL));
		}
		inline bignum bignum::from_long(unsigned long ul)
		{
			bignum result = create();

			result.set_value(ul);

			return result;
		}
		inline bignum::bignum()
		{
		}
		inline bignum::bignum(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline const bignum& bignum::operator+=(const bignum& bn) const
		{
			throw_error_if_not(BN_add(ptr().get(), ptr().get(), bn.raw()) != 0);

			return *this;
		}
		inline bignum& bignum::operator+=(const bignum& bn)
		{
			throw_error_if_not(BN_add(ptr().get(), ptr().get(), bn.raw()) != 0);

			return *this;
		}
		inline const bignum& bignum::operator-=(const bignum& bn) const
		{
			throw_error_if_not(BN_sub(ptr().get(), ptr().get(), bn.raw()) != 0);

			return *this;
		}
		inline bignum& bignum::operator-=(const bignum& bn)
		{
			throw_error_if_not(BN_sub(ptr().get(), ptr().get(), bn.raw()) != 0);

			return *this;
		}
		inline void bignum::copy(const bignum& bn)
		{
			throw_error_if_not(BN_copy(ptr().get(), bn.raw()) != 0);
		}
		inline size_t bignum::size() const
		{
			return BN_num_bytes(ptr().get());
		}
		inline buffer bignum::to_bin() const
		{
			buffer result(size());

			result.data().resize(to_bin(buffer_cast<uint8_t*>(result), buffer_size(result)));

			return result;
		}
		inline unsigned long bignum::to_long() const
		{
			return static_cast<unsigned long>(BN_get_word(ptr().get()));
		}
		inline bignum bignum::clone() const
		{
			return take_ownership(BN_dup(ptr().get()));
		}
		inline void bignum::set_value(unsigned long ul) const
		{
			throw_error_if_not(BN_set_word(ptr().get(), ul) != 0);
		}
		inline bignum::bignum(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const bignum& lhs, const bignum& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const bignum& lhs, const bignum& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
		inline bignum operator+(const bignum& lhs, const bignum& rhs)
		{
			bignum result = bignum::create();

			BN_add(result.raw(), lhs.raw(), rhs.raw());

			return result;
		}
		inline bignum operator-(const bignum& lhs, const bignum& rhs)
		{
			bignum result = bignum::create();

			BN_sub(result.raw(), lhs.raw(), rhs.raw());

			return result;
		}
	}
}

#endif /* CRYPTOPLUS_BN_bignum_HPP */

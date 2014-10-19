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
 * \file bignum.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A BIGNUM pointer class.
 */

#include "bn/bignum.hpp"
#include "asn1/integer.hpp"

#include <openssl/crypto.h>

#include <boost/shared_ptr.hpp>

#include <cassert>
#include <stdexcept>

namespace cryptoplus
{
	template <>
	bn::bignum::deleter_type pointer_wrapper<bn::bignum::value_type>::deleter = BN_clear_free;

	namespace bn
	{
		namespace
		{
			void _OPENSSL_free(void* buf)
			{
				OPENSSL_free(buf);
			}
		}

		bignum bignum::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return bignum(_ptr, deleter);
		}

		bignum bignum::from_hex(const std::string& str)
		{
			BIGNUM* bn = NULL;

			throw_error_if_not(BN_hex2bn(&bn, str.c_str()) != 0);

			return take_ownership(bn);
		}

		bignum bignum::from_dec(const std::string& str)
		{
			BIGNUM* bn = NULL;

			throw_error_if_not(BN_dec2bn(&bn, str.c_str()) != 0);

			return take_ownership(bn);
		}

		bignum bignum::from_integer(asn1::integer i)
		{
			return take_ownership(ASN1_INTEGER_to_BN(i.raw(), NULL));
		}

		size_t bignum::to_bin(void* out, size_t out_len) const
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			return BN_bn2bin(ptr().get(), static_cast<unsigned char*>(out));
		}

		std::string bignum::to_hex() const
		{
			boost::shared_ptr<char> result(BN_bn2hex(ptr().get()), _OPENSSL_free);

			return std::string(result.get());
		}

		std::string bignum::to_dec() const
		{
			boost::shared_ptr<char> result(BN_bn2dec(ptr().get()), _OPENSSL_free);

			return std::string(result.get());
		}

		asn1::integer bignum::to_integer() const
		{
			return asn1::integer::from_bignum(*this);
		}
	}
}


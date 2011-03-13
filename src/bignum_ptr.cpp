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
 * \file bignum_ptr.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A bio pointer class.
 */

#include "bn/bignum_ptr.hpp"

#include "error/cryptographic_exception.hpp"

#include <openssl/crypto.h>

#include <boost/shared_ptr.hpp>

#include <cassert>
#include <stdexcept>

namespace cryptopen
{
	namespace bn
	{
		namespace
		{
			void _OPENSSL_free(void* buf)
			{
				OPENSSL_free(buf);
			}
		}

		size_t bignum_ptr::to_bin(void* out, size_t out_len) const
		{
			assert(out_len >= size());

			if (out_len < size())
			{
				throw std::invalid_argument("out_len");
			}

			return BN_bn2bin(m_bignum, static_cast<unsigned char*>(out));
		}
		
		void bignum_ptr::from_bin(const void* buf, size_t buf_len)
		{
			BIGNUM* result = BN_bin2bn(static_cast<const unsigned char*>(buf), buf_len, m_bignum);

			error::throw_error_if_not(result);

			m_bignum = result;
		}

		std::string bignum_ptr::to_hex() const
		{
			boost::shared_ptr<char> result(BN_bn2hex(m_bignum), _OPENSSL_free);

			return std::string(result.get());
		}
		
		std::string bignum_ptr::to_dec() const
		{
			boost::shared_ptr<char> result(BN_bn2dec(m_bignum), _OPENSSL_free);

			return std::string(result.get());
		}
	}
}


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
 * \file string.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_STRING pointer class.
 */

#include "asn1/string.hpp"

#include <boost/shared_ptr.hpp>

#include <cassert>

namespace cryptoplus
{
	template <>
	asn1::string::deleter_type pointer_wrapper<asn1::string::value_type>::deleter = ASN1_STRING_free;

	namespace asn1
	{
		namespace
		{
			void _OPENSSL_free(unsigned char* c)
			{
				OPENSSL_free(c);
			}
		}

		string string::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return string(_ptr, deleter);
		}

		std::string string::to_utf8() const
		{
			unsigned char* out = NULL;

			const int _size = ASN1_STRING_to_UTF8(&out, ptr().get());

			throw_error_if(_size < 0);

			const boost::shared_ptr<unsigned char> pout(out, _OPENSSL_free);

			return std::string(out, out + _size);
		}
	}
}


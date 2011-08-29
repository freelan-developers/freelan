/*
 * libiconvplus - A lightweight C++ wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file iconv.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv class.
 */

#include "iconv.hpp"

#include "iconv_error_category.hpp"

#include <cassert>

namespace iconvplus
{
	size_t iconv::convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft, boost::system::error_code& ec) const
	{
		size_t result = raw_convert(inbuf, inbytesleft, outbuf, outbytesleft);

		if (result == ERROR_VALUE)
		{
			ec = boost::system::error_code(errno, iconv_error_category());
		}

		return result;
	}
	
	size_t iconv::convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) const
	{
		boost::system::error_code ec;

		size_t result = convert(inbuf, inbytesleft, outbuf, outbytesleft, ec);

		if (result == ERROR_VALUE)
		{
			throw ec;
		}

		return result;
	}

	size_t iconv::convert_all(const void* in, size_t in_len, void* out, size_t out_len, size_t* non_reversible_conversions)
	{
		assert(in);
		assert(out);

		reset();

		const char* inbuf = static_cast<const char*>(in);
		char* outbuf = static_cast<char*>(out);

		size_t result = convert(&inbuf, &in_len, &outbuf, &out_len);

		if (non_reversible_conversions) *non_reversible_conversions = result;

		return (outbuf - static_cast<char*>(out));
	}
}

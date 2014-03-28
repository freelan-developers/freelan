/*
 * libiconvplus - A lightweight C++ wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libiconvplus.
 *
 * libiconvplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libiconvplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libiconvplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file iconv_error_category.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv error category.
 */

#include "iconv_error_category.hpp"

#include <boost/lexical_cast.hpp>

#include <cerrno>
#include <cstring>

namespace iconvplus
{
	std::string iconv_error_category::message(int ev) const
	{
		switch (ev)
		{
			case E2BIG:
				return "Output buffer is too small";
			case EILSEQ:
				return "An invalid multibyte sequence has been encountered in the input";
			case EINVAL:
				return "An incomplete multibyte sequence has been encountered in the input";
			default:
#ifdef _MSC_VER
				char error_buf[256] = {};
				strerror_s(error_buf, ev);
				const std::string error_string(error_buf, strnlen_s(error_buf, sizeof(error_buf)));

				return "Unknown error " + boost::lexical_cast<std::string>(ev) + ": " + error_string;
#else
				return "Unknown error " + boost::lexical_cast<std::string>(ev) + ": " + strerror(ev);
#endif
		}
	}

	const iconv_error_category iconv_error_category_instance;

	const iconv_error_category& iconv_category()
	{
		return iconv_error_category_instance;
	}
}

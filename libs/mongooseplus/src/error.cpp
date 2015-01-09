/*
 * libmongooseplus - A lightweight C++ wrapper around the libmongoose
 * library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libmongooseplus.
 *
 * libmongooseplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libmongooseplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libmongooseplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file error.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The errors.
 */

#include "error.hpp"

namespace mongooseplus
{
	const boost::system::error_category& mongooseplus_category()
	{
		static mongooseplus_category_impl instance;

		return instance;
	}

	const char* mongooseplus_category_impl::name() const throw()
	{
		return "mongooseplus::error";
	}

	std::string mongooseplus_category_impl::message(int ev) const
	{
		switch (static_cast<mongooseplus_error>(ev))
		{
			case mongooseplus_error::http_200_ok:
			{
				return "HTTP 200 OK";
			}
			case mongooseplus_error::http_204_no_content:
			{
				return "HTTP 204 NO CONTENT";
			}
			case mongooseplus_error::http_400_bad_request:
			{
				return "HTTP 400 BAD REQUEST";
			}
			case mongooseplus_error::http_401_unauthorized:
			{
				return "HTTP 401 UNAUTHORIZED";
			}
			case mongooseplus_error::http_405_method_not_allowed:
			{
				return "HTTP 405 METHOD NOT ALLOWED";
			}
			case mongooseplus_error::http_406_not_acceptable:
			{
				return "HTTP 406 NOT ACCEPTABLE";
			}
			default:
			{
				return "Unknown mongooseplus error";
			}
		}
	}
}

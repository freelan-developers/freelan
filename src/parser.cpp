/*
 * libkfather - C++ JSON parser/producer library.
 * Copyright (C) 2010-2012 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libkfather.
 *
 * libkfather is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libkfather is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * If you intend to use libkfather in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file kfather.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The global kfather include file implemenation.
 */

#include "parser.hpp"

#include <string>
#include <cctype>

namespace kfather
{
	bool parser::parse(const char* buf, size_t buflen, const char** error_token)
	{
		const char* const end = buf + buflen;
		const char* ch = buf;

		if (!parse_value(ch, end))
		{
			if (error_token)
			{
				*error_token = ch;
			}

			return false;
		}

		return true;
	}
		
	bool parser::parse_value(const char*& ch, const char* end)
	{
		if (!skip_whitespace(ch, end))
		{
			return false;
		}

		if (*ch != '{')
		{
			return false;
		}

		return true;
	}

	bool parser::skip_whitespace(const char*& ch, const char* end)
	{
		for (; (ch != end) && std::isspace(*ch); ++ch) {}

		return (ch != end);
	}
}

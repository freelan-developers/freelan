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
 * \file kfather.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The global kfather include file.
 */

#ifndef KFATHER_PARSER_HPP
#define KFATHER_PARSER_HPP

#include <boost/function.hpp>

namespace kfather
{
	/**
	 * \brief A JSON parser class.
	 *
	 * parser objects are thread-safe and reentrant in the sense that the same
	 * parser instance can be used to parse different strings (or even the same
	 * string) at the same time.
	 *
	 * Note however that modifying a parser instance while parsing has undefined
	 * behavior.
	 */
	class parser
	{
		public:

			/**
			 * \brief A JSON object start callback type.
			 */
			typedef boost::function<void ()> object_start_callback;

			/**
			 * \brief Parse the specified JSON string buffer.
			 * \param buf The buffer to parse. Must contain a valid JSON string or
			 * the call will fail.
			 * \param buflen The length of buf.
			 * \param error_token A pointer to a pointer to the first invalid
			 * character of buf, in case the parsing fails.
			 * \return true if the parsing succeeds, false otherwise.
			 *
			 * If the parsing fails, and error_token is not null, *error_token is set
			 * to the first invalid character in buf. That is, the position of the
			 * first invalid character of buf can be get by doing (error_token -
			 * buf).
			 *
			 * If the parsing succeeds, *error_token is guaranteed not to be
			 * modified.
			 */
			bool parse(const char* buf, size_t buflen, const char** error_token = NULL);

		private:

			bool parse_char(char c, const char*& ch, const char* end);
			bool parse_value(const char*& ch, const char* end);
			bool parse_object(const char*& ch, const char* end);
			bool parse_array(const char*& ch, const char* end);
			bool parse_string(const char*& ch, const char* end);
			bool parse_number(const char*& ch, const char* end);
			bool parse_true(const char*& ch, const char* end);
			bool parse_false(const char*& ch, const char* end);
			bool parse_null(const char*& ch, const char* end);
			void skip_whitespace(const char*& ch, const char* end);

			object_start_callback m_object_start_callback;
	};
}

#endif /* KFATHER_PARSER_HPP */

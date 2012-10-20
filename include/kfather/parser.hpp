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

			/**
			 * \brief Parse the specified JSON string.
			 * \param str The JSON string to parse. Must contain a valid JSON string or
			 * the call will fail.
			 * \param error_pos A pointer to a position of the first invalid character.
			 * \return true if the parsing succeeds, false otherwise.
			 *
			 * If the parsing fails, and error_pos is not null, *error_pos is set
			 * to the position of the first invalid character in str.
			 *
			 * If the parsing succeeds, *error_pos is guaranteed not to be
			 * modified.
			 */
			bool parse(const std::string& str, std::string::size_type* error_pos = NULL);

		private:

			template <typename IteratorType>
			bool parse_char(char c, IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_value(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_object(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_array(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_string(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_number(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_true(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_false(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			bool parse_null(IteratorType& ch, IteratorType end);
			template <typename IteratorType>
			void skip_whitespace(IteratorType& ch, IteratorType end);

			object_start_callback m_object_start_callback;
	};
}

#endif /* KFATHER_PARSER_HPP */

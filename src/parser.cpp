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
#include <streambuf>

namespace kfather
{
	parser::parser() :
		m_object_callback(),
		m_array_callback(),
		m_string_callback(),
		m_number_callback(),
		m_true_callback(),
		m_false_callback(),
		m_null_callback()
	{
	}

	bool parser::parse(const char* buf, size_t buflen, const char** error_token)
	{
		const char* const end = buf + buflen;
		const char* ch = buf;

		skip_whitespace(ch, end);

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
		
	bool parser::parse(const std::string& str, std::string::size_type* error_pos)
	{
		if (error_pos)
		{
			std::string::const_iterator error_it;

			if (!parse(str.begin(), str.end(), &error_it))
			{
				*error_pos = std::distance(str.begin(), error_it);

				return false;
			}

			return true;
		}
		else
		{
			 return parse(str.begin(), str.end());
		}
	}

	bool parser::parse(std::istream& is, size_t* error_pos)
	{
		const std::istreambuf_iterator<char> begin = std::istreambuf_iterator<char>(is);
		const std::istreambuf_iterator<char> end;

		if (error_pos)
		{
			std::istreambuf_iterator<char> error_it;

			if (!parse(begin, end, &error_it))
			{
				*error_pos = std::distance(begin, error_it);

				return false;
			}

			return true;
		}
		else
		{
			 return parse(begin, end);
		}
	}

	template <typename IteratorType>
	bool parser::parse(IteratorType begin, IteratorType end, IteratorType* error_pos)
	{
		IteratorType it = begin;

		skip_whitespace(it, end);

		if (!parse_value(it, end))
		{
			if (error_pos)
			{
				*error_pos = it;
			}

			return false;
		}

		return true;
	}

	template <typename IteratorType>
	bool parser::parse_char(char c, IteratorType& ch, IteratorType end)
	{
		if ((ch != end) && (*ch == c))
		{
			++ch;

			return true;
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_value(IteratorType& ch, IteratorType end)
	{
		if (ch != end)
		{
			switch (*ch)
			{
				case '{':
					return parse_object(ch, end);

				case '[':
					return parse_array(ch, end);

				case '"':
					return parse_string(ch, end);

				case 't':
					return parse_true(ch, end);

				case 'f':
					return parse_false(ch, end);

				case 'n':
					return parse_null(ch, end);
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_object(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('{', ch, end))
		{
			return false;
		}

		skip_whitespace(ch, end);

		if (ch != end)
		{
			if (*ch == '}')
			{
				++ch;

				// We have an empty object.

				return true;
			}

			while (ch != end)
			{
				if (!parse_string(ch, end))
				{
					return false;
				}

				skip_whitespace(ch, end);

				if (!parse_char(':', ch, end))
				{
					return false;
				}

				skip_whitespace(ch, end);

				if (!parse_value(ch, end))
				{
					return false;
				}

				skip_whitespace(ch, end);

				if (ch != end)
				{
					if (*ch == '}')
					{
						++ch;

						// Object is over.

						return true;
					}

					if (*ch != ',')
					{
						return false;
					}

					++ch;

					skip_whitespace(ch, end);
				}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_array(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('[', ch, end))
		{
			return false;
		}

		skip_whitespace(ch, end);

		if (ch != end)
		{
			if (*ch == ']')
			{
				++ch;

				// We have an empty array.

				return true;
			}

			while (ch != end)
			{
				skip_whitespace(ch, end);

				if (!parse_value(ch, end))
				{
					return false;
				}

				skip_whitespace(ch, end);

				if (ch != end)
				{
					if (*ch == ']')
					{
						++ch;

						// Array is over.

						return true;
					}

					if (*ch != ',')
					{
						return false;
					}

					++ch;

					skip_whitespace(ch, end);
				}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_string(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('"', ch, end))
		{
			return false;
		}

		while (ch != end)
		{
			if (std::iscntrl(*ch))
			{
				return false;
			}

			switch (*ch)
			{
				case '"':
					{
						// The string ends.
						++ch;
						return true;
					}

				case '\\':
					{
						// An escape character was provided.
						++ch;

						if (ch == end)
						{
							return false;
						}

						switch (*ch)
						{
							case '"':
								break;

							case '\\':
								break;

							case '/':
								break;

							case 'b':
								break;

							case 'n':
								break;

							case 'f':
								break;

							case 'r':
								break;

							case 't':
								break;

							case 'u':
								{
									for (size_t i = 0; i < 4; ++i)
									{
										if (ch == end)
										{
											return false;
										}

										if (!std::isxdigit(*ch))
										{
											return false;
										}

										++ch;
									}

									break;
								}

							default:
								return false;
						}

						++ch;

						break;
					}

				default:
					{
						++ch;
					}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_number(IteratorType& ch, IteratorType end)
	{
		// Check if the number is negative
		if (ch != end)
		{
			if (*ch == '-')
			{
				++ch;
			}
		}

		if (ch == end)
		{
			return false;
		}

		switch (*ch)
		{
			case '0':
				++ch;
				break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				++ch;

				while ((ch != end) && std::isdigit(*ch))
				{
					++ch;
				}

				break;
		}

		// Check for a period
		if (ch != end)
		{
			if (*ch == '.')
			{
				++ch;

				while ((ch != end) && std::isdigit(*ch))
				{
					++ch;
				}
			}
		}

		// Check for an exponent
		if (ch != end)
		{
			if ((*ch == 'e') || (*ch == 'E'))
			{
				++ch;

				if (ch == end)
				{
					return false;
				}

				switch (*ch)
				{
					case '-':
						++ch;
						break;

					case '+':
						++ch;
						break;
				}

				if (ch == end)
				{
					return false;
				}

				while ((ch != end) && std::isdigit(*ch))
				{
					++ch;
				}
			}
		}

		return true;
	}

	template <typename IteratorType>
	bool parser::parse_true(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('t', ch, end))
		{
			return false;
		}

		if (!parse_char('r', ch, end))
		{
			return false;
		}

		if (!parse_char('u', ch, end))
		{
			return false;
		}

		if (!parse_char('e', ch, end))
		{
			return false;
		}

		return true;
	}

	template <typename IteratorType>
	bool parser::parse_false(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('f', ch, end))
		{
			return false;
		}

		if (!parse_char('a', ch, end))
		{
			return false;
		}

		if (!parse_char('l', ch, end))
		{
			return false;
		}

		if (!parse_char('s', ch, end))
		{
			return false;
		}

		if (!parse_char('e', ch, end))
		{
			return false;
		}

		return true;
	}

	template <typename IteratorType>
	bool parser::parse_null(IteratorType& ch, IteratorType end)
	{
		if (!parse_char('n', ch, end))
		{
			return false;
		}

		if (!parse_char('u', ch, end))
		{
			return false;
		}

		if (!parse_char('l', ch, end))
		{
			return false;
		}

		if (!parse_char('l', ch, end))
		{
			return false;
		}

		return true;
	}

	template <typename IteratorType>
	void parser::skip_whitespace(IteratorType& ch, IteratorType end)
	{
		for (; (ch != end) && std::isspace(*ch); ++ch) {}
	}
}

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
 * \file parser.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A JSON parser class.
 */

#include "parser.hpp"

#include <string>
#include <sstream>
#include <cctype>
#include <cassert>
#include <streambuf>

namespace kfather
{
	namespace
	{
		int xdigit_to_int(char c)
		{
			static const int digit_map[256] =
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};

			return digit_map[static_cast<uint8_t>(c)];
		}
	}

	bool parser::parse(value_type& value, const char* buf, size_t buflen, const char** error_token)
	{
		assert(buf);

		const char* const end = buf + buflen;
		const char* ch = buf;

		context ctx;

		skip_whitespace(ctx, ch, end);

		if (!parse_value(ctx, value, ch, end))
		{
			if (error_token)
			{
				*error_token = ch;
			}

			return false;
		}

		return true;
	}

	bool parser::parse(value_type& value, const std::string& str, std::string::size_type* error_pos)
	{
		if (error_pos)
		{
			std::string::const_iterator error_it;

			if (!parse(value, str.begin(), str.end(), &error_it))
			{
				*error_pos = std::distance(str.begin(), error_it);

				return false;
			}

			return true;
		}
		else
		{
			return parse(value, str.begin(), str.end());
		}
	}

	bool parser::parse(value_type& value, std::istream& is, size_t* error_pos)
	{
		const std::istreambuf_iterator<char> begin = std::istreambuf_iterator<char>(is);
		const std::istreambuf_iterator<char> end;

		if (error_pos)
		{
			std::istreambuf_iterator<char> error_it;

			if (!parse(value, begin, end, &error_it))
			{
				*error_pos = static_cast<size_t>(std::distance(begin, error_it));

				return false;
			}

			return true;
		}
		else
		{
			return parse(value, begin, end);
		}
	}

	template <typename IteratorType>
	bool parser::parse(value_type& value, IteratorType begin, IteratorType end, IteratorType* error_pos)
	{
		IteratorType it = begin;

		context ctx;

		skip_whitespace(ctx, it, end);

		if (!parse_value(ctx, value, it, end))
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
	bool parser::parse_value(context& ctx, value_type& value, IteratorType& ch, IteratorType end)
	{
		bool result = false;

		if (ch != end)
		{
			switch (*ch)
			{
				case '{':
					{
						object_type object;

						result = parse_object(ctx, object, ch, end);

						if (result) { value = object; }

						break;
					}

				case '[':
					{
						array_type array;

						result = parse_array(ctx, array, ch, end);

						if (result) { value = array; }

						break;
					}

				case '"':
					{
						string_type str;

						result = parse_string(ctx, str, ch, end);

						if (result) { value = str; }

						break;
					}

				case 't':
				case 'f':
					{

						boolean_type bt;

						result = parse_boolean(ctx, bt, ch, end);

						if (result) { value = bt; }

						break;
					}

				case 'n':
					{
						null_type nt;

						result = parse_null(ctx, nt, ch, end);

						if (result) { value = nt; }

						break;
					}

				default:
					{
						number_type nb;

						result = parse_number(ctx, nb, ch, end);

						if (result) { value = nb; }

						break;
					}
			}
		}

		return result;
	}

	template <typename IteratorType>
	bool parser::parse_object(context& ctx, object_type& value, IteratorType& ch, IteratorType end)
	{
		if (!check_char(ctx, '{', ch, end))
		{
			return false;
		}

		skip_whitespace(ctx, ch, end);

		if (ch != end)
		{
			object_type object;

			// Do we have an empty object ?
			if (*ch == '}')
			{
				++ch;

				value = object;

				return true;
			}

			while (ch != end)
			{
				string_type key;

				if (!parse_string(ctx, key, ch, end))
				{
					return false;
				}

				skip_whitespace(ctx, ch, end);

				if (!check_char(ctx, ':', ch, end))
				{
					return false;
				}

				skip_whitespace(ctx, ch, end);

				if (!parse_value(ctx, object.items[key], ch, end))
				{
					return false;
				}

				skip_whitespace(ctx, ch, end);

				if (ch != end)
				{
					if (*ch == '}')
					{
						++ch;

						value = object;

						return true;
					}

					if (*ch != ',')
					{
						return false;
					}

					++ch;

					skip_whitespace(ctx, ch, end);
				}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_array(context& ctx, array_type& value, IteratorType& ch, IteratorType end)
	{
		if (!check_char(ctx, '[', ch, end))
		{
			return false;
		}

		skip_whitespace(ctx, ch, end);

		if (ch != end)
		{
			array_type array;

			// Do we have an empty array.
			if (*ch == ']')
			{
				++ch;

				value = array;

				return true;
			}

			while (ch != end)
			{
				skip_whitespace(ctx, ch, end);

				array.items.resize(array.items.size() + 1);

				if (!parse_value(ctx, array.items[array.items.size() - 1], ch, end))
				{
					return false;
				}

				skip_whitespace(ctx, ch, end);

				if (ch != end)
				{
					if (*ch == ']')
					{
						++ch;

						value = array;

						return true;
					}

					if (*ch != ',')
					{
						return false;
					}

					++ch;

					skip_whitespace(ctx, ch, end);
				}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_string(context& ctx, string_type& value, IteratorType& ch, IteratorType end)
	{
		ctx.clear();

		if (!check_char(ctx, '"', ch, end))
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

						value = ctx.str();

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
								ctx.push_char('"');
								++ch;
								break;

							case '\\':
								ctx.push_char('\\');
								++ch;
								break;

							case '/':
								ctx.push_char('/');
								++ch;
								break;

							case 'b':
								ctx.push_char('\b');
								++ch;
								break;

							case 'n':
								ctx.push_char('\n');
								++ch;
								break;

							case 'f':
								ctx.push_char('\f');
								++ch;
								break;

							case 'r':
								ctx.push_char('\r');
								++ch;
								break;

							case 't':
								ctx.push_char('\t');
								++ch;
								break;

							case 'u':
								{
									++ch;

									uint16_t codepoint = 0x0000;

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

										codepoint *= 16;
										codepoint += xdigit_to_int(*ch);

										++ch;
									}

									ctx.push_codepoint(codepoint);

									break;
								}

							default:
								return false;
						}

						break;
					}

				default:
					{
						ctx.push_char(*ch);

						++ch;
					}
			}
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_number(context& ctx, number_type& value, IteratorType& ch, IteratorType end)
	{
		ctx.clear();

		// Check if the number is negative
		if (ch != end)
		{
			if (*ch == '-')
			{
				ctx.push_char(*ch);
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
				ctx.push_char(*ch);
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
				ctx.push_char(*ch);
				++ch;

				while ((ch != end) && std::isdigit(*ch))
				{
					ctx.push_char(*ch);
					++ch;
				}

				break;
		}

		// Check for a period
		if (ch != end)
		{
			if (*ch == '.')
			{
				ctx.push_char(*ch);
				++ch;

				while ((ch != end) && std::isdigit(*ch))
				{
					ctx.push_char(*ch);
					++ch;
				}
			}
		}

		// Check for an exponent
		if (ch != end)
		{
			if ((*ch == 'e') || (*ch == 'E'))
			{
				ctx.push_char(*ch);
				++ch;

				if (ch == end)
				{
					return false;
				}

				switch (*ch)
				{
					case '-':
						ctx.push_char(*ch);
						++ch;
						break;

					case '+':
						ctx.push_char(*ch);
						++ch;
						break;
				}

				if (ch == end)
				{
					return false;
				}

				while ((ch != end) && std::isdigit(*ch))
				{
					ctx.push_char(*ch);
					++ch;
				}
			}
		}

		if (!ctx.get_number(value))
		{
			return false;
		}

		return true;
	}

	template <typename IteratorType>
	bool parser::parse_boolean(context& ctx, boolean_type& value, IteratorType& ch, IteratorType end)
	{
		if (check_chars(ctx, "true", ch, end))
		{
			value = true;

			return true;
		}

		if (check_chars(ctx, "false", ch, end))
		{
			value = false;

			return true;
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::parse_null(context& ctx, null_type& value, IteratorType& ch, IteratorType end)
	{
		if (check_chars(ctx, "null", ch, end))
		{
			value = null_type();

			return true;
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::check_char(context&, char c, IteratorType& ch, IteratorType end)
	{
		if ((ch != end) && (*ch == c))
		{
			++ch;

			return true;
		}

		return false;
	}

	template <typename IteratorType>
	bool parser::check_chars(context&, const char* str, IteratorType& ch, IteratorType end)
	{
		assert(str);

		for (const char* c = str; *c != '\0'; ++c, ++ch)
		{
			if ((ch == end) || (*ch != *c))
			{
				return false;
			}
		}

		return true;
	}
	template <typename IteratorType>
	void parser::skip_whitespace(context&, IteratorType& ch, IteratorType end)
	{
		for (; (ch != end) && std::isspace(*ch); ++ch) {}
	}

	parser::context::context() :
		m_iconv("utf-8", "utf-16be")
	{
	}
	
	void parser::context::clear()
	{
		m_str.clear();
		m_utf16.clear();
	}

	void parser::context::push_char(char c)
	{
		end_codepoints();

		m_str.push_back(c);
	}

	void parser::context::push_codepoint(uint16_t cp)
	{
		m_utf16.push_back(static_cast<char>(cp / 256));
		m_utf16.push_back(static_cast<char>(cp % 256));
	}

	const std::string& parser::context::str()
	{
		end_codepoints();

		return m_str;
	}

	bool parser::context::get_number(number_type& value)
	{
		std::istringstream iss(m_str);

		m_str.clear();

		iss >> value;

		return !iss.fail();
	}

	void parser::context::end_codepoints()
	{
		if (!m_utf16.empty())
		{
			const size_t initial_size = m_str.size();

			// This is enough to hold all possible UTF-8 encoded characters.
			m_str.resize(m_str.size() + m_utf16.size() * 2);

			size_t wrote_cnt = m_iconv.convert_all(m_utf16.c_str(), m_utf16.size(), &m_str[initial_size], m_str.size() - initial_size);

			m_str.resize(initial_size + wrote_cnt);

			m_utf16.clear();
		}
	}
}

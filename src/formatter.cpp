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
 * \file formatter.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Formatter classes.
 */

#include "formatter.hpp"

#include "parser.hpp"

#include <boost/bind.hpp>

namespace kfather
{
	std::ostream& base_formatter_visitor::operator()(const null_type&) const
	{
		return os() << "null";
	}

	std::ostream& base_formatter_visitor::operator()(const boolean_type& bt) const
	{
		return os() << (bt ? "true" : "false");
	}

	std::ostream& base_formatter_visitor::operator()(const number_type& nb) const
	{
		return os() << nb;
	}

	std::ostream& base_formatter_visitor::operator()(const string_type& str) const
	{
		os() << '"';

		for (string_type::const_iterator it = str.begin(); it != str.end(); ++it)
		{
			switch (*it)
			{
				case '"':
					os() << "\\\"";
					break;

				case '\\':
					os() << "\\\\";
					break;

				case '/':
					os() << "\\/";
					break;

				case '\b':
					os() << "\\b";
					break;

				case '\n':
					os() << "\\n";
					break;

				case '\f':
					os() << "\\f";
					break;

				case '\r':
					os() << "\\r";
					break;

				case '\t':
					os() << "\\t";
					break;

				default:
					os() << *it;
			}
		}

		return os() << '"';
	}

	std::ostream& compact_formatter_visitor::operator()(const array_type& ar) const
	{
		os() << "[";

		if (!ar.items.empty())
		{
			boost::apply_visitor(*this, ar.items.front());

			for (array_type::items_type::const_iterator it = ar.items.begin() + 1; it != ar.items.end(); ++it)
			{
				os() << ",";

				boost::apply_visitor(*this, *it);
			}
		}

		return os() << "]";
	}

	std::ostream& compact_formatter_visitor::operator()(const object_type& obj) const
	{
		os() << "{";

		if (!obj.items.empty())
		{
			object_type::items_type::const_iterator it = obj.items.begin();

			(*this)(it->first);

			os() << ":";

			boost::apply_visitor(*this, it->second);

			++it;

			for (; it != obj.items.end(); ++it)
			{
				os() << ",";

				(*this)(it->first);

				os() << ":";

				boost::apply_visitor(*this, it->second);
			}
		}

		return os() << "}";
	}

	std::ostream& inline_formatter_visitor::operator()(const array_type& ar) const
	{
		os() << "[";

		if (!ar.items.empty())
		{
			boost::apply_visitor(*this, ar.items.front());

			for (array_type::items_type::const_iterator it = ar.items.begin() + 1; it != ar.items.end(); ++it)
			{
				os() << ", ";

				boost::apply_visitor(*this, *it);
			}
		}

		return os() << "]";
	}

	std::ostream& inline_formatter_visitor::operator()(const object_type& obj) const
	{
		os() << "{";

		if (!obj.items.empty())
		{
			object_type::items_type::const_iterator it = obj.items.begin();

			(*this)(it->first);

			os() << ": ";

			boost::apply_visitor(*this, it->second);

			++it;

			for (; it != obj.items.end(); ++it)
			{
				os() << ", ";

				(*this)(it->first);

				os() << ": ";

				boost::apply_visitor(*this, it->second);
			}
		}

		return os() << "}";
	}

	std::ostream& pretty_print_formatter_visitor::operator()(const array_type& ar) const
	{
		os() << "[";

		if (!ar.items.empty())
		{
			os() << "\n";
			m_indent_level++;
			indent();

			boost::apply_visitor(*this, ar.items.front());

			for (array_type::items_type::const_iterator it = ar.items.begin() + 1; it != ar.items.end(); ++it)
			{
				os() << ",\n";
				indent();

				boost::apply_visitor(*this, *it);
			}

			os() << "\n";
			m_indent_level--;
			indent();
		}

		return os() << "]";
	}

	std::ostream& pretty_print_formatter_visitor::operator()(const object_type& obj) const
	{
		os() << "{";

		if (!obj.items.empty())
		{
			os() << "\n";
			m_indent_level++;
			indent();

			object_type::items_type::const_iterator it = obj.items.begin();

			(*this)(it->first);

			os() << ": ";

			boost::apply_visitor(*this, it->second);

			++it;

			for (; it != obj.items.end(); ++it)
			{
				os() << ",\n";
				indent();

				(*this)(it->first);

				os() << ": ";

				boost::apply_visitor(*this, it->second);
			}

			os() << "\n";
			m_indent_level--;
			indent();
		}

		return os() << "}";
	}

	std::ostream& pretty_print_formatter_visitor::indent() const
	{
		for (unsigned int i = 0; i < m_indent_level; ++i)
		{
			os() << "  ";
		}

		return os();
	}
}

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
 * \file formatter.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Formatter classes.
 */

#ifndef KFATHER_FORMATTER_HPP
#define KFATHER_FORMATTER_HPP

#include <iostream>
#include <sstream>

#include "value.hpp"

namespace kfather
{
	class parser;

	/**
	 * \brief A base formatter class.
	 *
	 * The same formatter can be used to format JSON values in a thread-safe and
	 * reentrant manner.
	 */
	template <typename FormatterVisitorType>
	class generic_formatter
	{
		public:

			/**
			 * \brief Format the specified value to the specified stream.
			 * \param os The stream to format the value to.
			 * \param value The value to format.
			 * \return os.
			 */
			std::ostream& format(std::ostream& os, const value_type& value) const
			{
				return boost::apply_visitor(FormatterVisitorType(os), value);
			}

			/**
			 * \brief Format a value to a string.
			 * \param value The value to format.
			 * \return The formatted string.
			 *
			 * If your intent is to print a value to an output stream, use the first
			 * format() overload instead as it will be far more efficient.
			 */
			std::string format(const value_type& value) const
			{
				std::ostringstream oss;

				format(oss, value);

				return oss.str();
			}
	};

	/**
	 * \brief The base formatter visitor class.
	 */
	class base_formatter_visitor : public boost::static_visitor<std::ostream&>
	{
		public:

			/**
			 * \brief Prints a null value.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const null_type&) const;

			/**
			 * \brief Prints a boolean value.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const boolean_type&) const;

			/**
			 * \brief Prints a number.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const number_type&) const;

			/**
			 * \brief Prints a string.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const string_type&) const;

		protected:

			/**
			 * \brief Construct a formatter visitor.
			 * \param _os The output stream to hold a reference to.
			 */
			base_formatter_visitor(std::ostream& _os) : m_os(_os) {}

			/**
			 * \brief Get the referenced output stream.
			 * \return The referenced output stream instance.
			 */
			std::ostream& os() const { return m_os; }

		private:

			std::ostream& m_os;
	};

	/**
	 * \brief A formatter visitor that compact its values.
	 */
	class compact_formatter_visitor : public base_formatter_visitor
	{
		public:

			/**
			 * \brief Construct a formatter visitor.
			 * \param _os The output stream to hold a reference to.
			 */
			compact_formatter_visitor(std::ostream& _os) : base_formatter_visitor(_os) {}

			/**
			 * \brief Prints an array.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const array_type&) const;

			/**
			 * \brief Prints an object.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const object_type&) const;

			using base_formatter_visitor::operator();
	};

	/**
	 * \brief A formatter visitor that inlines its values.
	 */
	class inline_formatter_visitor : public base_formatter_visitor
	{
		public:

			/**
			 * \brief Construct a formatter visitor.
			 * \param _os The output stream to hold a reference to.
			 */
			inline_formatter_visitor(std::ostream& _os) : base_formatter_visitor(_os) {}

			/**
			 * \brief Prints an array.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const array_type&) const;

			/**
			 * \brief Prints an object.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const object_type&) const;

			using base_formatter_visitor::operator();
	};

	/**
	 * \brief A formatter visitor that pretty prints its values.
	 */
	class pretty_print_formatter_visitor : public base_formatter_visitor
	{
		public:

			/**
			 * \brief Construct a formatter visitor.
			 * \param _os The output stream to hold a reference to.
			 */
			pretty_print_formatter_visitor(std::ostream& _os) : base_formatter_visitor(_os), m_indent_level(0) {}

			/**
			 * \brief Prints an array.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const array_type&) const;

			/**
			 * \brief Prints an object.
			 * \return The referenced output stream.
			 */
			std::ostream& operator()(const object_type&) const;

			using base_formatter_visitor::operator();

		private:

			std::ostream& indent() const;

			mutable unsigned int m_indent_level;
	};

	/**
	 * \brief A compact formatter class.
	 */
	typedef generic_formatter<compact_formatter_visitor> compact_formatter;

	/**
	 * \brief An inline formatter class.
	 */
	typedef generic_formatter<inline_formatter_visitor> inline_formatter;

	/**
	 * \brief A pretty-print formatter class.
	 */
	typedef generic_formatter<pretty_print_formatter_visitor> pretty_print_formatter;
}

#endif /* KFATHER_FORMATTER_HPP */

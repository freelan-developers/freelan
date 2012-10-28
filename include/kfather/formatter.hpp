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

#include "value.hpp"

namespace kfather
{
	class parser;

	class formatter
	{
		public:

			/**
			 * \brief Create a formatter bound to the specified parser.
			 * \param parser The parser to bind to.
			 * \param os The output stream to use.
			 *
			 * The binding process may replace some or all of the callbacks of the
			 * parser instance. That is, you cannot use the same parser with
			 * different formatters simultaneously.
			 */
			formatter(parser& parser, std::ostream& os);

		private:

			void print_string(const string_type&);
			void print_boolean(const boolean_type&);
			void print_null(const null_type&);
			void print_object_start();
			void print_object_colon();
			void print_object_comma();
			void print_object_stop();
			void print_array_start();
			void print_array_comma();
			void print_array_stop();

			std::ostream& m_os;
	};
}

#endif /* KFATHER_FORMATTER_HPP */

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
 * \file value.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Value classes.
 */

#ifndef KFATHER_VALUE_HPP
#define KFATHER_VALUE_HPP

#include <string>

#include <boost/variant.hpp>

namespace kfather
{
	/**
	 * \brief The string type.
	 *
	 * Content must be UTF-8 encoded.
	 */
	typedef std::string string_type;

	/**
	 * \brief The generic value type.
	 */
	typedef boost::variant<string_type> value_type;

	/**
	 * \brief The string_type visitor.
	 */
	class string_type_visitor : public boost::static_visitor<string_type>
	{
		public:

			const string_type& operator()(const string_type& str) const
			{
				return str;
			}
	};
}

#endif /* KFATHER_VALUE_HPP */

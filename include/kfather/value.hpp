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
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace kfather
{
	class object_type;
	class array_type;

	/**
	 * \brief The string type.
	 *
	 * Assumed to be UTF-8 encoded.
	 */
	typedef std::string string_type;

	/**
	 * \brief A number.
	 */
	typedef double number_type;

	/**
	 * \brief A boolean value.
	 */
	typedef bool boolean_type;

	/**
	 * \brief The null type.
	 */
	class null_type {};

	/**
	 * \brief The generic value type.
	 */
	typedef boost::variant<null_type, boolean_type, number_type, string_type, array_type, object_type> value_type;

	/**
	 * \brief The array type.
	 */
	class array_type
	{
		public:

			typedef std::vector<value_type> list_type;

		private:

			list_type m_list;
	};

	/**
	 * \brief Output the array_type to a stream.
	 * \param os The output stream.
	 * \param ar The array.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const array_type& ar);

	/**
	 * \brief The object type.
	 */
	class object_type
	{
		public:

			typedef std::map<string_type, value_type> dict_type;

		private:

			dict_type m_dict;
	};

	/**
	 * \brief Output the object_type to a stream.
	 * \param os The output stream.
	 * \param obj The object.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const object_type& obj);

	/**
	 * \brief The generic visitor type.
	 * \tparam Type The visitor type.
	 */
	template <typename Type>
	class visitor;

	/**
	 * \brief The string_type visitor.
	 */
	template <>
	class visitor<string_type> : public boost::static_visitor<string_type>
	{
		public:

			string_type operator()(const null_type&) const
			{
				return "null";
			}

			string_type operator()(const boolean_type& bt) const
			{
				return bt ? "true" : "false";
			}

			string_type operator()(const number_type& nb) const
			{
				std::ostringstream oss;

				oss << nb;

				return oss.str();
			}

			const string_type& operator()(const string_type& str) const
			{
				return str;
			}

			string_type operator()(const array_type&) const
			{
				return "";
			}

			string_type operator()(const object_type&) const
			{
				return "[object Object]";
			}
	};

	/**
	 * \brief The number_type visitor.
	 */
	template <>
	class visitor<number_type> : public boost::static_visitor<number_type>
	{
		public:

			number_type operator()(const null_type&) const
			{
				return 0;
			}

			number_type operator()(const boolean_type& bt) const
			{
				return bt ? 1 : 0;
			}

			number_type operator()(const number_type& nb) const
			{
				return nb;
			}

			number_type operator()(const string_type& str) const
			{
				try
				{
					return boost::lexical_cast<number_type>(str);
				}
				catch (boost::bad_lexical_cast&)
				{
					return std::numeric_limits<number_type>::quiet_NaN();
				}
			}

			number_type operator()(const array_type&) const
			{
				return 0;
			}

			number_type operator()(const object_type&) const
			{
				return std::numeric_limits<number_type>::quiet_NaN();
			}
	};

	/**
	 * \brief The boolean_type visitor.
	 */
	template <>
	class visitor<boolean_type> : public boost::static_visitor<boolean_type>
	{
		public:

			number_type operator()(const null_type&) const
			{
				return false;
			}

			boolean_type operator()(const boolean_type& bt) const
			{
				return bt;
			}

			boolean_type operator()(const number_type& nb) const
			{
				return (nb != 0) && (nb != std::numeric_limits<number_type>::quiet_NaN());
			}

			boolean_type operator()(const string_type& str) const
			{
				return !str.empty();
			}

			boolean_type operator()(const array_type&) const
			{
				return true;
			}

			boolean_type operator()(const object_type&) const
			{
				return true;
			}
	};

	/**
	 * \brief Checks if a specified value is a falsy value.
	 * \param v The value.
	 * \return true if the value is falsy, according to the Javascript rules.
	 */
	inline bool is_falsy(const value_type& v)
	{
		return !boost::apply_visitor(visitor<boolean_type>(), v);
	}
}

#endif /* KFATHER_VALUE_HPP */

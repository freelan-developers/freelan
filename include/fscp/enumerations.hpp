/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file enumerations.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The enumerations routines.
 */

#ifndef FSCP_ENUMERATIONS_HPP
#define FSCP_ENUMERATIONS_HPP

#include <boost/asio.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>

namespace fscp
{
	namespace detail
	{
		template <template <uint8_t> class ValueType, uint8_t Value>
			struct validity_checker;

		template <template <uint8_t> class ValueType, uint8_t Value>
			inline bool recursive_is_valid(typename boost::enable_if<ValueType<Value>, uint8_t>::type value)
			{
				return (value == Value) || validity_checker<ValueType, Value - 1>::is_valid(value);
			}

		template <template <uint8_t> class ValueType, uint8_t Value>
			inline bool recursive_is_valid(typename boost::disable_if<ValueType<Value>, uint8_t>::type value)
			{
				return (value != Value) && validity_checker<ValueType, Value - 1>::is_valid(value);
			}

		template <template <uint8_t> class ValueType>
			inline bool final_is_valid(typename boost::enable_if<ValueType<0>, uint8_t>::type value)
			{
				return (value == 0);
			}

		template <template <uint8_t> class ValueType>
			inline bool final_is_valid(typename boost::disable_if<ValueType<0>, uint8_t>::type value)
			{
				return false;
			}

		template <template <uint8_t> class ValueType, uint8_t Value>
			struct validity_checker
			{
				static bool is_valid(uint8_t value)
				{
					return recursive_is_valid<ValueType, Value>(value);
				}
			};

		template <template <uint8_t> class ValueType>
			struct validity_checker<ValueType, 0>
			{
				static bool is_valid(uint8_t value)
				{
					return final_is_valid<ValueType>(value);
				}
			};
	}

	/**
	 * \brief Check the validity of a value.
	 * \tparam ValueType The value type.
	 * \param value The value to check.
	 * \return true if the value is valid.
	 */
	template <template <uint8_t> class ValueType>
	inline bool check_validity(uint8_t value)
	{
		return detail::validity_checker<ValueType, 255>::is_valid(value);
	}

	/**
	 * \brief The invalid value type.
	 */
	typedef boost::false_type invalid_value_type;

	/**
	 * \brief The valid value type.
	 */
	typedef boost::true_type valid_value_type;

	/**
	 * \brief A base class for all types that must implement the value type
	 * mechanism.
	 */
	template <typename ParentType>
	struct value_type
	{
		public:

			/**
			 * \brief The value type.
			 * \tparam Value The specialized value.
			 */
			template <uint8_t Value>
			struct raw_value_type : public invalid_value_type {};

			/**
			 * \brief Create a new value type.
			 * \param _value The native value type.
			 */
			value_type(uint8_t _value) :
				m_value(_value)
			{
			}

			/**
			 * \brief Get the native value type.
			 * \return The native value type.
			 */
			uint8_t value() const
			{
				return m_value;
			}

			/**
			 * \brief Conversion to the native value type.
			 * \return The native value type.
			 */
			operator uint8_t() const
			{
				return m_value;
			}

			/**
			 * \brief Check whether the value is valid.
			 * \return true if the value is valid.
			 */
			bool is_valid() const
			{
				return fscp::check_validity<raw_value_type>(m_value);
			}

		private:

			uint8_t m_value;
	};

	template <typename ParentType>
	inline bool operator==(const value_type<ParentType>& lhs, const value_type<ParentType>& rhs)
	{
		return (lhs.value() == rhs.value());
	}

	template <typename ParentType>
	inline bool operator!=(const value_type<ParentType>& lhs, const value_type<ParentType>& rhs)
	{
		return (lhs.value() != rhs.value());
	}

	#define DECLARE_VALUE_TYPE(name) struct name : public fscp::value_type<name> { name() : fscp::value_type<name>(0) {} name(uint8_t _value) : fscp::value_type<name>(_value) {} }
	#define DECLARE_VALUE(parentType,value,name,str) template <> template <> struct parentType::raw_value_type<value> : public fscp::valid_value_type {}; const uint8_t name = value
}

#endif /* FSCP_ENUMERATIONS_HPP */

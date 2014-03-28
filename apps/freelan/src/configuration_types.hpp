/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file configuration_types.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Configuration related types.
 */

#ifndef CONFIGURATION_TYPES_HPP
#define CONFIGURATION_TYPES_HPP

#include <iostream>
#include <iomanip>

#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * \brief A duration in milliseconds.
 */
class millisecond_duration
{
	public:

		/**
		 * \brief Create a null duration.
		 */
		millisecond_duration() : m_ms() {}

		/**
		 * \brief Create a millisecond duration.
		 * \param ms The millisecond count.
		 */
		millisecond_duration(unsigned int ms) : m_ms(ms) {}

		/**
		 * \brief Create a millisecond duration.
		 * \param duration The duration.
		 */
		millisecond_duration(const boost::posix_time::time_duration& duration) : m_ms(static_cast<unsigned int>(duration.total_milliseconds())) {}

		/**
		 * \brief Conversion operator.
		 * \return The converted value.
		 */
		unsigned int to_int() const
		{
			return m_ms;
		}

		/**
		 * \brief Conversion operator.
		 * \return The converted value.
		 */
		boost::posix_time::time_duration to_time_duration() const
		{
			return boost::posix_time::milliseconds(m_ms);
		}

	private:

		unsigned int m_ms;

		friend std::ostream& operator<<(std::ostream&, const millisecond_duration&);
		friend std::istream& operator>>(std::istream&, millisecond_duration&);
};

inline std::ostream& operator<<(std::ostream& os, const millisecond_duration& value)
{
	return os << std::dec << value.m_ms;
}
inline std::istream& operator>>(std::istream& is, millisecond_duration& value)
{
	return is >> std::dec >> value.m_ms;
}

/**
 * \brief A generic wrapper class.
 * \tparam Type The type of the object to wrap. Type must be default-constructible.
 */
template <typename Type>
class wrapper
{
	public:

		/**
		 * \brief The wrapped type.
		 */
		typedef Type wrapped_type;

		/**
		 * \brief Create a default wrapper object.
		 */
		wrapper() {}

		/**
		 * \brief Create a new wrapped object.
		 * \tparam OtherType The other type.
		 * \param value A value.
		 */
		template <typename OtherType>
		wrapper(OtherType value) : m_object(value) {}

		/**
		 * \brief Conversion operator.
		 * \tparam OtherType The other type.
		 * \return The converted value.
		 */
		template <typename OtherType>
		operator OtherType() const
		{
			return m_object;
		}

	private:

		Type m_object;

		template <typename OtherType> friend std::istream& operator>>(std::istream&, wrapper<OtherType>&);
		template <typename OtherType> friend std::ostream& operator<<(std::ostream&, const wrapper<OtherType>&);
};

template <typename Type>
inline std::istream& operator>>(std::istream& is, wrapper<Type>& value)
{
	return is >> value.m_object;
}
template <typename Type>
inline std::ostream& operator<<(std::ostream& os, const wrapper<Type>& value)
{
	return os << value.m_object;
}

#endif /* CONFIGURATION_TYPES_HPP */

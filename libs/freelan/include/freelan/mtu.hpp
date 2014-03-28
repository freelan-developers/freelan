/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file mtu.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A MTU type.
 */

#ifndef FREELAN_MTU_HPP
#define FREELAN_MTU_HPP

#include <iostream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace freelan
{
	/**
	 * \brief A MTU auto type.
	 */
	struct auto_mtu_type
	{
		static const std::string string_representation;

		friend bool operator==(const auto_mtu_type&, const auto_mtu_type&)
		{
			return true;
		}
	};

	/**
	 * \brief A MTU system type.
	 */
	struct system_mtu_type
	{
		static const std::string string_representation;

		friend bool operator==(const system_mtu_type&, const system_mtu_type&)
		{
			return true;
		}
	};

	/**
	 * \brief The MTU type.
	 */
	typedef boost::variant<auto_mtu_type, system_mtu_type, unsigned int> mtu_type;

	/**
	 * \brief A visitor that computes the MTU.
	 */
	class compute_mtu_visitor : public boost::static_visitor<unsigned int>
	{
		public:

			/**
			 * \brief Create a compute mtu visitor.
			 * \param auto_value The auto value to use with auto_mtu_type.
			 */
			compute_mtu_visitor(unsigned int auto_value) :
				m_auto_value(auto_value)
			{
			}

			/**
			 * \brief Return the numeric value of the MTU.
			 * \param value The numeric value.
			 * \return The numeric value of the MTU.
			 */
			result_type operator()(unsigned int value) const
			{
				return value;
			}

			/**
			 * \brief Return the automatic value for the MTU.
			 * \return The automatic value for the MTU.
			 */
			result_type operator()(const auto_mtu_type&) const
			{
				return m_auto_value;
			}

			/**
			 * \brief Return the system value for the MTU.
			 * \return The system value for the MTU.
			 */
			result_type operator()(const system_mtu_type&) const
			{
				return 0;
			}

		private:

			unsigned int m_auto_value;
	};

	/**
	 * \brief A visitor that get the string representation of the MTU.
	 */
	class mtu_to_string_visitor : public boost::static_visitor<std::string>
	{
		public:

			/**
			 * \brief Return the string representation of the MTU.
			 * \param value The numeric value.
			 * \return The string representation of the MTU.
			 */
			result_type operator()(unsigned int value) const
			{
				return boost::lexical_cast<std::string>(value);
			}

			/**
			 * \brief Return the string representation of the MTU.
			 * \return The string representation of the MTU.
			 */
			result_type operator()(const auto_mtu_type&) const
			{
				return auto_mtu_type::string_representation;
			}

			/**
			 * \brief Return the string representation of the MTU.
			 * \return The string representation of the MTU.
			 */
			result_type operator()(const system_mtu_type&) const
			{
				return system_mtu_type::string_representation;
			}
	};

	/**
	 * \brief Read a mtu from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, mtu_type& value);

	/**
	 * \brief Write a mtu to an output stream.
	 * \param os The output stream.
	 * \param value The mtu.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const mtu_type& value)
	{
		return os << boost::apply_visitor(mtu_to_string_visitor(), value);
	}

	/**
	 * \brief Compare two MTUs.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two endpoints are different.
	 */
	inline bool operator!=(const mtu_type& lhs, const mtu_type& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * \brief Compute the MTU numerical value.
	 * \param mtu The MTU.
	 * \param auto_value The value to use with auto_mtu_type.
	 * \return The MTU associated numerical value.
	 */
	inline unsigned int compute_mtu(const mtu_type& mtu, unsigned int auto_value)
	{
		return boost::apply_visitor(compute_mtu_visitor(auto_value), mtu);
	}
}

#endif /* FREELAN_MTU_HPP */

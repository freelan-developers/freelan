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
 * \file mss.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A MSS type.
 */

#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace freelan
{
	/**
	 * \brief A MSS auto type.
	 */
	struct auto_mss_type
	{
		static const std::string string_representation;

		friend bool operator==(const auto_mss_type&, const auto_mss_type&)
		{
			return true;
		}
	};

	/**
	 * \brief A disabled MSS type.
	 */
	struct disabled_mss_type
	{
		static const std::string string_representation;

		friend bool operator==(const disabled_mss_type&, const disabled_mss_type&)
		{
			return true;
		}
	};

	/**
	 * \brief The MSS type.
	 */
	typedef boost::variant<auto_mss_type, disabled_mss_type, size_t> mss_type;

	/**
	 * \brief A visitor that computes the MSS.
	 */
	class compute_mss_visitor : public boost::static_visitor<size_t>
	{
		public:

			/**
			 * \brief Create a compute mss visitor.
			 * \param auto_value The auto value to use with auto_mtu_type.
			 */
			explicit compute_mss_visitor(size_t auto_value) :
				m_auto_value(auto_value)
			{
			}

			/**
			 * \brief Return the numeric value of the MSS.
			 * \param value The numeric value.
			 * \return The numeric value of the MSS.
			 */
			result_type operator()(size_t value) const
			{
				return value;
			}

			/**
			 * \brief Return the automatic value for the MSS.
			 * \return The automatic value for the MSS.
			 */
			result_type operator()(const auto_mss_type&) const
			{
				return m_auto_value;
			}

			/**
			 * \brief Return the system value for the MSS.
			 * \return The system value for the MSS.
			 */
			result_type operator()(const disabled_mss_type&) const
			{
				return 0;
			}

		private:

			size_t m_auto_value;
	};

	/**
	 * \brief A visitor that get the string representation of the MSS.
	 */
	class mss_to_string_visitor : public boost::static_visitor<std::string>
	{
		public:

			/**
			 * \brief Return the string representation of the MSS.
			 * \param value The numeric value.
			 * \return The string representation of the MSS.
			 */
			result_type operator()(size_t value) const
			{
				return boost::lexical_cast<std::string>(value);
			}

			/**
			 * \brief Return the string representation of the MSS.
			 * \return The string representation of the MSS.
			 */
			result_type operator()(const auto_mss_type&) const
			{
				return auto_mss_type::string_representation;
			}

			/**
			 * \brief Return the string representation of the MSS.
			 * \return The string representation of the MSS.
			 */
			result_type operator()(const disabled_mss_type&) const
			{
				return disabled_mss_type::string_representation;
			}
	};

	/**
	 * \brief Read a MSS from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, mss_type& value);

	/**
	 * \brief Write a MSS to an output stream.
	 * \param os The output stream.
	 * \param value The MSS.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const mss_type& value)
	{
		return os << boost::apply_visitor(mss_to_string_visitor(), value);
	}

	/**
	 * \brief Compare two MSS values.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two endpoints are different.
	 */
	inline bool operator!=(const mss_type& lhs, const mss_type& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * \brief Compute the MSS numerical value.
	 * \param mtu The MSS.
	 * \param auto_value The value to use with auto_mtu_type.
	 * \return The MSS associated numerical value.
	 */
	inline unsigned int compute_mss(const mss_type& mtu, size_t auto_value)
	{
		return boost::apply_visitor(compute_mss_visitor(auto_value), mtu);
	}
}

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
 * \file metric.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A metric type.
 */

#ifndef FREELAN_METRIC_HPP
#define FREELAN_METRIC_HPP

#include <iostream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

namespace freelan
{
	/**
	 * \brief An auto metric type.
	 */
	struct auto_metric_type
	{
		static const std::string string_representation;

		friend bool operator==(const auto_metric_type&, const auto_metric_type&)
		{
			return true;
		}
	};

	/**
	 * \brief A system metric type.
	 */
	struct system_metric_type
	{
		static const std::string string_representation;

		friend bool operator==(const system_metric_type&, const system_metric_type&)
		{
			return true;
		}
	};

	/**
	 * \brief The metric type.
	 */
	typedef boost::variant<auto_metric_type, system_metric_type, unsigned int> metric_type;

	/**
	 * \brief A visitor that get the string representation of the metric.
	 */
	class metric_to_string_visitor : public boost::static_visitor<std::string>
	{
		public:

			/**
			 * \brief Return the string representation of the metric.
			 * \param value The numeric value.
			 * \return The string representation of the metric.
			 */
			result_type operator()(unsigned int value) const
			{
				return boost::lexical_cast<std::string>(value);
			}

			/**
			 * \brief Return the string representation of the metric.
			 * \return The string representation of the metric.
			 */
			template <typename T>
			result_type operator()(const T&) const
			{
				return T::string_representation;
			}
	};

	/**
	 * \brief Read a metric from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, metric_type& value);

	/**
	 * \brief Write a metric to an output stream.
	 * \param os The output stream.
	 * \param value The mtu.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const metric_type& value)
	{
		return os << boost::apply_visitor(metric_to_string_visitor(), value);
	}

	/**
	 * \brief Compare two metrics.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two endpoints are different.
	 */
	inline bool operator!=(const metric_type& lhs, const metric_type& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * \brief A visitor that get the metric value.
	 */
	class metric_to_value_visitor : public boost::static_visitor<boost::optional<unsigned int>>
	{
		public:

			/**
			 * \brief Return the value of the metric.
			 * \param value The numeric value.
			 * \return The value of the metric.
			 */
			result_type operator()(unsigned int value) const
			{
				return value;
			}

			/**
			 * \brief Return the value of the metric.
			 * \return The value of the metric.
			 */
			result_type operator()(const auto_metric_type&) const
			{
				return 3;
			}

			/**
			 * \brief Return the value of the metric.
			 * \return The value of the metric.
			 */
			result_type operator()(const system_metric_type&) const
			{
				return boost::none;
			}
	};

	/**
	 * \brief Get the value of the metric.
	 * \param metric The metric.
	 * \return The value, if any.
	 */
	inline boost::optional<unsigned int> get_metric_value(const metric_type& metric)
	{
		return boost::apply_visitor(metric_to_value_visitor(), metric);
	}
}

#endif /* FREELAN_METRIC_HPP */

/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file error.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The errors.
 */

#ifndef ASIOTAP_ERROR_HPP
#define ASIOTAP_ERROR_HPP

#include <boost/system/error_code.hpp>
#include <boost/type_traits/integral_constant.hpp>

#include <string>

namespace asiotap
{
	/**
	 * @brief Get the default server error category.
	 * @return The default server error category instance.
	 *
	 * @warning The first call to this function is thread-safe only starting with C++11.
	 */
	const boost::system::error_category& asiotap_category();

	/**
	 * The list of errors.
	 */
	enum class asiotap_error
	{
		success = 0,
		invalid_tap_adapter_layer,
		invalid_type,
		no_ethernet_address,
		process_handle_expected,
		external_process_output_parsing_error,
		no_such_tap_adapter,
		invalid_ip_configuration,
		external_process_execution_failed,
		no_dns_script_provided
	};

	/**
	 * @brief Create an error_code instance for the given error.
	 * @param error The error.
	 * @return The error_code instance.
	 */
	inline boost::system::error_code make_error_code(asiotap_error error)
	{
		return boost::system::error_code(static_cast<int>(error), asiotap_category());
	}

	/**
	 * @brief Create an error_condition instance for the given error.
	 * @param error The error.
	 * @return The error_condition instance.
	 */
	inline boost::system::error_condition make_error_condition(asiotap_error error)
	{
		return boost::system::error_condition(static_cast<int>(error), asiotap_category());
	}

	/**
	 * @brief A server error category.
	 */
	class asiotap_category_impl : public boost::system::error_category
	{
		public:
			/**
			 * @brief Get the name of the category.
			 * @return The name of the category.
			 */
			virtual const char* name() const throw();

			/**
			 * @brief Get the error message for a given error.
			 * @param ev The error numeric value.
			 * @return The message associated to the error.
			 */
			virtual std::string message(int ev) const;
	};
}

namespace boost
{
	namespace system
	{
		template <>
		struct is_error_code_enum< ::asiotap::asiotap_error> : public boost::true_type {};
	}
}

#endif /* ASIOTAP_ERROR_HPP */

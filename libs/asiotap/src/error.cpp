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
 * \file error.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The errors.
 */

#include "error.hpp"

namespace asiotap
{
	const boost::system::error_category& asiotap_category()
	{
		static asiotap_category_impl instance;

		return instance;
	}

	const char* asiotap_category_impl::name() const throw()
	{
		return "asiotap::error";
	}

	std::string asiotap_category_impl::message(int ev) const
	{
		switch (static_cast<asiotap_error>(ev))
		{
			case asiotap_error::success:
			{
				return "Success";
			}
			case asiotap_error::invalid_tap_adapter_layer:
			{
				return "The TAP adapter doesn't operate on the required layer";
			}
			case asiotap_error::invalid_type:
			{
				return "The value has an invalid type";
			}
			case asiotap_error::no_such_tap_adapter:
			{
				return "No TAP adapter found which has the specified name";
			}
			case asiotap_error::no_ethernet_address:
			{
				return "No ethernet address";
			}
			case asiotap_error::process_handle_expected:
			{
				return "A process handle was expected";
			}
			case asiotap_error::external_process_output_parsing_error:
			{
				return "Unable to parse the output of the external process";
			}
			case asiotap_error::invalid_ip_configuration:
			{
				return "The specified IP configuration is invalid";
			}
			case asiotap_error::external_process_execution_failed:
			{
				return "The execution of the external process failed";
			}
			case asiotap_error::no_dns_script_provided:
			{
				return "No DNS script was provided";
			}
			default:
			{
				return "Unknown asiotap error";
			}
		}
	}
}

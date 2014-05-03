/*
 * libnetlinkplus - A portable netlink extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libnetlinkplus.
 *
 * libnetlinkplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libnetlinkplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libnetlinkplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file error.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The errors.
 */

#include "error.hpp"

namespace netlinkplus
{
	const boost::system::error_category& netlinkplus_category()
	{
		static netlinkplus_category_impl instance;

		return instance;
	}

	const char* netlinkplus_category_impl::name() const throw()
	{
		return "netlinkplus::error";
	}

	std::string netlinkplus_category_impl::message(int ev) const
	{
		switch (static_cast<netlinkplus_error>(ev))
		{
			case netlinkplus_error::success:
			{
				return "Success";
			}
			case netlinkplus_error::invalid_response:
			{
				return "Invalid response";
			}
			case netlinkplus_error::unexpected_response_type:
			{
				return "Unexpected response type";
			}
			case netlinkplus_error::invalid_route_destination:
			{
				return "Invalid route destination";
			}
			case netlinkplus_error::invalid_route_source:
			{
				return "Invalid route source";
			}
			case netlinkplus_error::invalid_route_input_interface:
			{
				return "Invalid route input interface";
			}
			case netlinkplus_error::invalid_route_output_interface:
			{
				return "Invalid route output interface";
			}
			case netlinkplus_error::invalid_route_gateway:
			{
				return "Invalid route gateway";
			}
			case netlinkplus_error::invalid_route_priority:
			{
				return "Invalid route priority";
			}
			case netlinkplus_error::invalid_route_metric:
			{
				return "Invalid route metric";
			}
			default:
			{
				return "Unknown netlinkplus error";
			}
		}
	}
}

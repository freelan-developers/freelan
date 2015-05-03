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
 * \file ip_route.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IP route type, extended with specific configuration values.
 */

#include "ip_route.hpp"

namespace freelan
{
	const std::string default_ipv4_gateway_type::string_representation = "ipv4_proxy";
	const std::string default_ipv6_gateway_type::string_representation = "ipv6_proxy";

	std::istream& operator>>(std::istream& is, ip_route& value)
	{
		asiotap::ip_route new_value;

		if (is >> new_value)
		{
			value = new_value;
		} else {
			if (!is.eof())
			{
				is.clear();

				std::string str;

				if (is >> str)
				{
					if (str == default_ipv4_gateway_type::string_representation)
					{
						value = default_ipv4_gateway_type();
					}
					else if (str == default_ipv6_gateway_type::string_representation)
					{
						value = default_ipv6_gateway_type();
					}
					else
					{
						is.clear(std::istream::failbit);
					}
				}
			}
		}

		return is;
	}
}

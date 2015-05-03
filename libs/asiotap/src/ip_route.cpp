/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
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
 * \file ip_configuration.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Deals with IP configurations.
 */

#include "types/ip_route.hpp"

#include "types/stream_operations.hpp"

#include <boost/lexical_cast.hpp>

namespace asiotap
{
	template <typename AddressType>
	std::istream& operator>>(std::istream& is, base_ip_route<AddressType>& value)
	{
		std::string ip_address;
		std::string prefix_length;
		std::string gateway;

		if (read_ip_address_prefix_length_gateway<AddressType>(is, ip_address, prefix_length, gateway))
		{
			base_ip_network_address<AddressType> ina;

			if (prefix_length.empty())
			{
				ina = AddressType::from_string(ip_address);
			}
			else
			{
				ina = { AddressType::from_string(ip_address), boost::lexical_cast<unsigned int>(prefix_length) };
			}

			if (gateway.empty())
			{
				value = ina;
			}
			else
			{
				value = { ina, AddressType::from_string(gateway) };
			}
		}

		return is;
	}

	template std::istream& operator>>(std::istream& is, ipv4_route& value);
	template std::istream& operator>>(std::istream& is, ipv6_route& value);

	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const base_ip_route<AddressType>& value)
	{
		if (value.gateway())
		{
			return os << value.network_address() << " => " << *value.gateway();
		}
		else
		{
			return os << value.network_address();
		}
	}

	template std::ostream& operator<<(std::ostream& is, const ipv4_route& value);
	template std::ostream& operator<<(std::ostream& is, const ipv6_route& value);

	std::istream& operator>>(std::istream& is, ip_route& value)
	{
		if (is)
		{
			ipv6_route ir;

			if (is >> ir)
			{
				value = ir;
				return is;
			}

			is.clear();
		}

		if (is)
		{
			ipv4_route ir;

			if (is >> ir)
			{
				value = ir;
				return is;
			}
		}

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const ip_route_set& routes)
	{
		if (routes.size() > 0)
		{
			auto route = routes.begin();

			os << *route;

			for (++route; route != routes.end(); ++route)
			{
				os << ", " << *route;
			}
		}

		return os;
	}
}

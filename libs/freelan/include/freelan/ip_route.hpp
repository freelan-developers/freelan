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
 * \file ip_route.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IP route type, extended with specific configuration values.
 */

#pragma once

#include <iostream>

#include <asiotap/types/ip_route.hpp>

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace freelan
{
	/**
	 * \brief A default IPv4 gateway value.
	 */
	struct default_ipv4_gateway_type
	{
		static const std::string string_representation;

		friend bool operator==(const default_ipv4_gateway_type&, const default_ipv4_gateway_type&)
		{
			return true;
		}

		friend bool operator<(const default_ipv4_gateway_type&, const default_ipv4_gateway_type&)
		{
			return false;
		}
	};

	/**
	 * \brief A default IPv6 gateway value.
	 */
	struct default_ipv6_gateway_type
	{
		static const std::string string_representation;

		friend bool operator==(const default_ipv6_gateway_type&, const default_ipv6_gateway_type&)
		{
			return true;
		}

		friend bool operator<(const default_ipv6_gateway_type&, const default_ipv6_gateway_type&)
		{
			return false;
		}
	};

	/**
	 * \brief The IP route type.
	 */
	typedef boost::variant<default_ipv4_gateway_type, default_ipv6_gateway_type, asiotap::ip_route> ip_route;

	/**
	 * \brief A visitor that get the route.
	 */
	class get_ip_route_visitor : public boost::static_visitor<asiotap::ip_route>
	{
		public:

			get_ip_route_visitor(asiotap::ipv4_route ipv4_default, asiotap::ipv6_route ipv6_default) :
				m_ipv4_default(ipv4_default),
				m_ipv6_default(ipv6_default)
			{}

			result_type operator()(default_ipv4_gateway_type) const
			{
				return m_ipv4_default;
			}

			result_type operator()(default_ipv6_gateway_type) const
			{
				return m_ipv6_default;
			}

			result_type operator()(const asiotap::ip_route& value) const
			{
				return value;
			}

	private:

			asiotap::ipv4_route m_ipv4_default;
			asiotap::ipv6_route m_ipv6_default;
	};

	inline asiotap::ip_route to_ip_route(const ip_route& value, asiotap::ipv4_route ipv4_default, asiotap::ipv6_route ipv6_default)
	{
		return boost::apply_visitor(get_ip_route_visitor(ipv4_default, ipv6_default), value);
	}

	inline asiotap::ip_route_set to_ip_routes(const std::set<ip_route>& values, asiotap::ipv4_route ipv4_default, asiotap::ipv6_route ipv6_default)
	{
		asiotap::ip_route_set result;

		for (auto&& value : values) {
			result.insert(to_ip_route(value, ipv4_default, ipv6_default));
		}

		return result;
	}

	/**
	 * \brief A visitor that get the string representation of the IP route.
	 */
	class ip_route_to_string_visitor : public boost::static_visitor<std::string>
	{
		public:
			result_type operator()(const asiotap::ip_route& value) const
			{
				return boost::lexical_cast<std::string>(value);
			}

			template <typename T>
			result_type operator()(const T&) const
			{
				return T::string_representation;
			}
	};

	std::istream& operator>>(std::istream& is, ip_route& value);

	inline std::ostream& operator<<(std::ostream& os, const ip_route& value)
	{
		return os << boost::apply_visitor(ip_route_to_string_visitor(), value);
	}

	inline bool operator!=(const ip_route& lhs, const ip_route& rhs)
	{
		return !(lhs == rhs);
	}
}

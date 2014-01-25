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
 * \file base_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Deals with IP configurations.
 */

#ifndef ASIOTAP_IP_CONFIGURATION_HPP
#define ASIOTAP_IP_CONFIGURATION_HPP

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <iomanip>
#include <vector>

#include "os.hpp"
#include "error.hpp"

namespace asiotap
{
	/**
	 * \brief An IP address and prefix length type.
	 */
	template <typename AddressType>
	struct generic_ip_address_prefix_length
	{
		/**
		 * \brief The address type.
		 */
		typedef AddressType address_type;

		/**
		 * \brief The IP address.
		 */
		address_type ip_address;

		/**
		 * \brief The prefix length.
		 */
		unsigned int prefix_length;

		friend bool operator<(const generic_ip_address_prefix_length& lhs, const generic_ip_address_prefix_length& rhs)
		{
			if (lhs.ip_address < rhs.ip_address)
			{
				return true;
			}
			else if (lhs.ip_address == rhs.ip_address)
			{
				return (lhs.prefix_length < rhs.prefix_length);
			}
			else
			{
				return false;
			}
		}

		friend bool operator==(const generic_ip_address_prefix_length& lhs, const generic_ip_address_prefix_length& rhs)
		{
			return ((lhs.ip_address == rhs.ip_address) && (lhs.prefix_length == rhs.prefix_length));
		}
	};

	typedef generic_ip_address_prefix_length<boost::asio::ip::address_v4> ipv4_address_prefix_length;
	typedef generic_ip_address_prefix_length<boost::asio::ip::address_v6> ipv6_address_prefix_length;

	inline std::ostream& operator<<(std::ostream& os, const ipv4_address_prefix_length& v)
	{
		return os << v.ip_address << ":" << v.prefix_length;
	}

	inline std::ostream& operator<<(std::ostream& os, const ipv6_address_prefix_length& v)
	{
		return os << "[" << v.ip_address << "]:" << v.prefix_length;
	}

	struct ip_configuration
	{
		public:
			boost::optional<ipv4_address_prefix_length> ipv4;
			boost::optional<ipv6_address_prefix_length> ipv6;
			boost::optional<boost::asio::ip::address_v4> remote_ipv4_address;
	};

	struct ip_addresses
	{
		std::vector<ipv4_address_prefix_length> ipv4;
		std::vector<ipv6_address_prefix_length> ipv6;
	};
}

#endif /* ASIOTAP_IP_CONFIGURATION_HPP */

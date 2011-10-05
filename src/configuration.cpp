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
 * \file configuration.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The configuration structure.
 */

#include "configuration.hpp"

#include <sstream>

namespace freelan
{
	configuration::configuration() :
		hostname_resolution_protocol(boost::asio::ip::udp::v4()),
		certificate_validation_callback(0)
	{
	}

	std::ostream& operator<<(std::ostream& os, configuration::ethernet_address_type value)
	{
		return os 
			<< std::hex << std::setfill('0') << std::setw(2) <<
			static_cast<unsigned int>(value[0]) << ':' <<
			static_cast<unsigned int>(value[1]) << ':' <<
			static_cast<unsigned int>(value[2]) << ':' <<
			static_cast<unsigned int>(value[3]) << ':' <<
			static_cast<unsigned int>(value[4]) << ':' <<
			static_cast<unsigned int>(value[5]);
	}

	std::istream& operator>>(std::istream& is, configuration::ethernet_address_type& value)
	{
		unsigned int digit;
		char delimiter;
		size_t i = 0;

		is >> std::hex >> std::setfill('0') >> std::setw(2);

		for (; is && (i < value.size() - 1); ++i)
		{
			is >> digit >> delimiter;

			if ((delimiter != ':') && (delimiter != '-'))
			{
				is.setstate(std::ios::badbit);
			} else
			{
				value[i] = static_cast<configuration::ethernet_address_type::value_type>(digit);
			}
		}

		if (is)
		{
			is >> digit;
			value[i] = static_cast<configuration::ethernet_address_type::value_type>(digit);
		}

		return is;
	}

	std::istream& operator>>(std::istream& is, configuration::ipv4_address_prefix_length_type& value)
	{
		std::ostringstream oss;

		is.get(*oss.rdbuf(), '/').ignore() >> std::dec >> value.prefix_length;

		value.address = boost::asio::ip::address_v4::from_string(oss.str());

		return is;
	}

	std::istream& operator>>(std::istream& is, configuration::ipv6_address_prefix_length_type& value)
	{
		std::ostringstream oss;

		is.get(*oss.rdbuf(), '/').ignore() >> std::dec >> value.prefix_length;

		value.address = boost::asio::ip::address_v6::from_string(oss.str());

		return is;
	}
}

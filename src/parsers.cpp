/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file parsers.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Parsing functions.
 */

#include "parsers.hpp"

#include <cctype>
#include <cassert>

#include <boost/lexical_cast.hpp>

namespace
{
	template <typename IpAddressType>
		bool is_ip_address_character(char c);

	template <>
		bool is_ip_address_character<boost::asio::ip::address_v4>(char c)
		{
			return (std::isdigit(c) || (c == '.'));
		}

	template <>
		bool is_ip_address_character<boost::asio::ip::address_v6>(char c)
		{
			return (std::isxdigit(c) || (c == ':'));
		}

	template <typename IpAddressType>
		bool parse_ip_address(std::string::const_iterator& begin, std::string::const_iterator end, IpAddressType& val)
		{
			assert(begin < end);

			const std::string::const_iterator save_begin = begin;

			for (; (begin != end) && is_ip_address_character<IpAddressType>(*begin); ++begin);

			try
			{
				val = IpAddressType::from_string(std::string(save_begin, begin));
			}
			catch (std::exception&)
			{
				return false;
			}

			return true;
		}

	bool is_ethernet_address_delimiter(char c)
	{
		return (c == ':') || (c == '-');
	}

	bool is_ethernet_address_character(char c)
	{
		return (std::isxdigit(c) || is_ethernet_address_delimiter(c));
	}

	template <typename NumberType>
		bool parse_number(std::string::const_iterator& begin, std::string::const_iterator end, NumberType& val)
		{
			assert(begin < end);

			const std::string::const_iterator save_begin = begin;

			for (; (begin != end) && std::isdigit(*begin); ++begin);

			try
			{
				val = boost::lexical_cast<NumberType>(std::string(save_begin, begin));
			}
			catch (std::exception&)
			{
				return false;
			}

			return true;
		}

	uint8_t xdigit_to_numeric(char c)
	{
		static const char table[] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		return table[static_cast<uint8_t>(c)];
	}
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v4& val)
{
	return parse_ip_address(begin, end, val);
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v6& val)
{
	return parse_ip_address(begin, end, val);
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, uint16_t& val)
{
	return parse_number(begin, end, val);
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, size_t& val)
{
	return parse_number(begin, end, val);
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ethernet_address_type& val)
{
	assert(begin < end);

	// Wrong size
	if (std::distance(begin, end) != (val.size() * 3 - 1))
	{
		begin = end;
		return false;
	}

	const std::string::const_iterator save_begin = begin;

	for (; (begin != end); ++begin)
	{
		if (std::distance(save_begin, begin) % 3 == 2)
		{
			if (!is_ethernet_address_delimiter(*begin))
			{
				return false;
			}
		} else
		{
			if (!std::isxdigit(*begin))
			{
				return false;
			}

			if (std::distance(save_begin, begin) % 3 == 1)
			{
				val[std::distance(save_begin, begin) / 3] = (xdigit_to_numeric(*(begin - 1)) << 4) | xdigit_to_numeric(*begin);
			}
		}
	}

	return true;
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ipv4_address_prefix_length_type& val)
{
	if (!parse(begin, end, val.address))
	{
		return false;
	}

	if ((begin == end) || (*begin != '/'))
	{
		return false;
	}

	++begin;

	if (!parse(begin, end, val.prefix_length))
	{
		return false;
	}

	return true;
}

bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ipv6_address_prefix_length_type& val)
{
	if ((begin == end) || (*begin != '['))
	{
		return false;
	}

	++begin;

	if (!parse(begin, end, val.address))
	{
		return false;
	}

	if ((begin == end) || (*begin != ']'))
	{
		return false;
	}

	++begin;

	if ((begin == end) || (*begin != '/'))
	{
		return false;
	}

	++begin;

	if (!parse(begin, end, val.prefix_length))
	{
		return false;
	}

	return true;
}

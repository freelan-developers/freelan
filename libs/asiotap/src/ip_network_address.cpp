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

#include "types/ip_network_address.hpp"

#include "types/stream_operations.hpp"

#include <boost/lexical_cast.hpp>

namespace asiotap
{
	template <typename AddressType>
	bool base_ip_network_address<AddressType>::has_address(const AddressType& addr) const
	{
		const typename AddressType::bytes_type network_bytes = address().to_bytes();
		const typename AddressType::bytes_type addr_bytes = addr.to_bytes();

		unsigned int prefix_len = m_prefix_length;

		for (size_t i = 0; i < network_bytes.size(); ++i)
		{
			if (prefix_len >= 8)
			{
				if (network_bytes[i] != addr_bytes[i])
					return false;

				prefix_len -= 8;
			}
			else
			{
				const typename AddressType::bytes_type::value_type mask = (static_cast<unsigned char>(0xFF) >> prefix_len) ^ static_cast<unsigned char>(0xFF);

				if ((network_bytes[i] & mask) != (addr_bytes[i] & mask))
					return false;

				break;
			}
		}

		return true;
	}

	template bool base_ip_network_address<boost::asio::ip::address_v4>::has_address(const boost::asio::ip::address_v4&) const;
	template bool base_ip_network_address<boost::asio::ip::address_v6>::has_address(const boost::asio::ip::address_v6&) const;

	template <typename AddressType>
	bool base_ip_network_address<AddressType>::has_network(const base_ip_network_address& addr) const
	{
		// If the network address has a prefix length that is more or equally inclusive to ours: it can't be a subnet.
		if (addr.m_prefix_length <= m_prefix_length)
		{
			return false;
		}

		return has_address(addr.get_network_address());
	}

	template bool base_ip_network_address<boost::asio::ip::address_v4>::has_network(const base_ip_network_address<boost::asio::ip::address_v4>&) const;
	template bool base_ip_network_address<boost::asio::ip::address_v6>::has_network(const base_ip_network_address<boost::asio::ip::address_v6>&) const;

	template <typename AddressType>
	typename base_ip_network_address<AddressType>::address_type base_ip_network_address<AddressType>::get_network_address() const
	{
		typename AddressType::bytes_type network_bytes = address().to_bytes();

		unsigned int prefix_len = m_prefix_length;

		for (auto&& byte : network_bytes)
		{
			if (prefix_len >= 8)
			{
				prefix_len -= 8;
			}
			else if (prefix_len > 0)
			{
				const auto byte_mask = static_cast<typename AddressType::bytes_type::value_type>((0xFF >> prefix_len) ^ 0xFF);

				byte ^= byte_mask;
			}
			else
			{
				byte = 0x00;
			}
		}

		return AddressType(network_bytes);
	}

	template boost::asio::ip::address_v4 base_ip_network_address<boost::asio::ip::address_v4>::get_network_address() const;
	template boost::asio::ip::address_v6 base_ip_network_address<boost::asio::ip::address_v6>::get_network_address() const;

	template <typename AddressType>
	std::istream& operator>>(std::istream& is, base_ip_network_address<AddressType>& value)
	{
		std::string ip_address;
		std::string prefix_length;

		if (read_ip_address_prefix_length<AddressType>(is, ip_address, prefix_length))
		{
			if (prefix_length.empty())
			{
				value = base_ip_network_address<AddressType>(AddressType::from_string(ip_address));
			}
			else
			{
				value = base_ip_network_address<AddressType>(AddressType::from_string(ip_address), boost::lexical_cast<unsigned int>(prefix_length));
			}
		}

		return is;
	}

	template std::istream& operator>>(std::istream& is, ipv4_network_address& value);
	template std::istream& operator>>(std::istream& is, ipv6_network_address& value);

	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const base_ip_network_address<AddressType>& value)
	{
		return os << value.address().to_string() << "/" << std::dec << value.prefix_length();
	}

	template std::ostream& operator<<(std::ostream& is, const ipv4_network_address& value);
	template std::ostream& operator<<(std::ostream& is, const ipv6_network_address& value);

	std::istream& operator>>(std::istream& is, ip_network_address& value)
	{
		if (is)
		{
			ipv6_network_address ina;

			if (is >> ina)
			{
				value = ina;
				return is;
			}

			is.clear();
		}

		if (is)
		{
			ipv4_network_address ina;

			if (is >> ina)
			{
				value = ina;
				return is;
			}

			is.clear();
		}

		return is;
	}
}

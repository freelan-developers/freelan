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
 * \file ip_endpoint.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief IP endpoint classes.
 */

#include "ip_endpoint.hpp"

#include <boost/bind.hpp>

namespace freelan
{
	namespace
	{
		std::istream& putback(std::istream& is, const std::string& str)
		{
			std::ios::iostate state = is.rdstate();
			is.clear();

			std::for_each(str.rbegin(), str.rend(), boost::bind(&std::istream::putback, boost::ref(is), _1));

			is.setstate(state);

			return is;
		}

		template <typename AddressType>
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

		template <typename AddressType>
		std::istream& read_ip_address(std::istream& is, std::string& ip_address)
		{
			if (is.good())
			{
				if (!is_ip_address_character<AddressType>(is.peek()))
				{
					is.setstate(std::ios_base::failbit);
				}
				else
				{
					std::ostringstream oss;

					do
					{
						oss.put(static_cast<char>(is.get()));
					}
					while (is.good() && is_ip_address_character<AddressType>(is.peek()));

					if (is)
					{
						const std::string& result = oss.str();
						boost::system::error_code ec;

						AddressType::from_string(result, ec);

						if (ec)
						{
							// Unable to parse the IP address: putting back characters.
							putback(is, result);
							is.setstate(std::ios_base::failbit);
						}
						else
						{
							ip_address = result;
						}
					}
				}
			}

			return is;
		}

		std::istream& read_port(std::istream& is, std::string& port)
		{
			uint16_t num_port;

			if (is >> std::dec >> num_port)
			{
				port = boost::lexical_cast<std::string>(num_port);
			}

			return is;
		}

		template <typename AddressType>
		std::istream& read_ip_address_port(std::istream& is, std::string& ip_address, std::string& port);

		template <>
		std::istream& read_ip_address_port<boost::asio::ip::address_v4>(std::istream& is, std::string& ip_address, std::string& port)
		{
			if (is.good())
			{
				if (read_ip_address<boost::asio::ip::address_v4>(is, ip_address))
				{
					if (is.good() && (is.peek() == ':'))
					{
						is.ignore();

						if (!read_port(is, port))
						{
							putback(is, ip_address + ':');
							is.setstate(std::ios_base::failbit);
						}
					}
				}
			}

			return is;
		}

		template <>
		std::istream& read_ip_address_port<boost::asio::ip::address_v6>(std::istream& is, std::string& ip_address, std::string& port)
		{
			if (is.good())
			{
				if (is.peek() == '[')
				{
					is.ignore();

					if (!read_ip_address<boost::asio::ip::address_v6>(is, ip_address))
					{
						is.clear();
						is.putback('[');
						is.setstate(std::ios_base::failbit);
					}
					else
					{
						if (is.peek() != ']')
						{
							// End bracket not found: lets put back everything and fail.
							putback(is, '[' + ip_address);
							is.setstate(std::ios_base::failbit);
						}
						else
						{
							is.ignore();

							if (is.good() && (is.peek() == ':'))
							{
								is.ignore();

								if (!read_port(is, port))
								{
									putback(is, '[' + ip_address + ']' + ':');
									is.setstate(std::ios_base::failbit);
								}
							}
						}
					}
				}
				else
				{
					read_ip_address<boost::asio::ip::address_v6>(is, ip_address);
				}
			}

			return is;
		}
	}

	template <typename AddressType>
	std::istream& operator>>(std::istream& is, ip_endpoint<AddressType>& value)
	{
		std::string ip_address;
		std::string port;

		if (read_ip_address_port<AddressType>(is, ip_address, port))
		{
			value = ip_endpoint<AddressType>(AddressType::from_string(ip_address), port.empty() ? boost::none : boost::optional<uint16_t>(boost::lexical_cast<uint16_t>(port)));
		}

		return is;
	}

	template <>
	std::ostream& operator<<(std::ostream& os, const ipv4_endpoint& value)
	{
		os << value.address().to_string();

		if (value.has_port())
		{
			os << ":" << value.port();
		}

		return os;
	}

	template std::istream& operator>>(std::istream& is, ipv4_endpoint& value);
	template std::istream& operator>>(std::istream& is, ipv6_endpoint& value);

	template <>
	std::ostream& operator<<(std::ostream& os, const ipv6_endpoint& value)
	{
		if (value.has_port())
		{
			os << "[" << value.address().to_string() << "]:" << value.port();
		}
		else
		{
			os << value.address().to_string();
		}

		return os;
	}
}

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
 * \file stream_operations.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Stream related functions.
 */

#include "stream_operations.hpp"

#include <algorithm>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace freelan
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

	template std::istream& read_ip_address<boost::asio::ip::address_v4>(std::istream& is, std::string& ip_address);
	template std::istream& read_ip_address<boost::asio::ip::address_v6>(std::istream& is, std::string& ip_address);

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
	bool check_prefix_length(uint8_t prefix_length);

	template <>
	bool check_prefix_length<boost::asio::ip::address_v4>(uint8_t prefix_length)
	{
		return (prefix_length < 32);
	}

	template <>
	bool check_prefix_length<boost::asio::ip::address_v6>(uint8_t prefix_length)
	{
		return (prefix_length < 128);
	}

	template <typename AddressType>
	std::istream& read_prefix_length(std::istream& is, std::string& prefix_length)
	{
		if (is.good())
		{
			if (!isdigit(is.peek()))
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
				while (is.good() && isdigit(is.peek()));

				if (is)
				{
					const std::string& result = oss.str();

					std::istringstream iss(result);
					uint8_t num_prefix_length;

					if ((iss >> std::dec >> num_prefix_length) && check_prefix_length<AddressType>(num_prefix_length))
					{
						prefix_length = result;
					}
					else
					{
						putback(is, result);
						is.setstate(std::ios_base::failbit);
					}
				}
			}
		}

		return is;
	}

	// Hostname labels are 63 characters long at most
	const size_t HOSTNAME_LABEL_MAX_SIZE = 63;

	// Hostnames are at most 255 characters long
	const size_t HOSTNAME_MAX_SIZE = 255;

	bool is_hostname_label_regular_character(char c)
	{
		return std::isalnum(c);
	}

	bool is_hostname_label_special_character(char c)
	{
		return (c == '-');
	}

	bool is_hostname_label_character(char c)
	{
		return is_hostname_label_regular_character(c) || is_hostname_label_special_character(c);
	}

	std::istream& read_hostname_label(std::istream& is, std::string& label)
	{
		// Parse hostname labels according to RFC1123

		if (is.good())
		{
			if (!is_hostname_label_regular_character(is.peek()))
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
				while (is.good() && is_hostname_label_character(is.peek()));

				if (is)
				{
					const std::string& result = oss.str();

					// Check if the label is too long, if the last character is not a regular character or if it contains only digits
					if ((result.size() > HOSTNAME_LABEL_MAX_SIZE) || (!is_hostname_label_regular_character(result[result.size() - 1])) || (result.find_first_not_of("0123456789") == std::string::npos))
					{
						putback(is, result);
						is.setstate(std::ios_base::failbit);
					}
					else
					{
						label = result;
					}
				}
			}
		}

		return is;
	}

	std::istream& read_hostname(std::istream& is, std::string& hostname)
	{
		// Parse hostnames labels according to RFC952 and RFC1123

		std::string label;

		if (read_hostname_label(is, label))
		{
			if (is.eof())
			{
				// There is nothing more to read: lets use the content of the first label
				std::swap(hostname, label);
			}
			else
			{
				std::ostringstream oss(label);
				oss.seekp(0, std::ios::end);

				while (is.good() && (is.peek() == '.'))
				{
					is.ignore();
					oss.put('.');

					if (!read_hostname_label(is, label))
					{
						putback(is, oss.str());
						is.setstate(std::ios_base::failbit);
					}
					else
					{
						oss << label;
					}
				}

				if (is)
				{
					const std::string& result = oss.str();

					if (result.size() > HOSTNAME_MAX_SIZE)
					{
						putback(is, result);
						is.setstate(std::ios_base::failbit);
					}
					else
					{
						hostname = result;
					}
				}
			}
		}

		return is;
	}

	std::istream& read_service(std::istream& is, std::string& service)
	{
		if (is.good())
		{
			if (!isalnum(is.peek()))
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
				while (is.good() && isalnum(is.peek()));

				if (is)
				{
					const std::string& result = oss.str();

					// Check if the label contains only digits
					if (result.find_first_not_of("0123456789") == std::string::npos)
					{
						std::istringstream iss(result);
						uint16_t num_service;

						if (iss >> std::dec >> num_service)
						{
							service = result;
						}
						else
						{
							putback(is, result);
							is.setstate(std::ios_base::failbit);
						}
					}
					else
					{
						service = result;
					}
				}
			}
		}

		return is;
	}
}

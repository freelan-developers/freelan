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
 * \file hostname_endpoint.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A hostname endpoint class.
 */

#include "hostname_endpoint.hpp"

#include <cassert>

namespace freelan
{
	namespace
	{
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

					// Hostname labels are 63 characters long at most
					const size_t hostname_label_max_size = 63;

					do
					{
						oss.put(static_cast<char>(is.get()));
					}
					while (is.good() && (oss.tellp() < static_cast<std::streampos>(hostname_label_max_size)) && is_hostname_label_character(is.peek()));

					if (is)
					{
						label = oss.str();
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
				if (is.good())
				{
					// Don't replace this with:
					// std::ostringstream oss(label);
					//
					// Or tellp() will give an incorrect value !
					std::ostringstream oss;

					oss << label;

					// Hostnames are at most 255 characters long
					const size_t hostname_max_size = 255;

					while (is.good() && (oss.tellp() < static_cast<std::streampos>(hostname_max_size - 1)) && (is.peek() == '.'))
					{
						oss << static_cast<char>(is.get());

						if (read_hostname_label(is, label))
						{
							if (oss.tellp() > static_cast<std::streampos>(hostname_max_size - label.size()))
							{
								size_t cnt = hostname_max_size - oss.tellp();
								oss << label.substr(0, cnt);
								is.seekg(-static_cast<std::streamoff>(label.size() - cnt), std::ios_base::cur);
							}
							else
							{
								oss << label;
							}
						}
					}

					if (is)
					{
						hostname = oss.str();
					}
				}
				else
				{
					std::swap(hostname, label);
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
						service = oss.str();
					}
				}
			}

			return is;
		}

		std::istream& read_hostname_service(std::istream& is, std::string& hostname, std::string& service)
		{
			if (read_hostname(is, hostname))
			{
				if (is.good() && (is.get() == ':'))
				{
					read_service(is, service);
				}
			}

			return is;
		}
	}

	boost::asio::ip::udp::endpoint resolve(const hostname_endpoint& ep, hostname_endpoint::resolver& resolver, hostname_endpoint::resolver::protocol_type protocol, hostname_endpoint::resolver::query::flags flags, const std::string& default_service)
	{
		hostname_endpoint::resolver::query query(protocol, ep.hostname(), ep.service().empty() ? default_service : ep.service(), flags);

		return *resolver.resolve(query);
	}

	void async_resolve(const hostname_endpoint& ep, hostname_endpoint::resolver& resolver, hostname_endpoint::resolver::protocol_type protocol, hostname_endpoint::resolver::query::flags flags, const std::string& default_service, hostname_endpoint::handler handler)
	{
		hostname_endpoint::resolver::query query(protocol, ep.hostname(), ep.service().empty() ? default_service : ep.service(), flags);

		resolver.async_resolve(query, handler);
	}

	std::ostream& operator<<(std::ostream& os, const hostname_endpoint& value)
	{
		os << value.hostname();

		if (!value.service().empty())
		{
			os << ":" << value.service();
		}

		return os;
	}

	std::istream& operator>>(std::istream& is, hostname_endpoint& value)
	{
		std::string hostname;
		std::string service;

		if (read_hostname_service(is, hostname, service))
		{
			value = hostname_endpoint(hostname, service);
		}

		return is;
	}
	
	bool operator==(const hostname_endpoint& lhs, const hostname_endpoint& rhs)
	{
		return (lhs.hostname() == rhs.hostname()) && (lhs.service() == rhs.service());
	}
}

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

#include <boost/bind.hpp>

#include <cassert>

namespace freelan
{
	namespace
	{
		// Hostname labels are 63 characters long at most
		const size_t HOSTNAME_LABEL_MAX_SIZE = 63;
		// Hostnames are at most 255 characters long
		const size_t HOSTNAME_MAX_SIZE = 255;

		std::istream& putback(std::istream& is, const std::string& str)
		{
			std::ios::iostate state = is.rdstate();
			is.clear();

			std::for_each(str.rbegin(), str.rend(), boost::bind(&std::istream::putback, boost::ref(is), _1));

			is.setstate(state);

			return is;
		}

		size_t get_size(std::ostringstream& oss)
		{
			const std::streampos initial_position = oss.tellp();
			oss.seekp(0, std::ios::end);
			const size_t result = oss.tellp();
			oss.seekp(initial_position);

			return result;
		}

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

		std::istream& read_hostname_label(std::istream& is, std::string& label, size_t max_size = HOSTNAME_LABEL_MAX_SIZE)
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
					while (is.good() && (get_size(oss) < max_size) && is_hostname_label_character(is.peek()));

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
				if (is.eof())
				{
					// There is nothing more to read: lets use the content of the first label
					std::swap(hostname, label);
				}
				else
				{
					std::ostringstream oss(label);
					oss.seekp(0, std::ios::end);

					while (is.good() && (is.peek() == '.') && (get_size(oss) + 1 < HOSTNAME_MAX_SIZE))
					{
						is.ignore();

						if (!read_hostname_label(is, label, std::min(HOSTNAME_MAX_SIZE - get_size(oss) - 1, HOSTNAME_LABEL_MAX_SIZE)))
						{
							is.clear();
							is.putback('.');

							break;
						}

						oss << '.' << label;
					}

					if (is)
					{
						hostname = oss.str();
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
				if (is.good() && (is.peek() == ':'))
				{
					is.ignore();

					if (!read_service(is, service))
					{
						is.clear();
						is.putback(':');
					}
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

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

#include "stream_operations.hpp"

namespace freelan
{
	namespace
	{
		std::istream& read_hostname_service(std::istream& is, std::string& hostname, std::string& service)
		{
			if (read_hostname(is, hostname))
			{
				if (is.good() && (is.peek() == ':'))
				{
					is.ignore();

					if (!read_service(is, service))
					{
						putback(is, hostname + ':');
						is.setstate(std::ios_base::failbit);
					}
				}
			}

			return is;
		}
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

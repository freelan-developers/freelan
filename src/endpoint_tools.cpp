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
 * \file endpoint_tools.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Endpoints related functions.
 */

#include "endpoint_tools.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "endpoint.hpp"
#include "endpoint_parser.hpp"

namespace qi = boost::spirit::qi;
namespace ph = boost::phoenix;
namespace fl = freelan;

const std::string DEFAULT_PORT = "12000";

fl::configuration::ep_type parse_endpoint(
		const std::string& str,
		fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol,
		boost::asio::ip::udp::resolver::query::flags flags
		)
{
	boost::shared_ptr<endpoint> ep;

	std::string::const_iterator first = str.begin();
	bool r = qi::phrase_parse(first, str.end(), custom_parser::endpoint[ph::ref(ep) = qi::_1], qi::space);

	if (r && (first == str.end()) && ep)
	{
		try
		{
			return ep->to_boost_asio_endpoint(hostname_resolution_protocol, flags, DEFAULT_PORT);
		}
		catch (boost::system::system_error& se)
		{
			throw std::runtime_error("Unable to parse the specified endpoint: " + str + " (" + se.what() + ")");
		}
	}

	throw std::runtime_error("Unable to parse the specified endpoint: " + str);
}

std::vector<fl::configuration::ep_type> parse_endpoint_list(
		const std::string& str,
		fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol,
		boost::asio::ip::udp::resolver::query::flags flags
		)
{
	typedef std::vector<boost::shared_ptr<endpoint> > list_type;

	list_type list;

	std::string::const_iterator first = str.begin();
	bool r = qi::phrase_parse(first, str.end(), *(custom_parser::endpoint[boost::bind(&list_type::push_back, &list, _1)]), qi::space);

	if (r && (first == str.end()))
	{
		typedef std::vector<fl::configuration::ep_type> result_type;

		result_type result;

		BOOST_FOREACH(boost::shared_ptr<endpoint>& ep, list)
		{
			try
			{
				result.push_back(ep->to_boost_asio_endpoint(hostname_resolution_protocol, flags, DEFAULT_PORT));
			}
			catch (boost::system::system_error&)
			{
			}
		}

		return result;
	}

	throw std::runtime_error("Unable to parse the specified endpoint list: " + str);
}

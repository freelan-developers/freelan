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
 * \file endpoint_parser.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An endpoint parser.
 */

#ifndef ENDPOINT_PARSER_HPP
#define ENDPOINT_PARSER_HPP

#include <boost/asio.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include "endpoint.hpp"

#include "ipv4_address_parser.hpp"
#include "ipv6_address_parser.hpp"
#include "hostname_parser.hpp"

// This was written according to:
// http://boost-spirit.com/home/articles/qi-example/creating-your-own-parser-component-for-spirit-qi/

namespace custom_parser
{
	BOOST_SPIRIT_TERMINAL(endpoint)
}

namespace boost
{
	namespace spirit
	{
		template <>
		struct use_terminal<qi::domain, custom_parser::tag::endpoint> : mpl::true_
		{
		};
	}
}

namespace custom_parser
{
	struct endpoint_parser :
		boost::spirit::qi::primitive_parser<endpoint_parser>
	{
		template <typename Context, typename Iterator>
		struct attribute
		{
			typedef boost::shared_ptr< ::endpoint> type;
		};

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const
		{
			namespace qi = boost::spirit::qi;
			namespace ph = boost::phoenix;

			using custom_parser::ipv4_address;
			using custom_parser::ipv6_address;
			using custom_parser::hostname;

			qi::uint_parser<uint16_t, 10, 1, 5> port_parser;

			qi::skip_over(first, last, skipper);

			const Iterator first_save = first;
			bool r;

			boost::asio::ip::address_v6 address_v6;
			boost::asio::ip::address_v4 address_v4;
			boost::optional<uint16_t> port;
			std::string host;
			boost::optional<std::string> service;

			r = qi::parse(
					first,
					last,
					ipv6_address[ph::ref(address_v6) = qi::_1] |
					qi::no_skip[('[' >> ipv6_address[ph::ref(address_v6) = qi::_1] >> ']' >> ':' >> port_parser[ph::ref(port) = qi::_1])]
					);

			if (r)
			{
				boost::spirit::traits::assign_to(boost::make_shared<ipv6_endpoint>(address_v6, port), attr);
			} else
			{
				first = first_save;

				r = qi::parse(
						first,
						last,
						ipv4_address[ph::ref(address_v4) = qi::_1] >> qi::no_skip[-(':' >> (port_parser)[ph::ref(port) = qi::_1])]
						);

				if (r)
				{
					boost::spirit::traits::assign_to(boost::make_shared<ipv4_endpoint>(address_v4, port), attr);
				} else
				{
					first = first_save;

					std::vector<char> service_vector;

					r = qi::parse(
							first,
							last,
							hostname[ph::ref(host) = qi::_1] >> -(':' >> (qi::repeat(1, 63)[qi::alnum][ph::ref(service_vector) = qi::_1]))
							);

					if (r)
					{
						if (service_vector.size())
						{
							service = std::string(service_vector.begin(), service_vector.end());
						}

						boost::spirit::traits::assign_to(boost::make_shared<hostname_endpoint>(host, service), attr);
					}
				}
			}

			return r;
		}

		template <typename Context>
		boost::spirit::info what(Context&) const
		{
			return boost::spirit::info("endpoint");
		}
	};
}

namespace boost
{
	namespace spirit
	{
		namespace qi
		{
			template <typename Modifiers>
			struct make_primitive<custom_parser::tag::endpoint, Modifiers>
			{
				typedef custom_parser::endpoint_parser result_type;

				result_type operator()(unused_type, unused_type) const
				{
					return result_type();
				}
			};
		}
	}
}

#endif /* ENDPOINT_PARSER_HPP */

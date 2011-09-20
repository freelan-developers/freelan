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
 * \file ethernet_address_parser.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ethernet address parser.
 */

#ifndef ETHERNET_ADDRESS_PARSER_HPP
#define ETHERNET_ADDRESS_PARSER_HPP

#include <vector>

#include <boost/asio.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/optional.hpp>

#include <freelan/configuration.hpp>

// This was written according to:
// http://boost-spirit.com/home/articles/qi-example/creating-your-own-parser-component-for-spirit-qi/

namespace custom_parser
{
	BOOST_SPIRIT_TERMINAL(ethernet_address)
}

namespace boost
{
	namespace spirit
	{
		template <>
		struct use_terminal<qi::domain, custom_parser::tag::ethernet_address> : mpl::true_
		{
		};
	}
}

namespace custom_parser
{
	struct ethernet_address_parser:
		boost::spirit::qi::primitive_parser<ethernet_address_parser>
	{
		template <typename Context, typename Iterator>
		struct attribute
		{
			typedef freelan::configuration::ethernet_address_type type;
		};

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const
		{
			namespace qi = boost::spirit::qi;
			namespace ph = boost::phoenix;

			qi::uint_parser<uint8_t, 16, 2, 2> digit;

			qi::skip_over(first, last, skipper);

			const Iterator first_save = first;
			bool r;

			std::vector<uint8_t> values;

			r = qi::parse(
					first,
					last,
					qi::repeat(5, 5)[digit[boost::bind(&std::vector<uint8_t>::push_back, &values, _1)] >> -(qi::char_(':') | '-')] >>
					digit[boost::bind(&std::vector<uint8_t>::push_back, &values, _1)]
					);

			freelan::configuration::ethernet_address_type result;

			if (r && values.size() == result.size())
			{
				std::copy(result.begin(), &*values.begin(), &*values.end());
				boost::spirit::traits::assign_to(result, attr);
			}

			return r;
		}

		template <typename Context>
		boost::spirit::info what(Context&) const
		{
			return boost::spirit::info("ethernet_address");
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
			struct make_primitive<custom_parser::tag::ethernet_address, Modifiers>
			{
				typedef custom_parser::ethernet_address_parser result_type;

				result_type operator()(unused_type, unused_type) const
				{
					return result_type();
				}
			};
		}
	}
}

#endif /* ETHERNET_ADDRESS_PARSER_HPP */

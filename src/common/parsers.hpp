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
 * \file parsers.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Parsing functions.
 */

#ifndef PARSERS_HPP
#define PARSERS_HPP

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <freelan/configuration.hpp>

/**
 * \brief Parse a string.
 * \param str The string to parse.
 * \return The result of the parsing.
 *
 * If the string contains unparsed characters or if the parsing fails, an exception is thrown.
 */
template <typename Type>
Type parse(const std::string& str);

/**
 * \brief Parse an optional string.
 * \param str The string to parse.
 * \return The result of the parsing.
 *
 * If the string contains unparsed characters or if the parsing fails, an exception is thrown.
 *
 * An empty string returns boost::none.
 */
template <typename Type>
boost::optional<Type> parse_optional(const std::string& str);

/**
 * \brief Parse an IPv4 address.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v4& val);

/**
 * \brief Parse an IPv6 address.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v6& val);

/**
 * \brief Parse a port number.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, uint16_t& val);

/**
 * \brief Parse a prefix length
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, unsigned int& val);

/**
 * \brief Parse an Ethernet address.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::tap_adapter_configuration::ethernet_address_type& val);

/**
 * \brief Parse an IPv4 address and its prefix length.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::tap_adapter_configuration::ipv4_address_prefix_length_type& val);

/**
 * \brief Parse an IPv6 address and its prefix length.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::tap_adapter_configuration::ipv6_address_prefix_length_type& val);

/**
 * \brief Parse an endpoint.
 * \param begin The first character to parse.
 * \param end The last character to stop parsing at.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::fscp_configuration::ep_type& val);

template <typename Type>
inline Type parse(const std::string& str)
{
	std::string::const_iterator begin = str.begin();
	const std::string::const_iterator end = str.end();
	Type val;

	if (!parse(begin, end, val))
	{
		throw std::runtime_error("Parsing of \"" + str + "\" failed at position " + boost::lexical_cast<std::string>(std::distance(str.begin(), begin)));
	}

	if (begin != end)
	{
		throw std::runtime_error("Extra characters found in \"" + str + "\" at position " + boost::lexical_cast<std::string>(std::distance(str.begin(), begin)));
	}

	return val;
}

template <typename Type>
boost::optional<Type> parse_optional(const std::string& str)
{
	if (str.empty())
	{
		return boost::none;
	}
	else
	{
		return parse<Type>(str);
	}
}

#endif /* PARSERS_HPP */

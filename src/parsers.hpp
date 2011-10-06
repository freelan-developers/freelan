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

#include <freelan/configuration.hpp>

/**
 * \brief Parse an IPv4 address.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v4& val);

/**
 * \brief Parse an IPv6 address.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, boost::asio::ip::address_v6& val);

/**
 * \brief Parse a port number.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, uint16_t& val);

/**
 * \brief Parse a prefix length
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, size_t& val);

/**
 * \brief Parse an Ethernet address.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ethernet_address_type& val);

/**
 * \brief Parse an IPv4 address and its prefix length.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ipv4_address_prefix_length_type& val);

/**
 * \brief Parse an IPv6 address and its prefix length.
 * \param str The string to parse.
 * \param val The value to put the result into.
 * \return true if the parsing succeeded. On false, the value of val is undetermined.
 */
bool parse(std::string::const_iterator& begin, std::string::const_iterator end, freelan::configuration::ipv6_address_prefix_length_type& val);

#endif /* PARSERS_HPP */

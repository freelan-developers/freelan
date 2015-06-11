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

#include <gtest/gtest.h>

#include <sstream>

#include <boost/asio.hpp>

#include "../internal/stream_parsers.hpp"
#include "../internal/ipv4_address.hpp"
#include "../internal/ipv6_address.hpp"
#include "../internal/port_number.hpp"

using boost::asio::ip::address_v4;
using boost::asio::ip::address_v6;
using freelan::read_generic_ip_address;
using freelan::read_hostname_label;
using freelan::read_hostname;
using freelan::read_port_number;
using freelan::read_generic_ip_endpoint;
using freelan::IPv4Address;
using freelan::IPv6Address;
using freelan::PortNumber;

TEST(stream_parsers, read_ipv4_address_success) {
	const std::string str_value = "9.0.0.1";
	address_v4 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v4>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(address_v4::from_string(str_value), value);
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv4_address_extra) {
	const std::string str_value = "9.0.0.1";
	const std::string extra = "roo";
	address_v4 value;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v4>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(address_v4::from_string(str_value), value);
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_ipv4_address_invalid) {
	const std::string str_value = "9.0.f0.1";
	address_v4 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v4>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv4_address_truncated) {
	const std::string str_value = "9.0.0";
	address_v4 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v4>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_address_success) {
	const std::string str_value = "fe80::000:00:0:1";
	address_v6 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v6>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(address_v6::from_string(str_value), value);
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_address_extra) {
	const std::string str_value = "fe80::000:00:0:1";
	const std::string extra = "roo";
	address_v6 value;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v6>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(address_v6::from_string(str_value), value);
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_ipv6_address_invalid) {
	const std::string str_value = "fe8z::000:00:0:1";
	address_v6 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v6>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_address_truncated) {
	const std::string str_value = "fe8:";
	address_v6 value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_address<address_v6>(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_hostname_label_success) {
	const std::string str_value = "some-domain4people";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname_label(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_hostname_label_too_long) {
	const std::string str_value = "z123456789z123456789z123456789z123456789z123456789z123456789z1234";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname_label(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);
}

TEST(stream_parsers, read_hostname_label_only_digits) {
	const std::string str_value = "1337";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname_label(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);
}

TEST(stream_parsers, read_hostname_label_last_character_invalid) {
	const std::string str_value = "alpha-";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname_label(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);
}

TEST(stream_parsers, read_hostname_label_extra) {
	const std::string str_value = "myhost";
	const std::string extra = "_hello_";
	std::string value;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_hostname_label(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_hostname_single_label) {
	const std::string str_value = "label";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_hostname_multiple_labels) {
	const std::string str_value = "this.is.my.host4you";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_hostname_end_with_dot) {
	const std::string str_value = "this.is.my.";
	std::string value;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_hostname(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_port_number_within_range) {
	const std::string str_value = "17";
	uint16_t value = 0;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_port_number(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(std::stoi(str_value), value);
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_port_number_too_big) {
	const std::string str_value = "65536";
	uint16_t value = 0;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_port_number(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ(uint8_t(), value);
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_port_number_extra) {
	const std::string str_value = "31";
	const std::string extra = "foo";
	uint16_t value = 0;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_port_number(iss, value, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(std::stoi(str_value), value);
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_ipv4_endpoint_success) {
	const std::string str_ip_address = "9.0.0.1";
	const std::string str_port_number = "12000";
	const std::string str_value = str_ip_address + ':' + str_port_number;
	IPv4Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv4Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(IPv4Address::from_string(str_ip_address), ip_address);
	ASSERT_EQ(PortNumber::from_string(str_port_number), port_number);
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv4_endpoint_extra) {
	const std::string str_ip_address = "9.0.0.1";
	const std::string str_port_number = "12000";
	const std::string str_value = str_ip_address + ':' + str_port_number;
	const std::string extra = "roo";
	IPv4Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv4Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(IPv4Address::from_string(str_ip_address), ip_address);
	ASSERT_EQ(PortNumber::from_string(str_port_number), port_number);
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_ipv4_endpoint_invalid_ip_address) {
	const std::string str_ip_address = "9.0.f0.1";
	const std::string str_port_number = "12000";
	const std::string str_value = str_ip_address + ':' + str_port_number;
	IPv4Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv4Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv4_endpoint_invalid_port_number) {
	const std::string str_ip_address = "9.0.0.1";
	const std::string str_port_number = "g12000";
	const std::string str_value = str_ip_address + ':' + str_port_number;
	IPv4Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv4Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv4_endpoint_truncated) {
	const std::string str_ip_address = "9.0.0.1";
	const std::string str_port_number = "";
	const std::string str_value = str_ip_address + ':' + str_port_number;
	IPv4Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv4Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_endpoint_success) {
	const std::string str_ip_address = "ff02:1001::e0:abcd";
	const std::string str_port_number = "12000";
	const std::string str_value = '[' + str_ip_address + ']' + ':' + str_port_number;
	IPv6Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv6Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(IPv6Address::from_string(str_ip_address), ip_address);
	ASSERT_EQ(PortNumber::from_string(str_port_number), port_number);
	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_endpoint_extra) {
	const std::string str_ip_address = "ff02:1001::e0:abcd";
	const std::string str_port_number = "12000";
	const std::string str_value = '[' + str_ip_address + ']' + ':' + str_port_number;
	const std::string extra = "roo";
	IPv6Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value + extra);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv6Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_TRUE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_FALSE(iss.fail());
	ASSERT_EQ(IPv6Address::from_string(str_ip_address), ip_address);
	ASSERT_EQ(PortNumber::from_string(str_port_number), port_number);
	ASSERT_EQ(str_value, parsed_str);

	// Make sure the stream wasn't eaten up.
	iss >> parsed_str;

	ASSERT_EQ(extra, parsed_str);
}

TEST(stream_parsers, read_ipv6_endpoint_invalid_ip_address) {
	const std::string str_ip_address = "ff02:1001:-:e0:abcd";
	const std::string str_port_number = "12000";
	const std::string str_value = '[' + str_ip_address + ']' + ':' + str_port_number;
	IPv6Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv6Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_endpoint_invalid_port_number) {
	const std::string str_ip_address = "ff02:1001::e0:abcd";
	const std::string str_port_number = "g12000";
	const std::string str_value = '[' + str_ip_address + ']' + ':' + str_port_number;
	IPv6Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv6Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

TEST(stream_parsers, read_ipv6_endpoint_truncated) {
	const std::string str_ip_address = "ff02:1001::e0:abcd";
	const std::string str_port_number = "";
	const std::string str_value = '[' + str_ip_address + ']' + ':' + str_port_number;
	IPv6Address ip_address;
	PortNumber port_number;
	std::istringstream iss(str_value);
	std::string parsed_str;

	auto&& result = read_generic_ip_endpoint<IPv6Address>(iss, ip_address, port_number, &parsed_str);

	ASSERT_EQ(&iss, &result);
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.eof());
	ASSERT_TRUE(iss.fail());
	ASSERT_EQ("", parsed_str);

	// Make sure the stream wasn't eaten up.
	iss.clear();
	iss >> parsed_str;

	ASSERT_EQ(str_value, parsed_str);
}

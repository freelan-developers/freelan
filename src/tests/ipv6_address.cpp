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

#include "../internal/ipv6_address.hpp"
#include "../internal/common.hpp"

using boost::asio::ip::address_v6;

using freelan::IPv6Address;
using freelan::from_string;

TEST(IPv6Address, default_instantiation) {
	const IPv6Address value {};
}

TEST(IPv6Address, boost_asio_ip_address_v6_instantiation) {
	const address_v6 raw_value;
	const IPv6Address value { raw_value };

	ASSERT_EQ(raw_value, value.to_raw_value());
}

TEST(IPv6Address, string_instantiation) {
	const std::string str_value = "ff02:1001::e0:abcd";
	const auto value = IPv6Address::from_string(str_value);

	ASSERT_EQ(str_value, value.to_string());
}

TEST(IPv6Address, string_instantiation_failure) {
	try {
		IPv6Address::from_string("invalid");
	} catch (boost::system::system_error& ex) {
		ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ex.code());
	}
}

TEST(IPv6Address, string_instantiation_failure_no_throw) {
	boost::system::error_code ec;
	const auto value = IPv6Address::from_string("invalid", ec);

	ASSERT_EQ(IPv6Address(), value);
	ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ec);
}

TEST(IPv6Address, bytes_instantiation) {
	const IPv6Address::bytes_type bytes_value {{
		0xff, 0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xab, 0xcd,
	}};
	const auto value = IPv6Address::from_bytes(bytes_value);

	ASSERT_EQ(bytes_value, value.to_bytes());
}

TEST(IPv6Address, implicit_string_conversion) {
	const std::string str_value = "ff02:1001::e0:abcd";
	const auto value = from_string<IPv6Address>(str_value);

	ASSERT_EQ(str_value, to_string(value));
}

TEST(IPv6Address, compare_to_same_instance) {
	const auto value = from_string<IPv6Address>("ff02:1001::e0:abcd");

	ASSERT_TRUE(value == value);
	ASSERT_FALSE(value != value);
	ASSERT_FALSE(value < value);
	ASSERT_TRUE(value <= value);
	ASSERT_FALSE(value > value);
	ASSERT_TRUE(value >= value);
}

TEST(IPv6Address, compare_to_same_value) {
	const auto value_a = from_string<IPv6Address>("ff02:1001::e0:abcd");
	const auto value_b = from_string<IPv6Address>("ff02:1001::e0:abcd");

	ASSERT_TRUE(value_a == value_b);
	ASSERT_FALSE(value_a != value_b);
	ASSERT_FALSE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_TRUE(value_a >= value_b);
}

TEST(IPv6Address, compare_to_different_values) {
	const auto value_a = from_string<IPv6Address>("ff02:1001::e0:abcd");
	const auto value_b = from_string<IPv6Address>("ff02:1001::e0:abce");

	ASSERT_FALSE(value_a == value_b);
	ASSERT_TRUE(value_a != value_b);
	ASSERT_TRUE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_FALSE(value_a >= value_b);
}

TEST(IPv6Address, stream_input) {
	const std::string str_value = "ff02:1001::e0:abcd";
	const auto value_ref = from_string<IPv6Address>(str_value);

	std::istringstream iss(str_value);
	IPv6Address value;

	iss >> value;

	ASSERT_EQ(value_ref, value);
	ASSERT_TRUE(iss.eof());
	ASSERT_FALSE(iss.good());
	ASSERT_FALSE(iss.fail());
}

TEST(IPv6Address, stream_output) {
	const std::string str_value = "ff02:1001::e0:abcd";
	const auto value = from_string<IPv6Address>(str_value);

	std::ostringstream oss;
	oss << value;

	ASSERT_EQ(str_value, oss.str());
}

TEST(IPv6Address, increment) {
	auto value = IPv6Address::from_string("ff02::a:0003");

	ASSERT_EQ(IPv6Address::from_string("ff02::a:0004"), ++value);
}

TEST(IPv6Address, increment_byte_boundary) {
	auto value = IPv6Address::from_string("ff02::a:00ff");

	ASSERT_EQ(IPv6Address::from_string("ff02::a:0100"), ++value);
}

TEST(IPv6Address, increment_last) {
	auto value = IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

	ASSERT_EQ(IPv6Address::from_string("::"), ++value);
}

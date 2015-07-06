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

#include "../internal/ipv6_address.hpp"
#include "../internal/ipv6_prefix_length.hpp"
#include "../internal/ipv6_route.hpp"
#include "../internal/common.hpp"

using freelan::IPv6Address;
using freelan::IPv6PrefixLength;
using freelan::IPv6Route;
using freelan::from_string;

TEST(IPv6Route, default_instantiation) {
	const IPv6Route value {};
}

TEST(IPv6Route, value_instanciation) {
	const IPv6Address ipv6_address = IPv6Address::from_string("ff02:1001::e0:0");
	const IPv6PrefixLength prefix_length = 120;
	const IPv6Route value { ipv6_address, prefix_length };

	ASSERT_EQ(ipv6_address, value.get_ip_address());
	ASSERT_EQ(prefix_length, value.get_prefix_length());
}

TEST(IPv6Route, string_instantiation) {
	const std::string str_value = "ff02:1001::e0:0/120";
	const auto value = IPv6Route::from_string(str_value);

	ASSERT_EQ(str_value, value.to_string());
}

TEST(IPv6Route, string_instantiation_failure) {
	try {
		IPv6Route::from_string("invalid");
	} catch (boost::system::system_error& ex) {
		ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ex.code());
	}
}

TEST(IPv6Route, string_instantiation_failure_no_throw) {
	boost::system::error_code ec;
	const auto value = IPv6Route::from_string("invalid", ec);

	ASSERT_EQ(IPv6Route(), value);
	ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ec);
}

TEST(IPv6Route, implicit_string_conversion) {
	const std::string str_value = "ff02:1001::e0:0/120";
	const auto value = from_string<IPv6Route>(str_value);

	ASSERT_EQ(str_value, to_string(value));
}

TEST(IPv6Route, compare_to_same_instance) {
	const auto value = from_string<IPv6Route>("ff02:1001::e0:ab00/124");

	ASSERT_TRUE(value == value);
	ASSERT_FALSE(value != value);
	ASSERT_FALSE(value < value);
	ASSERT_TRUE(value <= value);
	ASSERT_FALSE(value > value);
	ASSERT_TRUE(value >= value);
}

TEST(IPv6Route, compare_to_same_value) {
	const auto value_a = from_string<IPv6Route>("ff02:1001::e0:abc0/124");
	const auto value_b = from_string<IPv6Route>("ff02:1001::e0:abcd/124");

	ASSERT_TRUE(value_a == value_b);
	ASSERT_FALSE(value_a != value_b);
	ASSERT_FALSE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_TRUE(value_a >= value_b);
}

TEST(IPv6Route, compare_to_different_values) {
	const auto value_a = from_string<IPv6Route>("ff02:1001::e0:abcf/124");
	const auto value_b = from_string<IPv6Route>("ff02:1001::e0:abdf/124");

	ASSERT_FALSE(value_a == value_b);
	ASSERT_TRUE(value_a != value_b);
	ASSERT_TRUE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_FALSE(value_a >= value_b);
}

TEST(IPv6Route, stream_input) {
	const std::string str_value = "ff02:1001::e0:0/120";
	const auto value_ref = from_string<IPv6Route>(str_value);

	std::istringstream iss(str_value);
	IPv6Route value;

	iss >> value;

	ASSERT_EQ(value_ref, value);
	ASSERT_TRUE(iss.eof());
	ASSERT_TRUE(!iss.good());
	ASSERT_TRUE(!iss.fail());
}

TEST(IPv6Route, stream_output) {
	const std::string str_value = "ff02:1001::e0:0/120";
	const auto value = from_string<IPv6Route>(str_value);

	std::ostringstream oss;
	oss << value;

	ASSERT_EQ(str_value, oss.str());
}

TEST(IPv6Route, ordering) {
	const auto value_a = IPv6Route::from_string("ff02:1001::e0:0/120");
	const auto value_b = IPv6Route::from_string("ff03:1001::e0:0/120");
	const auto value_c = IPv6Route::from_string("ff02:1001::e0:0/112");
	const auto value_d = IPv6Route::from_string("::/0");

	ASSERT_LT(value_a, value_b);
	ASSERT_LT(value_a, value_c);
	ASSERT_LT(value_a, value_d);
	ASSERT_LT(value_b, value_c);
	ASSERT_LT(value_b, value_d);
	ASSERT_LT(value_c, value_d);
}

TEST(IPv6Route, netmask_limits) {
	const IPv6Address ipv6_address = IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 0).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("8000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 1).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("c000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 2).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("e000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 3).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("f000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 4).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("f800:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 5).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fc00:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 6).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fe00:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 7).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ff00:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 8).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ff80:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 9).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffc0:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 10).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffe0:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 11).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fff0:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 12).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fff8:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 13).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fffc:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 14).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("fffe:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 15).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 16).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:8000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 17).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:c000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 18).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:e000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 19).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:f000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 20).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:f800:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 21).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fc00:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 22).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fe00:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 23).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ff00:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 24).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ff80:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 25).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffc0:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 26).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffe0:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 27).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fff0:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 28).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fff8:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 29).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fffc:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 30).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:fffe:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 31).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 32).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:8000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 33).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:c000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 34).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:e000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 35).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:f000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 36).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:f800:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 37).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fc00:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 38).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fe00:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 39).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ff00:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 40).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ff80:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 41).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffc0:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 42).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffe0:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 43).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fff0:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 44).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fff8:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 45).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fffc:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 46).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:fffe:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 47).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 48).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:8000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 49).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:c000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 50).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:e000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 51).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:f000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 52).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:f800:0000:0000:0000:0000"), IPv6Route(ipv6_address, 53).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fc00:0000:0000:0000:0000"), IPv6Route(ipv6_address, 54).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fe00:0000:0000:0000:0000"), IPv6Route(ipv6_address, 55).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ff00:0000:0000:0000:0000"), IPv6Route(ipv6_address, 56).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ff80:0000:0000:0000:0000"), IPv6Route(ipv6_address, 57).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffc0:0000:0000:0000:0000"), IPv6Route(ipv6_address, 58).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffe0:0000:0000:0000:0000"), IPv6Route(ipv6_address, 59).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fff0:0000:0000:0000:0000"), IPv6Route(ipv6_address, 60).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fff8:0000:0000:0000:0000"), IPv6Route(ipv6_address, 61).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fffc:0000:0000:0000:0000"), IPv6Route(ipv6_address, 62).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:fffe:0000:0000:0000:0000"), IPv6Route(ipv6_address, 63).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:0000:0000:0000:0000"), IPv6Route(ipv6_address, 64).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:8000:0000:0000:0000"), IPv6Route(ipv6_address, 65).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:c000:0000:0000:0000"), IPv6Route(ipv6_address, 66).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:e000:0000:0000:0000"), IPv6Route(ipv6_address, 67).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:f000:0000:0000:0000"), IPv6Route(ipv6_address, 68).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:f800:0000:0000:0000"), IPv6Route(ipv6_address, 69).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fc00:0000:0000:0000"), IPv6Route(ipv6_address, 70).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fe00:0000:0000:0000"), IPv6Route(ipv6_address, 71).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ff00:0000:0000:0000"), IPv6Route(ipv6_address, 72).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ff80:0000:0000:0000"), IPv6Route(ipv6_address, 73).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffc0:0000:0000:0000"), IPv6Route(ipv6_address, 74).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffe0:0000:0000:0000"), IPv6Route(ipv6_address, 75).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fff0:0000:0000:0000"), IPv6Route(ipv6_address, 76).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fff8:0000:0000:0000"), IPv6Route(ipv6_address, 77).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fffc:0000:0000:0000"), IPv6Route(ipv6_address, 78).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:fffe:0000:0000:0000"), IPv6Route(ipv6_address, 79).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:0000:0000:0000"), IPv6Route(ipv6_address, 80).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:8000:0000:0000"), IPv6Route(ipv6_address, 81).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:c000:0000:0000"), IPv6Route(ipv6_address, 82).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:e000:0000:0000"), IPv6Route(ipv6_address, 83).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:f000:0000:0000"), IPv6Route(ipv6_address, 84).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:f800:0000:0000"), IPv6Route(ipv6_address, 85).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fc00:0000:0000"), IPv6Route(ipv6_address, 86).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fe00:0000:0000"), IPv6Route(ipv6_address, 87).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ff00:0000:0000"), IPv6Route(ipv6_address, 88).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ff80:0000:0000"), IPv6Route(ipv6_address, 89).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffc0:0000:0000"), IPv6Route(ipv6_address, 90).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffe0:0000:0000"), IPv6Route(ipv6_address, 91).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fff0:0000:0000"), IPv6Route(ipv6_address, 92).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fff8:0000:0000"), IPv6Route(ipv6_address, 93).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fffc:0000:0000"), IPv6Route(ipv6_address, 94).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:fffe:0000:0000"), IPv6Route(ipv6_address, 95).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:0000:0000"), IPv6Route(ipv6_address, 96).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:8000:0000"), IPv6Route(ipv6_address, 97).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:c000:0000"), IPv6Route(ipv6_address, 98).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:e000:0000"), IPv6Route(ipv6_address, 99).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:f000:0000"), IPv6Route(ipv6_address, 100).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:f800:0000"), IPv6Route(ipv6_address, 101).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fc00:0000"), IPv6Route(ipv6_address, 102).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fe00:0000"), IPv6Route(ipv6_address, 103).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ff00:0000"), IPv6Route(ipv6_address, 104).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ff80:0000"), IPv6Route(ipv6_address, 105).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffc0:0000"), IPv6Route(ipv6_address, 106).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffe0:0000"), IPv6Route(ipv6_address, 107).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fff0:0000"), IPv6Route(ipv6_address, 108).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fff8:0000"), IPv6Route(ipv6_address, 109).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fffc:0000"), IPv6Route(ipv6_address, 110).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:fffe:0000"), IPv6Route(ipv6_address, 111).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:0000"), IPv6Route(ipv6_address, 112).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:8000"), IPv6Route(ipv6_address, 113).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:c000"), IPv6Route(ipv6_address, 114).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:e000"), IPv6Route(ipv6_address, 115).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:f000"), IPv6Route(ipv6_address, 116).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:f800"), IPv6Route(ipv6_address, 117).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fc00"), IPv6Route(ipv6_address, 118).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fe00"), IPv6Route(ipv6_address, 119).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ff00"), IPv6Route(ipv6_address, 120).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ff80"), IPv6Route(ipv6_address, 121).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffc0"), IPv6Route(ipv6_address, 122).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffe0"), IPv6Route(ipv6_address, 123).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff0"), IPv6Route(ipv6_address, 124).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff8"), IPv6Route(ipv6_address, 125).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffc"), IPv6Route(ipv6_address, 126).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"), IPv6Route(ipv6_address, 127).get_ip_address());
	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 128).get_ip_address());
}

TEST(IPv6Route, get_broadcast_ip_address) {
	const IPv6Address ipv6_address = IPv6Address::from_string("::");

	ASSERT_EQ(IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 0).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("7fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 1).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("3fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 2).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("1fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 3).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 4).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("07ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 5).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("03ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 6).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("01ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 7).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("00ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 8).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("007f:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 9).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("003f:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 10).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("001f:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 11).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("000f:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 12).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0007:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 13).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0003:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 14).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0001:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 15).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 16).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:7fff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 17).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:3fff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 18).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:1fff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 19).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0fff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 20).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:07ff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 21).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:03ff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 22).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:01ff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 23).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:00ff:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 24).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:007f:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 25).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:003f:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 26).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:001f:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 27).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:000f:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 28).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0007:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 29).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0003:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 30).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0001:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 31).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:ffff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 32).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:7fff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 33).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:3fff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 34).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:1fff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 35).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0fff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 36).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:07ff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 37).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:03ff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 38).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:01ff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 39).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:00ff:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 40).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:007f:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 41).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:003f:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 42).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:001f:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 43).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:000f:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 44).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0007:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 45).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0003:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 46).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0001:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 47).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:ffff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 48).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:7fff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 49).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:3fff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 50).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:1fff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 51).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0fff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 52).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:07ff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 53).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:03ff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 54).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:01ff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 55).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:00ff:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 56).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:007f:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 57).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:003f:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 58).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:001f:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 59).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:000f:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 60).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0007:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 61).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0003:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 62).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0001:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 63).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:ffff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 64).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:7fff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 65).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:3fff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 66).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:1fff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 67).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0fff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 68).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:07ff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 69).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:03ff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 70).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:01ff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 71).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:00ff:ffff:ffff:ffff"), IPv6Route(ipv6_address, 72).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:007f:ffff:ffff:ffff"), IPv6Route(ipv6_address, 73).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:003f:ffff:ffff:ffff"), IPv6Route(ipv6_address, 74).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:001f:ffff:ffff:ffff"), IPv6Route(ipv6_address, 75).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:000f:ffff:ffff:ffff"), IPv6Route(ipv6_address, 76).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0007:ffff:ffff:ffff"), IPv6Route(ipv6_address, 77).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0003:ffff:ffff:ffff"), IPv6Route(ipv6_address, 78).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0001:ffff:ffff:ffff"), IPv6Route(ipv6_address, 79).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:ffff:ffff:ffff"), IPv6Route(ipv6_address, 80).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:7fff:ffff:ffff"), IPv6Route(ipv6_address, 81).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:3fff:ffff:ffff"), IPv6Route(ipv6_address, 82).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:1fff:ffff:ffff"), IPv6Route(ipv6_address, 83).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0fff:ffff:ffff"), IPv6Route(ipv6_address, 84).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:07ff:ffff:ffff"), IPv6Route(ipv6_address, 85).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:03ff:ffff:ffff"), IPv6Route(ipv6_address, 86).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:01ff:ffff:ffff"), IPv6Route(ipv6_address, 87).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:00ff:ffff:ffff"), IPv6Route(ipv6_address, 88).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:007f:ffff:ffff"), IPv6Route(ipv6_address, 89).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:003f:ffff:ffff"), IPv6Route(ipv6_address, 90).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:001f:ffff:ffff"), IPv6Route(ipv6_address, 91).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:000f:ffff:ffff"), IPv6Route(ipv6_address, 92).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0007:ffff:ffff"), IPv6Route(ipv6_address, 93).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0003:ffff:ffff"), IPv6Route(ipv6_address, 94).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0001:ffff:ffff"), IPv6Route(ipv6_address, 95).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:ffff:ffff"), IPv6Route(ipv6_address, 96).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:7fff:ffff"), IPv6Route(ipv6_address, 97).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:3fff:ffff"), IPv6Route(ipv6_address, 98).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:1fff:ffff"), IPv6Route(ipv6_address, 99).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0fff:ffff"), IPv6Route(ipv6_address, 100).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:07ff:ffff"), IPv6Route(ipv6_address, 101).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:03ff:ffff"), IPv6Route(ipv6_address, 102).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:01ff:ffff"), IPv6Route(ipv6_address, 103).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:00ff:ffff"), IPv6Route(ipv6_address, 104).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:007f:ffff"), IPv6Route(ipv6_address, 105).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:003f:ffff"), IPv6Route(ipv6_address, 106).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:001f:ffff"), IPv6Route(ipv6_address, 107).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:000f:ffff"), IPv6Route(ipv6_address, 108).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0007:ffff"), IPv6Route(ipv6_address, 109).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0003:ffff"), IPv6Route(ipv6_address, 110).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0001:ffff"), IPv6Route(ipv6_address, 111).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:ffff"), IPv6Route(ipv6_address, 112).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:7fff"), IPv6Route(ipv6_address, 113).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:3fff"), IPv6Route(ipv6_address, 114).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:1fff"), IPv6Route(ipv6_address, 115).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0fff"), IPv6Route(ipv6_address, 116).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:07ff"), IPv6Route(ipv6_address, 117).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:03ff"), IPv6Route(ipv6_address, 118).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:01ff"), IPv6Route(ipv6_address, 119).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:00ff"), IPv6Route(ipv6_address, 120).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:007f"), IPv6Route(ipv6_address, 121).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:003f"), IPv6Route(ipv6_address, 122).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:001f"), IPv6Route(ipv6_address, 123).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:000f"), IPv6Route(ipv6_address, 124).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0007"), IPv6Route(ipv6_address, 125).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0003"), IPv6Route(ipv6_address, 126).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0001"), IPv6Route(ipv6_address, 127).get_broadcast_ip_address());
	ASSERT_EQ(IPv6Address::from_string("0000:0000:0000:0000:0000:0000:0000:0000"), IPv6Route(ipv6_address, 128).get_broadcast_ip_address());
}

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

#include "../internal/ipv4_address.hpp"
#include "../internal/ipv4_prefix_length.hpp"
#include "../internal/ipv4_route.hpp"
#include "../internal/common.hpp"

using freelan::IPv4Address;
using freelan::IPv4PrefixLength;
using freelan::IPv4Route;
using freelan::from_string;

TEST(IPv4Route, default_instantiation) {
	const IPv4Route value {};
}

TEST(IPv4Route, value_instanciation) {
	const IPv4Address ipv4_address = IPv4Address::from_string("9.0.0.1");
	const IPv4PrefixLength prefix_length = 32;
	const IPv4Route value { ipv4_address, prefix_length };

	ASSERT_EQ(ipv4_address, value.get_ip_address());
	ASSERT_EQ(prefix_length, value.get_prefix_length());
	ASSERT_FALSE(value.has_gateway());
}

TEST(IPv4Route, value_instanciation_with_gateway) {
	const IPv4Address ipv4_address = IPv4Address::from_string("9.0.0.1");
	const IPv4PrefixLength prefix_length = 32;
	const IPv4Address gateway = IPv4Address::from_string("9.0.0.255");
	const IPv4Route value { ipv4_address, prefix_length, gateway };

	ASSERT_EQ(ipv4_address, value.get_ip_address());
	ASSERT_EQ(prefix_length, value.get_prefix_length());
	ASSERT_TRUE(value.has_gateway());
	ASSERT_EQ(gateway, value.get_gateway());
}

TEST(IPv4Route, string_instantiation) {
	const std::string str_value = "9.0.255.0/24@9.0.0.254";
	const auto value = IPv4Route::from_string(str_value);

	ASSERT_EQ(str_value, value.to_string());
}

TEST(IPv4Route, string_instantiation_failure) {
	try {
		IPv4Route::from_string("invalid");
	} catch (boost::system::system_error& ex) {
		ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ex.code());
	}
}

TEST(IPv4Route, string_instantiation_failure_no_throw) {
	boost::system::error_code ec;
	const auto value = IPv4Route::from_string("invalid", ec);

	ASSERT_EQ(IPv4Route(), value);
	ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ec);
}

TEST(IPv4Route, implicit_string_conversion) {
	const std::string str_value = "9.0.255.0/24";
	const auto value = from_string<IPv4Route>(str_value);

	ASSERT_EQ(str_value, to_string(value));
}

TEST(IPv4Route, compare_to_same_instance) {
	const auto value = from_string<IPv4Route>("9.0.0.0/24");

	ASSERT_TRUE(value == value);
	ASSERT_FALSE(value != value);
	ASSERT_FALSE(value < value);
	ASSERT_TRUE(value <= value);
	ASSERT_FALSE(value > value);
	ASSERT_TRUE(value >= value);
}

TEST(IPv4Route, compare_to_same_value) {
	const auto value_a = from_string<IPv4Route>("9.0.0.0/24");
	const auto value_b = from_string<IPv4Route>("9.0.0.255/24");

	ASSERT_TRUE(value_a == value_b);
	ASSERT_FALSE(value_a != value_b);
	ASSERT_FALSE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_TRUE(value_a >= value_b);
}

TEST(IPv4Route, compare_to_different_values) {
	const auto value_a = from_string<IPv4Route>("9.0.1.0/24");
	const auto value_b = from_string<IPv4Route>("9.0.2.0/24");

	ASSERT_FALSE(value_a == value_b);
	ASSERT_TRUE(value_a != value_b);
	ASSERT_TRUE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_FALSE(value_a >= value_b);
}

TEST(IPv4Route, stream_input) {
	const std::string str_value = "9.0.0.0/24";
	const auto value_ref = from_string<IPv4Route>(str_value);

	std::istringstream iss(str_value);
	IPv4Route value;

	iss >> value;

	ASSERT_EQ(value_ref, value);
	ASSERT_TRUE(iss.eof());
	ASSERT_TRUE(!iss.good());
	ASSERT_TRUE(!iss.fail());
}

TEST(IPv4Route, stream_output) {
	const std::string str_value = "9.0.0.0/24";
	const auto value = from_string<IPv4Route>(str_value);

	std::ostringstream oss;
	oss << value;

	ASSERT_EQ(str_value, oss.str());
}

TEST(IPv4Route, ordering) {
	const auto value_a = IPv4Route::from_string("9.0.0.0/24");
	const auto value_b = IPv4Route::from_string("10.0.0.0/24");
	const auto value_c = IPv4Route::from_string("9.0.0.0/16");
	const auto value_d = IPv4Route::from_string("0.0.0.0/0");

	ASSERT_LT(value_a, value_b);
	ASSERT_LT(value_a, value_c);
	ASSERT_LT(value_a, value_d);
	ASSERT_LT(value_b, value_c);
	ASSERT_LT(value_b, value_d);
	ASSERT_LT(value_c, value_d);
}

TEST(IPv4Route, netmask_limits) {
	const IPv4Address ipv4_address = IPv4Address::from_string("255.255.255.255");

	ASSERT_EQ(IPv4Address::from_string("0.0.0.0"), IPv4Route(ipv4_address, 0).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("128.0.0.0"), IPv4Route(ipv4_address, 1).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("192.0.0.0"), IPv4Route(ipv4_address, 2).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("224.0.0.0"), IPv4Route(ipv4_address, 3).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("240.0.0.0"), IPv4Route(ipv4_address, 4).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("248.0.0.0"), IPv4Route(ipv4_address, 5).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("252.0.0.0"), IPv4Route(ipv4_address, 6).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("254.0.0.0"), IPv4Route(ipv4_address, 7).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.0.0.0"), IPv4Route(ipv4_address, 8).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.128.0.0"), IPv4Route(ipv4_address, 9).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.192.0.0"), IPv4Route(ipv4_address, 10).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.224.0.0"), IPv4Route(ipv4_address, 11).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.240.0.0"), IPv4Route(ipv4_address, 12).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.248.0.0"), IPv4Route(ipv4_address, 13).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.252.0.0"), IPv4Route(ipv4_address, 14).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.254.0.0"), IPv4Route(ipv4_address, 15).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.0.0"), IPv4Route(ipv4_address, 16).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.128.0"), IPv4Route(ipv4_address, 17).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.192.0"), IPv4Route(ipv4_address, 18).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.224.0"), IPv4Route(ipv4_address, 19).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.240.0"), IPv4Route(ipv4_address, 20).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.248.0"), IPv4Route(ipv4_address, 21).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.252.0"), IPv4Route(ipv4_address, 22).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.254.0"), IPv4Route(ipv4_address, 23).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.0"), IPv4Route(ipv4_address, 24).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.128"), IPv4Route(ipv4_address, 25).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.192"), IPv4Route(ipv4_address, 26).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.224"), IPv4Route(ipv4_address, 27).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.240"), IPv4Route(ipv4_address, 28).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.248"), IPv4Route(ipv4_address, 29).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.252"), IPv4Route(ipv4_address, 30).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.254"), IPv4Route(ipv4_address, 31).get_ip_address());
	ASSERT_EQ(IPv4Address::from_string("255.255.255.255"), IPv4Route(ipv4_address, 32).get_ip_address());
}

TEST(IPv4Route, get_broadcast_ip_address) {
	const IPv4Address ipv4_address = IPv4Address::from_string("0.0.0.0");

	ASSERT_EQ(IPv4Address::from_string("255.255.255.255"), IPv4Route(ipv4_address, 0).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("127.255.255.255"), IPv4Route(ipv4_address, 1).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("63.255.255.255"), IPv4Route(ipv4_address, 2).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("31.255.255.255"), IPv4Route(ipv4_address, 3).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("15.255.255.255"), IPv4Route(ipv4_address, 4).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("7.255.255.255"), IPv4Route(ipv4_address, 5).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("3.255.255.255"), IPv4Route(ipv4_address, 6).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("1.255.255.255"), IPv4Route(ipv4_address, 7).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.255.255.255"), IPv4Route(ipv4_address, 8).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.127.255.255"), IPv4Route(ipv4_address, 9).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.63.255.255"), IPv4Route(ipv4_address, 10).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.31.255.255"), IPv4Route(ipv4_address, 11).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.15.255.255"), IPv4Route(ipv4_address, 12).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.7.255.255"), IPv4Route(ipv4_address, 13).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.3.255.255"), IPv4Route(ipv4_address, 14).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.1.255.255"), IPv4Route(ipv4_address, 15).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.255.255"), IPv4Route(ipv4_address, 16).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.127.255"), IPv4Route(ipv4_address, 17).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.63.255"), IPv4Route(ipv4_address, 18).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.31.255"), IPv4Route(ipv4_address, 19).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.15.255"), IPv4Route(ipv4_address, 20).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.7.255"), IPv4Route(ipv4_address, 21).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.3.255"), IPv4Route(ipv4_address, 22).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.1.255"), IPv4Route(ipv4_address, 23).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.255"), IPv4Route(ipv4_address, 24).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.127"), IPv4Route(ipv4_address, 25).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.63"), IPv4Route(ipv4_address, 26).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.31"), IPv4Route(ipv4_address, 27).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.15"), IPv4Route(ipv4_address, 28).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.7"), IPv4Route(ipv4_address, 29).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.3"), IPv4Route(ipv4_address, 30).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.1"), IPv4Route(ipv4_address, 31).get_broadcast_ip_address());
	ASSERT_EQ(IPv4Address::from_string("0.0.0.0"), IPv4Route(ipv4_address, 32).get_broadcast_ip_address());
}

TEST(IPv4Route, iteration) {
	const auto value = IPv4Route::from_string("1.2.3.0/29");
	auto it = value.begin();

	ASSERT_EQ(IPv4Address::from_string("1.2.3.1"), *it); ++it;
	ASSERT_EQ(IPv4Address::from_string("1.2.3.2"), *it); ++it;
	ASSERT_EQ(IPv4Address::from_string("1.2.3.3"), *it); ++it;
	ASSERT_EQ(IPv4Address::from_string("1.2.3.4"), *it); ++it;
	ASSERT_EQ(IPv4Address::from_string("1.2.3.5"), *it); ++it;
	ASSERT_EQ(IPv4Address::from_string("1.2.3.6"), *it); ++it;
	ASSERT_TRUE(value.end() == it) << "it should be the 'end' iterator";

	ASSERT_EQ(static_cast<ssize_t>(6), std::distance(value.begin(), value.end()));
}

TEST(IPv4Route, contains_ip_address) {
	const auto value = IPv4Route::from_string("1.2.3.4/30");

	ASSERT_FALSE(value.contains_ip_address(IPv4Address::from_string("1.2.3.3")));
	ASSERT_TRUE(value.contains_ip_address(IPv4Address::from_string("1.2.3.4")));
	ASSERT_TRUE(value.contains_ip_address(IPv4Address::from_string("1.2.3.5")));
	ASSERT_TRUE(value.contains_ip_address(IPv4Address::from_string("1.2.3.6")));
	ASSERT_TRUE(value.contains_ip_address(IPv4Address::from_string("1.2.3.7")));
	ASSERT_FALSE(value.contains_ip_address(IPv4Address::from_string("1.2.3.8")));
}

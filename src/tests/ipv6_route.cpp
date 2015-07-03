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

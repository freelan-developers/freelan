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

#include "../internal/common.hpp"
#include "../internal/types/ethernet_address.hpp"

using freelan::EthernetAddress;
using freelan::from_string;

TEST(EthernetAddress, default_instantiation) {
	const EthernetAddress value {};
}

TEST(EthernetAddress, bytes_instantiation) {
	const std::string str_value = "ab:cd:ef:12:34:56";
	const EthernetAddress::value_type ref_value{{0xab, 0xcd, 0xef, 0x12, 0x34, 0x56}};
	const auto value = EthernetAddress::from_bytes(ref_value.data());

	ASSERT_EQ(str_value, value.to_string());
}

TEST(EthernetAddress, string_instantiation) {
	const std::string str_value = "ab:cd:ef:12:34:56";
	const auto value = EthernetAddress::from_string(str_value);

	ASSERT_EQ(str_value, value.to_string());
}

TEST(EthernetAddress, string_instantiation_failure) {
	try {
		EthernetAddress::from_string("ab:cd:ef:12:34:56a");

		FAIL();
	} catch (boost::system::system_error& ex) {
		ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ex.code());
	}
}

TEST(EthernetAddress, string_instantiation_failure_no_throw) {
	boost::system::error_code ec;
	const auto value = EthernetAddress::from_string("ab:cd:ef:12:34:56a", ec);

	ASSERT_EQ(EthernetAddress(), value);
	ASSERT_EQ(make_error_condition(boost::system::errc::invalid_argument), ec);
}

TEST(EthernetAddress, implicit_string_conversion) {
	const std::string str_value = "ab:cd:ef:12:34:56";
	const auto value = from_string<EthernetAddress>(str_value);

	ASSERT_EQ(str_value, to_string(value));
}

TEST(EthernetAddress, compare_to_same_instance) {
	const auto value = from_string<EthernetAddress>("ab:cd:ef:12:34:56");

	ASSERT_TRUE(value == value);
	ASSERT_FALSE(value != value);
	ASSERT_FALSE(value < value);
	ASSERT_TRUE(value <= value);
	ASSERT_FALSE(value > value);
	ASSERT_TRUE(value >= value);
}

TEST(EthernetAddress, compare_to_same_value) {
	const auto value_a = from_string<EthernetAddress>("ab:cd:ef:12:34:56");
	const auto value_b = from_string<EthernetAddress>("ab:cd:ef:12:34:56");

	ASSERT_TRUE(value_a == value_b);
	ASSERT_FALSE(value_a != value_b);
	ASSERT_FALSE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_TRUE(value_a >= value_b);
}

TEST(EthernetAddress, compare_to_different_values) {
	const auto value_a = from_string<EthernetAddress>("ab:cd:ef:12:34:56");
	const auto value_b = from_string<EthernetAddress>("ab:cd:ef:12:34:57");

	ASSERT_FALSE(value_a == value_b);
	ASSERT_TRUE(value_a != value_b);
	ASSERT_TRUE(value_a < value_b);
	ASSERT_TRUE(value_a <= value_b);
	ASSERT_FALSE(value_a > value_b);
	ASSERT_FALSE(value_a >= value_b);
}

TEST(EthernetAddress, stream_input) {
	const std::string str_value = "ab:cd:ef:12:34:56";
	const auto value_ref = from_string<EthernetAddress>(str_value);

	std::istringstream iss(str_value);
	EthernetAddress value;

	iss >> value;

	ASSERT_EQ(value_ref, value);
	ASSERT_TRUE(iss.eof());
	ASSERT_TRUE(!iss.good());
	ASSERT_TRUE(!iss.fail());
}

TEST(EthernetAddress, stream_output) {
	const std::string str_value = "ab:cd:ef:12:34:56";
	const auto value = from_string<EthernetAddress>(str_value);

	std::ostringstream oss;
	oss << value;

	ASSERT_EQ(str_value, oss.str());
}

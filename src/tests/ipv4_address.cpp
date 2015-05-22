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

#include "../internal/ipv4_address.hpp"
#include "../internal/common.hpp"

using boost::asio::ip::address_v4;

using freelan::IPv4Address;
using freelan::from_string;

TEST(IPv4Address, default_instantiation) {
	const IPv4Address value {};
}

TEST(IPv4Address, boost_asio_ip_address_v4_instantiation) {
	const address_v4 raw_value;
	const IPv4Address value { raw_value };

	ASSERT_EQ(raw_value, value.to_raw_value());
}

TEST(IPv4Address, string_instantiation) {
	const std::string str_value = "9.0.0.1";
	const auto value = IPv4Address::from_string(str_value);

	ASSERT_EQ(str_value, value.to_string());
}

TEST(IPv4Address, implicit_string_conversion) {
	const std::string str_value = "9.0.0.1";
	const auto value = from_string<IPv4Address>(str_value);

	ASSERT_EQ(str_value, to_string(value));
}

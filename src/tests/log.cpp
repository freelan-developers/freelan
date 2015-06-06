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

#include <functional>

#include "../internal/log.hpp"

using freelan::LogLevel;
using freelan::to_payload;
using freelan::Logger;
using std::string;

class LogTest : public ::testing::Test {
	protected:
		virtual void SetUp() {
			using namespace std::placeholders;

			freelan::set_log_function(std::bind(&LogTest::on_log, this, _1, _2, _3, _4, _5, _6, _7, _8));
			freelan::set_log_level(LogLevel::INFORMATION);
		}

		virtual void TearDown() {
			::freelan_set_logging_callback(nullptr);
		}

		bool on_log(LogLevel level, const boost::posix_time::ptime& timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line) {
			last_level = level;
			last_timestamp = timestamp;
			last_domain = domain;
			last_code = code;
			last_payload_size = payload_size;
			last_payload = payload;
			last_file = file;
			last_line = line;

			return return_value;
		}

		bool return_value = false;
		LogLevel last_level = LogLevel::INFORMATION;
		boost::posix_time::ptime last_timestamp;
		const char* last_domain = nullptr;
		const char* last_code = nullptr;
		size_t last_payload_size = 0;
		const FreeLANLogPayload* last_payload = nullptr;
		const char* last_file = nullptr;
		unsigned int last_line = 0;
};

TEST_F(LogTest, logger_simple_failure) {
	const auto result = Logger(LogLevel::INFORMATION, "foo", "bar").commit();

	ASSERT_FALSE(result);
}

TEST_F(LogTest, logger_simple_success) {
	return_value = true;

	const auto result = Logger(LogLevel::INFORMATION, "foo", "bar").commit();

	ASSERT_TRUE(result);
	ASSERT_EQ(LogLevel::INFORMATION, last_level);
	ASSERT_FALSE(last_timestamp.is_special());
	ASSERT_EQ(string("foo"), string(last_domain));
	ASSERT_EQ(string("bar"), string(last_code));
	ASSERT_EQ(static_cast<size_t>(0), last_payload_size);
	ASSERT_EQ(nullptr, last_payload);
	ASSERT_EQ(nullptr, last_file);
	ASSERT_EQ(static_cast<unsigned int>(0), last_line);
}

TEST_F(LogTest, logger_payload) {
	return_value = true;

	const auto result = (
		Logger(LogLevel::INFORMATION, "foo", "bar")
		<< to_payload("a", "one")
		<< to_payload("b", std::string("two"))
		<< to_payload(std::string("c"), "three")
		<< to_payload(std::string("d"), std::string("four"))
		<< to_payload("e", 5)
		<< to_payload("f", 6.0f)
		<< to_payload("g", true)
	).commit();

	ASSERT_TRUE(result);
	ASSERT_EQ(LogLevel::INFORMATION, last_level);
	ASSERT_FALSE(last_timestamp.is_special());
	ASSERT_EQ(string("foo"), string(last_domain));
	ASSERT_EQ(string("bar"), string(last_code));
	ASSERT_EQ(static_cast<size_t>(7), last_payload_size);
	ASSERT_NE(nullptr, last_payload);
	ASSERT_EQ(nullptr, last_file);
	ASSERT_EQ(static_cast<unsigned int>(0), last_line);

	// Test the payload values.
	ASSERT_EQ(string("a"), string(last_payload[0].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_STRING, last_payload[0].type);
	ASSERT_EQ(string("one"), string(last_payload[0].value.as_string));
	ASSERT_EQ(string("b"), string(last_payload[1].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_STRING, last_payload[1].type);
	ASSERT_EQ(string("two"), string(last_payload[1].value.as_string));
	ASSERT_EQ(string("c"), string(last_payload[2].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_STRING, last_payload[2].type);
	ASSERT_EQ(string("three"), string(last_payload[2].value.as_string));
	ASSERT_EQ(string("d"), string(last_payload[3].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_STRING, last_payload[3].type);
	ASSERT_EQ(string("four"), string(last_payload[3].value.as_string));
	ASSERT_EQ(string("e"), string(last_payload[4].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_INTEGER, last_payload[4].type);
	ASSERT_EQ(5, last_payload[4].value.as_integer);
	ASSERT_EQ(string("f"), string(last_payload[5].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_FLOAT, last_payload[5].type);
	ASSERT_EQ(6.0f, last_payload[5].value.as_float);
	ASSERT_EQ(string("g"), string(last_payload[6].key));
	ASSERT_EQ(FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN, last_payload[6].type);
	ASSERT_EQ(1, last_payload[6].value.as_boolean);
}

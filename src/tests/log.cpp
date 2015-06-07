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

#include <boost/any.hpp>

#include "../internal/log.hpp"

using freelan::LogLevel;
using freelan::Logger;
using freelan::Payload;
using std::string;
using boost::any_cast;

class LogTest : public ::testing::Test {
	protected:
		virtual void SetUp() {
			using namespace std::placeholders;

			freelan::set_log_function(std::bind(&LogTest::on_log, this, _1, _2, _3, _4, _5, _6, _7, _8));
			freelan::set_log_level(LogLevel::INFORMATION);
		}

		virtual void TearDown() {
			::freelan_set_logging_callback(nullptr);

			return_value = false;
			last_level = LogLevel::INFORMATION;
			last_timestamp = boost::posix_time::ptime();
			last_domain.clear();
			last_code.clear();
			last_payload.clear();
			last_file = nullptr;
			last_line = 0;
		}

		bool on_log(LogLevel level, const boost::posix_time::ptime& timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line) {
			last_level = level;
			last_timestamp = timestamp;
			last_domain = domain;
			last_code = code;
			last_file = file;
			last_line = line;

			for (size_t i = 0; i < payload_size; ++i) {
				last_payload.push_back(Payload::from_native_payload(payload[i]));
			}

			return return_value;
		}

		bool return_value = false;
		LogLevel last_level = LogLevel::INFORMATION;
		boost::posix_time::ptime last_timestamp;
		string last_domain;
		string last_code;
		std::vector<Payload> last_payload;
		const char* last_file = nullptr;
		unsigned int last_line = 0;
};

TEST_F(LogTest, logger_simple_failure) {
	const bool result = Logger(LogLevel::INFORMATION, "foo", "bar")
		.commit();

	ASSERT_FALSE(result);
}

TEST_F(LogTest, logger_simple_level_failure) {
	return_value = true;

	const bool result = Logger(LogLevel::DEBUG, "foo", "bar")
		.commit();

	ASSERT_FALSE(result);
}

TEST_F(LogTest, logger_simple_success) {
	return_value = true;

	const bool result = Logger(LogLevel::INFORMATION, "foo", "bar")
		.commit();

	ASSERT_TRUE(result);
	ASSERT_EQ(LogLevel::INFORMATION, last_level);
	ASSERT_FALSE(last_timestamp.is_special());
	ASSERT_EQ("foo", last_domain);
	ASSERT_EQ("bar", last_code);
	ASSERT_EQ(static_cast<size_t>(0), last_payload.size());
	ASSERT_EQ(nullptr, last_file);
	ASSERT_EQ(static_cast<unsigned int>(0), last_line);
}

TEST_F(LogTest, logger_payload) {
	return_value = true;

	const bool result = Logger(LogLevel::INFORMATION, "foo", "bar")
		.attach("a", "one")
		.attach("b", string("two"))
		.attach(string("c"), "three")
		.attach(string("d"), string("four"))
		.attach("e", 5)
		.attach("f", 6.0f)
		.attach("g", true)
		.commit();

	ASSERT_TRUE(result);
	ASSERT_EQ(LogLevel::INFORMATION, last_level);
	ASSERT_FALSE(last_timestamp.is_special());
	ASSERT_EQ("foo", last_domain);
	ASSERT_EQ("bar", last_code);
	ASSERT_EQ(static_cast<size_t>(7), last_payload.size());
	ASSERT_EQ(nullptr, last_file);
	ASSERT_EQ(static_cast<unsigned int>(0), last_line);

	// Test the payload values.
	ASSERT_EQ("a", last_payload[0].key);
	ASSERT_EQ("one", any_cast<string>(last_payload[0].value));
	ASSERT_EQ("b", last_payload[1].key);
	ASSERT_EQ("two", any_cast<string>(last_payload[1].value));
	ASSERT_EQ("c", last_payload[2].key);
	ASSERT_EQ("three",any_cast<string>(last_payload[2].value));
	ASSERT_EQ("d", last_payload[3].key);
	ASSERT_EQ("four",any_cast<string>(last_payload[3].value));
	ASSERT_EQ("e", last_payload[4].key);
	ASSERT_EQ(5, any_cast<int64_t>(last_payload[4].value));
	ASSERT_EQ("f", last_payload[5].key);
	ASSERT_EQ(6.0f, any_cast<double>(last_payload[5].value));
	ASSERT_EQ("g", last_payload[6].key);
	ASSERT_EQ(1, any_cast<bool>(last_payload[6].value));
}

TEST_F(LogTest, logger_complete_success) {
	return_value = true;

	Logger(LogLevel::INFORMATION, "foo", "bar", "myfile.cpp", 123);

	ASSERT_EQ(string("myfile.cpp"), string(last_file));
	ASSERT_EQ(static_cast<unsigned int>(123), last_line);
}

TEST_F(LogTest, log_success) {
	return_value = true;

	LOG(LogLevel::INFORMATION, "foo", "bar").attach("a", "foo");

	ASSERT_EQ(LogLevel::INFORMATION, last_level);
	ASSERT_EQ(static_cast<size_t>(1), last_payload.size());
	ASSERT_FALSE(string(last_file).empty());
	ASSERT_EQ(static_cast<unsigned int>(185), last_line);

	// Test the payload values.
	ASSERT_EQ("a", last_payload[0].key);
	ASSERT_EQ("foo", any_cast<string>(last_payload[0].value));
}

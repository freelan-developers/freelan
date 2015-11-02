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

#include <boost/asio.hpp>

#include "../common.hpp"

#include "../internal/fscp/message.hpp"

using freelan::FSCPMessageType;
using freelan::write_fscp_message;
using freelan::write_fscp_hello_request_message;
using freelan::write_fscp_hello_response_message;

TEST(FSCPMessageTest, write_fscp_message_payload_too_big) {
    char buf[70000] = {};
    const char payload[65536] = {};
    const auto resulting_size = write_fscp_message(buf, sizeof(buf), FSCPMessageType::HELLO_REQUEST, payload, sizeof(payload));

    ASSERT_EQ(0, resulting_size);
}

TEST(FSCPMessageTest, write_fscp_message_buffer_too_small) {
    char buf[8] = {};
    const char payload[5] = {};
    const auto resulting_size = write_fscp_message(buf, sizeof(buf), FSCPMessageType::HELLO_REQUEST, payload, sizeof(payload));

    ASSERT_EQ(0, resulting_size);
}

TEST(FSCPMessageTest, write_fscp_message_success) {
    uint8_t buf[10] = {};
    std::memset(buf, 0xfd, sizeof(buf));
    const uint8_t payload[4] = { 1, 2, 3, 4 };
    const uint8_t ref[] = { 3, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04, 0xfd, 0xfd };
    const auto resulting_size = write_fscp_message(buf, sizeof(buf), FSCPMessageType::HELLO_REQUEST, payload, sizeof(payload));

    ASSERT_EQ(8, resulting_size);
    ASSERT_ARRAY_EQ(ref, buf);
}

TEST(FSCPMessageTest, write_fscp_hello_request_message) {
    uint8_t buf[10] = {};
    std::memset(buf, 0xfd, sizeof(buf));
    const uint32_t unique_number = 0x01020304;
    const uint8_t ref[] = { 3, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04, 0xfd, 0xfd };
    const auto resulting_size = write_fscp_hello_request_message(buf, sizeof(buf), unique_number);

    ASSERT_EQ(8, resulting_size);
    ASSERT_ARRAY_EQ(ref, buf);
}

TEST(FSCPMessageTest, write_fscp_hello_response_message) {
    uint8_t buf[10] = {};
    std::memset(buf, 0xfd, sizeof(buf));
    const uint32_t unique_number = 0x01020304;
    const uint8_t ref[] = { 3, 0x01, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04, 0xfd, 0xfd };
    const auto resulting_size = write_fscp_hello_response_message(buf, sizeof(buf), unique_number);

    ASSERT_EQ(8, resulting_size);
    ASSERT_ARRAY_EQ(ref, buf);
}
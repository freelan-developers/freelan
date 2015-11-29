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

#include "../internal/async_utils.hpp"

#include <functional>

#include <boost/asio.hpp>

using freelan::start_operation;

TEST(AsyncUtilsTest, start_operation_immediate_failure) {
    boost::asio::io_service io_service;

    // Our write operation fails immediately. There should be no timer started.
    const auto my_write_operation = [](std::function<void (const boost::system::error_code&)> handler) {
        handler(boost::asio::error::access_denied);
    };
    boost::system::error_code handler_result{};
    const auto my_handler = [&handler_result](const boost::system::error_code& ec) {
        handler_result = ec;
    };
    const auto my_operation_handler = [](std::function<bool (const boost::system::error_code&)>) {
        // If the operation handler gets called, something is wrong.
        GTEST_FAIL();
    };
    const auto timeout = boost::posix_time::seconds(0);

    start_operation(io_service, my_write_operation, my_handler, my_operation_handler, timeout);
    io_service.run();

    ASSERT_EQ(boost::asio::error::access_denied, handler_result);
}

TEST(AsyncUtilsTest, start_operation_timeout) {
    boost::asio::io_service io_service;

    // Our write operation succeeds. The timer should be started.
    const auto my_write_operation = [](std::function<void(const boost::system::error_code&)> handler) {
        handler(boost::system::error_code());
    };
    boost::system::error_code handler_result {};
    const auto my_handler = [&handler_result](const boost::system::error_code& ec) {
        handler_result = ec;
    };
    std::function<bool(const boost::system::error_code&)> stop_operation = nullptr;
    const auto my_operation_handler = [&stop_operation](std::function<bool(const boost::system::error_code&)> _stop_operation) {
        stop_operation = _stop_operation;
    };
    const auto timeout = boost::posix_time::seconds(0);

    start_operation(io_service, my_write_operation, my_handler, my_operation_handler, timeout);
    io_service.run();

    ASSERT_EQ(boost::asio::error::timed_out, handler_result);
    ASSERT_NE(nullptr, stop_operation);
}

TEST(AsyncUtilsTest, start_operation_success) {
    boost::asio::io_service io_service;

    // Our write operation succeeds. The timer should be started.
    const auto my_write_operation = [](std::function<void(const boost::system::error_code&)> handler) {
        handler(boost::system::error_code());
    };
    boost::system::error_code handler_result{};
    const auto my_handler = [&handler_result](const boost::system::error_code& ec) {
        handler_result = ec;
    };
    bool stop_operation_result = false;
    const auto my_operation_handler = [&stop_operation_result](std::function<bool(const boost::system::error_code&)> stop_operation) {
        stop_operation_result = stop_operation(boost::asio::error::address_in_use);
    };
    // 3 seconds should be plenty on any decent computer. If you get a flaky test, check this.
    const auto timeout = boost::posix_time::seconds(3);

    start_operation(io_service, my_write_operation, my_handler, my_operation_handler, timeout);
    io_service.run();

    ASSERT_EQ(boost::asio::error::address_in_use, handler_result);
    ASSERT_TRUE(stop_operation_result);
}
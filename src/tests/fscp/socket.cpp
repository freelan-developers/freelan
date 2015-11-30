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

#include "../internal/fscp/socket.hpp"

using freelan::Socket;

namespace {
    static const auto timeout = boost::posix_time::seconds(3);
}

TEST(FSCPSocketTest, socket_async_greet) {
    boost::asio::io_service io_service;
    Socket socket_a(io_service);
    Socket socket_b(io_service);
    const Socket::Endpoint ep_a(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12000);
    const Socket::Endpoint ep_b(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12001);
    socket_a.open(ep_a);
    socket_b.open(ep_b);

    unsigned int successes = 0;
    unsigned int failures = 0;

    const auto on_greet_response = [&](const boost::system::error_code& ec) {
        if (ec) {
            failures++;
        } else {
            successes++;
        }

        if ((successes + failures) == 2) {
            socket_a.close();
            socket_b.close();
        }
    };

    socket_a.async_greet(ep_b, on_greet_response, timeout);
    socket_b.async_greet(ep_a, on_greet_response, timeout);

    io_service.run();

    ASSERT_EQ((unsigned int)(2), successes);
    ASSERT_EQ((unsigned int)(0), failures);
}

TEST(FSCPSocketTest, socket_async_greet_self) {
    boost::asio::io_service io_service;
    Socket socket(io_service);
    const Socket::Endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12000);
    socket.open(ep);

    unsigned int successes = 0;
    unsigned int failures = 0;

    const auto on_greet_response = [&](const boost::system::error_code& ec) {
        if (ec) {
            failures++;
        } else {
            successes++;
        }

        if ((successes + failures) == 1) {
            socket.close();
        }
    };

    socket.async_greet(ep, on_greet_response, timeout);

    io_service.run();

    ASSERT_EQ((unsigned int)(1), successes);
    ASSERT_EQ((unsigned int)(0), failures);
}

TEST(FSCPSocketTest, socket_async_greet_timeout) {
    boost::asio::io_service io_service;
    Socket socket(io_service);
    const Socket::Endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12000);
    const Socket::Endpoint destination(boost::asio::ip::address_v4::from_string("127.0.0.254"), 12000);
    socket.open(ep);

    unsigned int successes = 0;
    unsigned int failures = 0;
    unsigned int timed_out = 0;

    const auto on_greet_response = [&](const boost::system::error_code& ec) {
        if (ec) {
            if (ec == boost::asio::error::timed_out) {
                timed_out++;
            } else {
                failures++;
            }
        } else {
            successes++;
        }

        if ((successes + failures + timed_out) == 1) {
            socket.close();
        }
    };

    socket.async_greet(destination, on_greet_response, boost::posix_time::seconds(0));

    io_service.run();

    ASSERT_EQ((unsigned int)(0), successes);
    ASSERT_EQ((unsigned int)(0), failures);
    ASSERT_EQ((unsigned int)(1), timed_out);
}

TEST(FSCPSocketTest, socket_async_greet_failure) {
    boost::asio::io_service io_service;
    Socket socket(io_service);
    const Socket::Endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12000);
    const Socket::Endpoint destination(boost::asio::ip::address_v4::from_string("0.0.0.0"), 12000);
    socket.open(ep);

    unsigned int successes = 0;
    unsigned int failures = 0;
    unsigned int timed_out = 0;

    const auto on_greet_response = [&](const boost::system::error_code& ec) {
        if (ec) {
            if (ec == boost::asio::error::timed_out) {
                timed_out++;
            }
            else {
                failures++;
            }
        }
        else {
            successes++;
        }

        if ((successes + failures + timed_out) == 1) {
            socket.close();
        }
    };

    socket.async_greet(destination, on_greet_response, timeout);

    io_service.run();

    ASSERT_EQ((unsigned int)(0), successes);
    ASSERT_EQ((unsigned int)(1), failures);
    ASSERT_EQ((unsigned int)(0), timed_out);
}

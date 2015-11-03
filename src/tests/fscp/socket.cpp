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

TEST(FSCPSocketTest, socket_get_endpoint_context_for) {
    boost::asio::io_service io_service;
    Socket socket(io_service);
    Socket::Endpoint ep1(boost::asio::ip::address_v4::from_string("127.0.0.1"), 123);
    Socket::Endpoint ep2(boost::asio::ip::address_v4::from_string("127.0.0.1"), 345);

    const auto& ctx1 = socket.get_endpoint_context_for(ep1);
    const auto& ctx2 = socket.get_endpoint_context_for(ep1);
    const auto& ctx3 = socket.get_endpoint_context_for(ep2);

    ASSERT_EQ(&ctx1, &ctx2);
    ASSERT_NE(&ctx1, &ctx3);
}

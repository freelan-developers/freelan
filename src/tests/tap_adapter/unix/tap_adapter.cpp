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

#include "../../common.hpp"

#include "../../../internal/tap_adapter/unix/tap_adapter.hpp"

#include <boost/asio.hpp>

using freelan::TapAdapter;
using freelan::TapAdapterLayer;

typedef LoggedTest TapAdapterTest;

TEST_F(TapAdapterTest, default_instanciation_ethernet) {
    IS_SYSTEM_TEST();

    boost::asio::io_service io_service;

    TapAdapter tap_adapter(io_service, TapAdapterLayer::ethernet);
    boost::system::error_code ec;

    const auto result = tap_adapter.open(ec);

    SCOPED_LOGS();

    ASSERT_EQ(ec, result);
    ASSERT_EQ(boost::system::error_code(), ec);
}

TEST_F(TapAdapterTest, default_instanciation_ip) {
    IS_SYSTEM_TEST();

    boost::asio::io_service io_service;

    TapAdapter tap_adapter(io_service, TapAdapterLayer::ip);
    boost::system::error_code ec;

    const auto result = tap_adapter.open(ec);

    SCOPED_LOGS();

    ASSERT_EQ(ec, result);
    ASSERT_EQ(boost::system::error_code(), ec);
}

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
#include "../internal/types/tap_adapter_layer.hpp"

using freelan::TapAdapterLayer;

TEST(TapAdapterLayer, default_instantiation) {
    const TapAdapterLayer value {};
    ASSERT_EQ(TapAdapterLayer::ethernet, value);
}

TEST(TapAdapterLayer, explicit_instantiation) {
    const TapAdapterLayer value = TapAdapterLayer::ip;
    ASSERT_EQ(TapAdapterLayer::ip, value);
}

TEST(TapAdapterLayer, to_output_stream_ethernet) {
    std::ostringstream oss;
    oss << TapAdapterLayer::ethernet;

    ASSERT_EQ("ethernet", oss.str());
}

TEST(TapAdapterLayer, to_output_stream_ip) {
    std::ostringstream oss;
    oss << TapAdapterLayer::ip;

    ASSERT_EQ("ip", oss.str());
}

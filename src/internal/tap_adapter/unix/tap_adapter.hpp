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

/**
 * \file tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A TAP adapter class.
 */

#pragma once

#include <map>
#include <string>

#include "../generic_tap_adapter.hpp"

namespace freelan {

class TapAdapter : public GenericTapAdapter<boost::asio::posix::stream_descriptor> {
    public:
        static std::map<std::string, std::string> enumerate(TapAdapterLayer _layer);

        TapAdapter(boost::asio::io_service& _io_service, TapAdapterLayer _layer) :
            GenericTapAdapter(_io_service, _layer)
        {}

        ~TapAdapter() {
            if (is_open()) {
                boost::system::error_code ec;

                close(ec);
                //TODO: Log the error.
            }
        }

        TapAdapter(const TapAdapter&) = delete;
        TapAdapter& operator=(const TapAdapter&) = delete;
        TapAdapter(TapAdapter&&) = default;
        TapAdapter& operator=(TapAdapter&&) = default;

        boost::system::error_code open(boost::system::error_code& ec) { return open("", ec); }
        boost::system::error_code open(const std::string& name, boost::system::error_code& ec);

        boost::system::error_code close(boost::system::error_code& ec) {
            destroy_device(ec);
            //TODO: Log the error.

            return GenericTapAdapter::close(ec);
        }

        void set_connected_state(bool connected);

    private:
        boost::system::error_code destroy_device(boost::system::error_code& ec);
};

}

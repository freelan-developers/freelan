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
 * \file ethernet_address.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Ethernet address.
 */

#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <array>

#include <boost/system/system_error.hpp>

#include "generic_value_type.hpp"
#include "stream_parsers.hpp"

namespace freelan
{

typedef std::array<uint8_t, 6> EthernetAddressBytes;

class EthernetAddress : public GenericValueType<EthernetAddressBytes, EthernetAddress> {
    public:
        EthernetAddress() = default;
		EthernetAddress(EthernetAddress::value_type&& value) : GenericValueType<EthernetAddressBytes, EthernetAddress>(std::move(value)) {}
		EthernetAddress(const EthernetAddress::value_type& value) : GenericValueType<EthernetAddressBytes, EthernetAddress>(value) {}

        static EthernetAddress from_string(const std::string& str) {
            boost::system::error_code ec;
            const EthernetAddress result = from_string(str, ec);

            if (ec) {
                throw boost::system::system_error(ec);
            }

            return result;
        }

        static EthernetAddress from_string(const std::string& str, boost::system::error_code& ec) {
            std::istringstream iss(str);
            EthernetAddress result;

            if (!read_from(iss, result) || !iss.eof()) {
                ec = make_error_code(boost::system::errc::invalid_argument);

                return {};
            }

            return result;
        }

        static std::istream& read_from(std::istream& is, EthernetAddress& value, std::string* buf = nullptr) {
            return read_ethernet_address(is, value, buf);
        }

        std::string to_string() const {
            std::ostringstream oss;
            write_to(oss);
            return oss.str();
        }

        std::string to_string(boost::system::error_code&) const {
            return to_string();
        }

        std::ostream& write_to(std::ostream& os) const;

        static EthernetAddress null() {
            const EthernetAddressBytes data = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

            return EthernetAddress(data);
        }

        static EthernetAddress broadcast() {
            const EthernetAddressBytes data = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

            return EthernetAddress(data);
        }
};

}

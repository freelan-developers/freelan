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
 * \file generic_ip_endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An generic IP endpoint.
 */

#pragma once

#include <sstream>

#include <boost/operators.hpp>

#include "hostname.hpp"
#include "port_number.hpp"

namespace freelan {

class HostnameEndpoint : public boost::operators<HostnameEndpoint> {
    public:
        HostnameEndpoint() = default;
        HostnameEndpoint(const Hostname& hostname, const PortNumber& port_number = PortNumber()) :
            m_hostname(hostname),
            m_port_number(port_number)
        {}

        static HostnameEndpoint from_string(const std::string& str) {
            boost::system::error_code ec;
            const HostnameEndpoint result = from_string(str, ec);

            if (ec) {
                throw boost::system::system_error(ec);
            }

            return result;
        }

        static HostnameEndpoint from_string(const std::string& str, boost::system::error_code& ec) {
            std::istringstream iss(str);
            HostnameEndpoint result;

            if (!read_from(iss, result) || !iss.eof()) {
                ec = make_error_code(boost::system::errc::invalid_argument);

                return {};
            }

            return result;
        }

        static std::istream& read_from(std::istream& is, HostnameEndpoint& value, std::string* buf = nullptr) {
            return read_hostname_endpoint(is, value.m_hostname, value.m_port_number, buf);
        }

        const Hostname& get_hostname() const { return m_hostname; }
        const PortNumber& get_port_number() const { return m_port_number; }

        std::string to_string() const {
            std::ostringstream oss;
            write_to(oss);

            return oss.str();
        }

        std::ostream& write_to(std::ostream& os) const {
            m_hostname.write_to(os);
            os << ":";
            m_port_number.write_to(os);

            return os;
        }

    private:
        Hostname m_hostname;
        PortNumber m_port_number;

        friend bool operator<(const HostnameEndpoint& lhs, const HostnameEndpoint& rhs) {
            if (lhs.m_hostname == rhs.m_hostname) {
                return (lhs.m_port_number < rhs.m_port_number);
            } else {
                return (lhs.m_hostname < rhs.m_hostname);
            }
        }

        friend bool operator==(const HostnameEndpoint& lhs, const HostnameEndpoint& rhs) {
            return (lhs.m_hostname == rhs.m_hostname) && (lhs.m_port_number == rhs.m_port_number);
        }

        friend std::istream& operator>>(std::istream& is, HostnameEndpoint& value) {
            return HostnameEndpoint::read_from(is, value);
        }

        friend std::ostream& operator<<(std::ostream& os, const HostnameEndpoint& value) {
            return value.write_to(os);
        }
};

}

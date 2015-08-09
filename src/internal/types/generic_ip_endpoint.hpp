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

#include "generic_ip_address.hpp"
#include "port_number.hpp"

namespace freelan {

template <typename AddressType>
class GenericIPEndpoint : public boost::operators<GenericIPEndpoint<AddressType> > {
	public:
		typedef GenericIPAddress<AddressType> IPAddressType;

		GenericIPEndpoint() = default;
		GenericIPEndpoint(const IPAddressType& ip_address, const PortNumber& port_number = PortNumber()) :
			m_ip_address(ip_address),
			m_port_number(port_number)
		{}

		static GenericIPEndpoint from_string(const std::string& str) {
			boost::system::error_code ec;
			const GenericIPEndpoint result = from_string(str, ec);

			if (ec) {
				throw boost::system::system_error(ec);
			}

			return result;
		}

		static GenericIPEndpoint from_string(const std::string& str, boost::system::error_code& ec) {
			std::istringstream iss(str);
			GenericIPEndpoint result;

			if (!read_from(iss, result) || !iss.eof()) {
				ec = make_error_code(boost::system::errc::invalid_argument);

				return {};
			}

			return result;
		}

		static std::istream& read_from(std::istream& is, GenericIPEndpoint& value, std::string* buf = nullptr) {
			return read_generic_ip_endpoint<IPAddressType>(is, value.m_ip_address, value.m_port_number, buf);
		}

		const IPAddressType& get_ip_address() const { return m_ip_address; }
		const PortNumber& get_port_number() const { return m_port_number; }

		std::string to_string() const {
			std::ostringstream oss;
			write_to(oss);

			return oss.str();
		}

		std::ostream& write_to(std::ostream& os) const;

	private:
		IPAddressType m_ip_address;
		PortNumber m_port_number;

		friend bool operator<(const GenericIPEndpoint& lhs, const GenericIPEndpoint& rhs) {
			if (lhs.m_ip_address == rhs.m_ip_address) {
				return (lhs.m_port_number < rhs.m_port_number);
			} else {
				return (lhs.m_ip_address < rhs.m_ip_address);
			}
		}

		friend bool operator==(const GenericIPEndpoint& lhs, const GenericIPEndpoint& rhs) {
			return (lhs.m_ip_address == rhs.m_ip_address) && (lhs.m_port_number == rhs.m_port_number);
		}

		friend std::istream& operator>>(std::istream& is, GenericIPEndpoint& value) {
			return GenericIPEndpoint::read_from(is, value);
		}

		friend std::ostream& operator<<(std::ostream& os, const GenericIPEndpoint& value) {
			return value.write_to(os);
		}
};

}

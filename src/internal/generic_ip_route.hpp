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
 * \file generic_ip_route.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A generic IP route.
 */

#pragma once

#include <sstream>
#include <iterator>

#include <boost/operators.hpp>

#include "generic_ip_address.hpp"
#include "generic_ip_prefix_length.hpp"

namespace freelan {

template <typename AddressType>
class GenericIPRoute : public boost::operators<GenericIPRoute<AddressType> > {
	public:
		typedef GenericIPAddress<AddressType> IPAddressType;
		typedef GenericIPPrefixLength<AddressType> IPPrefixLengthType;

		class const_iterator : public std::iterator<std::forward_iterator_tag, IPAddressType>, public boost::operators<const_iterator> {
			public:
				const_iterator& operator++() {
					++m_ip_address;

					return *this;
				}
				typename const_iterator::value_type operator*() const {
					return m_ip_address;
				}

			private:
				const_iterator(const typename const_iterator::value_type& ip_address) :
					m_ip_address(ip_address)
				{}

				typename const_iterator::value_type m_ip_address;

				friend bool operator==(const const_iterator& lhs, const const_iterator& rhs) {
					return (lhs.m_ip_address == rhs.m_ip_address);
				}

				friend class GenericIPRoute;
		};

		GenericIPRoute() = default;
		GenericIPRoute(const IPAddressType& ip_address, const IPPrefixLengthType& prefix_length) :
			m_ip_address(to_network_address(ip_address, prefix_length.to_raw_value())),
			m_prefix_length(prefix_length)
		{
		}

		static GenericIPRoute from_string(const std::string& str) {
			boost::system::error_code ec;
			const GenericIPRoute result = from_string(str, ec);

			if (ec) {
				throw boost::system::system_error(ec);
			}

			return result;
		}

		static GenericIPRoute from_string(const std::string& str, boost::system::error_code& ec) {
			std::istringstream iss(str);
			GenericIPRoute result;

			if (!read_from(iss, result) || !iss.eof()) {
				ec = make_error_code(boost::system::errc::invalid_argument);

				return {};
			}

			return result;
		}

		static std::istream& read_from(std::istream& is, GenericIPRoute& value, std::string* buf = nullptr) {
			if (read_generic_ip_route<IPAddressType, IPPrefixLengthType>(is, value.m_ip_address, value.m_prefix_length, buf)) {
				value.normalize();
			}

			return is;
		}

		const IPAddressType& get_ip_address() const { return m_ip_address; }
		const IPPrefixLengthType& get_prefix_length() const { return m_prefix_length; }
		IPAddressType get_broadcast_ip_address() const { return to_broadcast_address(m_ip_address, m_prefix_length.to_raw_value()); }

		std::string to_string() const {
			std::ostringstream oss;
			write_to(oss);

			return oss.str();
		}

		std::ostream& write_to(std::ostream& os) const {
			return os << m_ip_address << "/" << m_prefix_length;
		}

		const_iterator begin() const { auto first = m_ip_address; return const_iterator(++first); }
		const_iterator end() const { return const_iterator(get_broadcast_ip_address()); }

	private:
		static IPAddressType to_network_address(const IPAddressType& ip_address, size_t prefix_length) {
			auto bytes = ip_address.to_bytes();
			const auto index = prefix_length / 8;

			if (index < sizeof(bytes)) {
				const size_t bit_index = prefix_length - index * 8;
				const uint8_t bit_mask = ~(static_cast<uint8_t>(0xFF) >> bit_index);

				bytes[index] &= bit_mask;

				for (auto i = index + 1; i < sizeof(bytes); ++i) {
					bytes[i] = 0x00;
				}
			}

			return IPAddressType::from_bytes(bytes);
		}

		static IPAddressType to_broadcast_address(const IPAddressType& ip_address, size_t prefix_length) {
			auto bytes = ip_address.to_bytes();
			const auto index = prefix_length / 8;

			if (index < sizeof(bytes)) {
				const size_t bit_index = prefix_length - index * 8;
				const uint8_t bit_mask = static_cast<uint8_t>(0xFF) >> bit_index;

				bytes[index] |= bit_mask;

				for (auto i = index + 1; i < sizeof(bytes); ++i) {
					bytes[i] = 0xff;
				}
			}

			return IPAddressType::from_bytes(bytes);
		}

		void normalize() {
			m_ip_address = to_network_address(m_ip_address, static_cast<const IPPrefixLengthType&>(m_prefix_length).to_raw_value());
		}

		IPAddressType m_ip_address;
		IPPrefixLengthType m_prefix_length;

		friend bool operator<(const GenericIPRoute& lhs, const GenericIPRoute& rhs) {
			if (lhs.m_prefix_length == rhs.m_prefix_length) {
				return (lhs.m_ip_address < rhs.m_ip_address);
			} else {
				return (lhs.m_prefix_length > rhs.m_prefix_length);
			}
		}

		friend bool operator==(const GenericIPRoute& lhs, const GenericIPRoute& rhs) {
			return (lhs.m_ip_address == rhs.m_ip_address) && (lhs.m_prefix_length == rhs.m_prefix_length);
		}

		friend std::istream& operator>>(std::istream& is, GenericIPRoute& value) {
			return GenericIPRoute::read_from(is, value);
		}

		friend std::ostream& operator<<(std::ostream& os, const GenericIPRoute& value) {
			return value.write_to(os);
		}
};

}

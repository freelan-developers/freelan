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
 * \file generic_ip_address.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A generic IP address.
 */

#pragma once

#include <iostream>

#include <boost/operators.hpp>

#include "stream_parsers.hpp"

namespace freelan {

template <typename ValueType>
class GenericIPAddress : public boost::operators<GenericIPAddress<ValueType> > {
	public:
		typedef ValueType value_type;

		GenericIPAddress() :
			m_value()
		{}

		GenericIPAddress(value_type&& value) :
			m_value(std::move(value))
		{}

		GenericIPAddress(const value_type& value) :
			m_value(value)
		{}

		const value_type& to_raw_value() const {
			return m_value;
		}

		static GenericIPAddress from_string(const std::string& str) {
			return value_type::from_string(str);
		}

		static GenericIPAddress from_string(const std::string& str, boost::system::error_code& ec) {
			return value_type::from_string(str, ec);
		}

		static std::istream& read_from(std::istream& is, GenericIPAddress& value) {
			return read_generic_ip_address(is, value.m_value, nullptr);
		}

		std::string to_string() const {
			return m_value.to_string();
		}

		std::string to_string(boost::system::error_code& ec) const {
			return m_value.to_string(ec);
		}

		std::ostream& write_to(std::ostream& os) const {
			return os << to_string();
		}

	private:
		value_type m_value;

		friend bool operator<(const GenericIPAddress& lhs, const GenericIPAddress& rhs) {
			return (lhs.m_value < rhs.m_value);
		}

		friend bool operator==(const GenericIPAddress& lhs, const GenericIPAddress& rhs) {
			return (lhs.m_value == rhs.m_value);
		}

		friend std::istream& operator>>(std::istream& is, GenericIPAddress& value) {
			return GenericIPAddress::read_from(is, value);
		}

		friend std::ostream& operator<<(std::ostream& os, const GenericIPAddress& value) {
			return value.write_to(os);
		}
};

}

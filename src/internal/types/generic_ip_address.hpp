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

#include "generic_value_type.hpp"
#include "stream_parsers.hpp"

namespace freelan {

template <typename ValueType>
class GenericIPAddress : public GenericValueType<ValueType, GenericIPAddress<ValueType>> {
    public:
        typedef typename ValueType::bytes_type bytes_type;

        GenericIPAddress() = default;
        GenericIPAddress(typename GenericIPAddress::value_type&& value) : GenericValueType<ValueType, GenericIPAddress<ValueType> >(std::move(value)) {}
        GenericIPAddress(const typename GenericIPAddress::value_type& value) : GenericValueType<ValueType, GenericIPAddress<ValueType> >(value) {}

        static GenericIPAddress from_string(const std::string& str) {
            return GenericIPAddress::value_type::from_string(str);
        }

        static GenericIPAddress from_string(const std::string& str, boost::system::error_code& ec) {
            return GenericIPAddress::value_type::from_string(str, ec);
        }

        static GenericIPAddress from_bytes(const bytes_type& bytes) {
            return typename GenericIPAddress::value_type(bytes);
        }

        static std::istream& read_from(std::istream& is, GenericIPAddress& value, std::string* buf = nullptr) {
            return read_generic_ip_address(is, value.to_raw_value(), buf);
        }

        std::string to_string() const {
            return this->to_raw_value().to_string();
        }

        std::string to_string(boost::system::error_code& ec) const {
            return this->to_raw_value().to_string(ec);
        }

        bytes_type to_bytes() const {
            return this->to_raw_value().to_bytes();
        }

        std::ostream& write_to(std::ostream& os) const {
            return os << to_string();
        }

        GenericIPAddress& operator++() {
            auto bytes = this->to_raw_value().to_bytes();
            const auto bytes_size = sizeof(bytes);

            for (auto index = bytes_size; index > 0; --index) {
                if (bytes[index - 1] < 255) {
                    ++bytes[index - 1];
                    break;

                } else {
                    bytes[index - 1] = 0;
                }
            }

            this->set_raw_value(typename GenericIPAddress::value_type(bytes));

            return *this;
        }
};

}

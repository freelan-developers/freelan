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
 * \file generic_ip_prefix_length.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A generic IP prefix length.
 */

#pragma once

#include <iostream>
#include <string>
#include <cstdint>

#include <boost/system/system_error.hpp>
#include <boost/lexical_cast.hpp>

#include "generic_value_type.hpp"
#include "stream_parsers.hpp"

namespace freelan {

template <typename AddressType>
class GenericIPPrefixLength : public GenericValueType<uint8_t, GenericIPPrefixLength<AddressType> > {
    public:
        typedef AddressType address_type;

        GenericIPPrefixLength() = default;
        GenericIPPrefixLength(typename GenericIPPrefixLength::value_type&& value) : GenericValueType<uint8_t, GenericIPPrefixLength<AddressType> >(std::move(value)) {}
        GenericIPPrefixLength(const typename GenericIPPrefixLength::value_type& value) : GenericValueType<uint8_t, GenericIPPrefixLength<AddressType> >(value) {}

        static GenericIPPrefixLength from_string(const std::string& str) {
            boost::system::error_code ec;
            const GenericIPPrefixLength result = from_string(str, ec);

            if (ec) {
                throw boost::system::system_error(ec);
            }

            return result;
        }

        static GenericIPPrefixLength from_string(const std::string& str, boost::system::error_code& ec) {
            std::istringstream iss(str);
            GenericIPPrefixLength result;

            if (!read_from(iss, result) || !iss.eof()) {
                ec = make_error_code(boost::system::errc::invalid_argument);

                return {};
            }

            return result;
        }

        static std::istream& read_from(std::istream& is, GenericIPPrefixLength& value, std::string* buf = nullptr) {
            return read_generic_ip_prefix_length<AddressType>(is, value.to_raw_value(), buf);
        }

        std::string to_string() const {
            return boost::lexical_cast<std::string>(static_cast<unsigned int>(this->to_raw_value()));
        }

        std::string to_string(boost::system::error_code&) const {
            return boost::lexical_cast<std::string>(static_cast<unsigned int>(this->to_raw_value()));
        }

        std::ostream& write_to(std::ostream& os) const {
            return os << static_cast<unsigned int>(this->to_raw_value());
        }
};

}

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
 * \file stream_parsers.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Stream parsing functions.
 */

#pragma once

#include <iostream>
#include <string>

namespace freelan {

class PortNumber;

template <typename AddressType>
std::istream& read_generic_ip_address(std::istream& is, AddressType& value, std::string* buf = nullptr);
std::istream& read_hostname_label(std::istream& is, std::string& value, std::string* buf = nullptr);
std::istream& read_hostname(std::istream& is, std::string& value, std::string* buf = nullptr);
std::istream& read_port_number(std::istream& is, uint16_t& value, std::string* buf = nullptr);
template <typename AddressType>
std::istream& read_generic_ip_prefix_length(std::istream& is, uint8_t& value, std::string* buf = nullptr);
template <typename IPAddressType>
std::istream& read_generic_ip_endpoint(std::istream& is, IPAddressType& ip_address, PortNumber& value, std::string* buf = nullptr);
}

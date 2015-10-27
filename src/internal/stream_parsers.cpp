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

#include "stream_parsers.hpp"

#include <sstream>
#include <limits>

#include <boost/asio.hpp>

#include "types/ipv4_address.hpp"
#include "types/ipv6_address.hpp"
#include "types/port_number.hpp"
#include "types/hostname.hpp"
#include "types/ipv4_prefix_length.hpp"
#include "types/ipv6_prefix_length.hpp"
#include "types/ethernet_address.hpp"

namespace freelan {

namespace {
    template <typename AddressType>
    bool is_ip_address_character(char c);

    template <>
    inline bool is_ip_address_character<boost::asio::ip::address_v4>(char c)
    {
        return (std::isdigit(c) || (c == '.'));
    }

    template <>
    inline bool is_ip_address_character<boost::asio::ip::address_v6>(char c)
    {
        return (std::isxdigit(c) || (c == ':'));
    }

    template <typename AddressType>
    struct get_ip_max_prefix_length;

    template <>
    struct get_ip_max_prefix_length<boost::asio::ip::address_v4> {
        static const uint8_t value = 32;
    };

    template <>
    struct get_ip_max_prefix_length<boost::asio::ip::address_v6> {
        static const uint8_t value = 128;
    };

    // Hostname labels are 63 characters long at most
    const size_t HOSTNAME_LABEL_MAX_SIZE = 63;

    // Hostnames are at most 255 characters long
    const size_t HOSTNAME_MAX_SIZE = 255;

    bool is_hostname_label_regular_character(char c) {
        return (std::isalnum(c) != 0);
    }

    bool is_hostname_label_special_character(char c) {
        return (c == '-');
    }

    bool is_hostname_label_character(char c) {
        return is_hostname_label_regular_character(c) || is_hostname_label_special_character(c);
    }

    bool is_unsigned_integer_character(char c) {
        return (std::isdigit(c) != 0);
    }

    bool is_endpoint_separator(char c) {
        return (c == ':');
    }

    bool is_ipv6_endpoint_address_start_delimiter(char c) {
        return (c == '[');
    }

    bool is_ipv6_endpoint_address_stop_delimiter(char c) {
        return (c == ']');
    }

    bool is_route_separator(char c) {
        return (c == '/');
    }

    bool is_route_gateway_separator(char c) {
        return (c == '@');
    }

    bool is_ethernet_address_separator(char c)
    {
        return (c == ':') || (c == '-');
    }

    unsigned int xdigit_to_numeric(char c)
    {
        assert(std::isxdigit(c));

        static const char xdigit_map[] =
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        return xdigit_map[static_cast<size_t>(c)];
    }

    std::istream& putback(std::istream& is, const std::string& str)
    {
        const std::ios::iostate state = is.rdstate() & ~std::ios::eofbit;
        is.clear();

        std::for_each(str.rbegin(), str.rend(), [&is] (char c) { is.putback(c); });

        is.setstate(state);

        return is;
    }
}

template <typename AddressType>
std::istream& read_generic_ip_address(std::istream& is, AddressType& value, std::string* buf)
{
    if (is.good())
    {
        if (!is_ip_address_character<AddressType>(is.peek()))
        {
            is.setstate(std::ios_base::failbit);
        }
        else
        {
            std::ostringstream oss;

            do
            {
                oss.put(static_cast<char>(is.get()));
            }
            while (is.good() && is_ip_address_character<AddressType>(is.peek()));

            if (is)
            {
                const std::string& result = oss.str();
                boost::system::error_code ec;

                value = AddressType::from_string(result, ec);

                if (ec)
                {
                    // Unable to parse the IP address: putting back characters.
                    putback(is, result);
                    is.setstate(std::ios_base::failbit);
                }
                else
                {
                    if (buf != nullptr) {
                        *buf = result;
                    }
                }
            }
        }
    }

    return is;
}

template std::istream& read_generic_ip_address<boost::asio::ip::address_v4>(std::istream&, boost::asio::ip::address_v4&, std::string*);
template std::istream& read_generic_ip_address<boost::asio::ip::address_v6>(std::istream&, boost::asio::ip::address_v6&, std::string*);

std::istream& read_hostname_label(std::istream& is, std::string& value, std::string* buf)
{
    // Parse hostname labels according to RFC1123.
    if (is.good())
    {
        if (!is_hostname_label_regular_character(is.peek()))
        {
            is.setstate(std::ios_base::failbit);

            return is;
        }

        std::ostringstream oss;

        do
        {
            oss.put(static_cast<char>(is.get()));
        }
        while (is.good() && is_hostname_label_character(is.peek()));

        if (is)
        {
            const std::string& result = oss.str();

            // Check if the label is too long, if the last character is not a regular character or if it contains only digits
            if ((result.size() > HOSTNAME_LABEL_MAX_SIZE) || (!is_hostname_label_regular_character(result[result.size() - 1])) || (result.find_first_not_of("0123456789") == std::string::npos))
            {
                putback(is, result);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            value = result;

            if (buf != nullptr) {
                *buf = value;
            }
        }
    }

    return is;
}

std::istream& read_hostname(std::istream& is, std::string& value, std::string* buf)
{
    // Parse hostnames labels according to RFC952 and RFC1123.
    std::string label;

    if (read_hostname_label(is, label))
    {
        if (is.eof())
        {
            // There is nothing more to read: lets use the content of the first label
            std::swap(value, label);

            if (buf != nullptr) {
                *buf = value;
            }
        }
        else
        {
            std::ostringstream oss(label);
            oss.seekp(0, std::ios::end);

            while (is.good() && (is.peek() == '.'))
            {
                is.ignore();
                oss.put('.');

                if (!read_hostname_label(is, label))
                {
                    putback(is, oss.str());
                    is.setstate(std::ios_base::failbit);

                    return is;
                }

                oss << label;
            }

            if (is)
            {
                const std::string& result = oss.str();

                if (result.size() > HOSTNAME_MAX_SIZE)
                {
                    putback(is, result);
                    is.setstate(std::ios_base::failbit);

                    return is;
                }

                value = result;

                if (buf != nullptr) {
                    *buf = value;
                }
            }
        }
    }

    return is;
}

template <typename IntegerType, IntegerType MinValue = std::numeric_limits<IntegerType>::min(), IntegerType MaxValue = std::numeric_limits<IntegerType>::max()>
std::istream& read_unsigned_integer(std::istream& is, IntegerType& value, std::string* buf) {
    if (is.good())
    {
        if (!is_unsigned_integer_character(is.peek()))
        {
            is.setstate(std::ios_base::failbit);

            return is;
        }

        std::stringstream oss;

        do
        {
            oss.put(static_cast<char>(is.get()));
        }
        while (is.good() && is_unsigned_integer_character(is.peek()));

        if (is)
        {
            const std::string& result = oss.str();
            uint32_t tmp_value;
            oss >> tmp_value;

            if (!oss || (tmp_value < MinValue) || (tmp_value > MaxValue)) {
                putback(is, result);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            value = tmp_value;

            if (buf != nullptr) {
                *buf = result;
            }
        }
    }

    return is;
}

std::istream& read_port_number(std::istream& is, uint16_t& value, std::string* buf) {
    return read_unsigned_integer<uint16_t, 0, 65535>(is, value, buf);
}

template <typename AddressType>
std::istream& read_generic_ip_prefix_length(std::istream& is, uint8_t& value, std::string* buf) {
    return read_unsigned_integer<uint8_t, 0, get_ip_max_prefix_length<AddressType>::value>(is, value, buf);
}

template std::istream& read_generic_ip_prefix_length<boost::asio::ip::address_v4>(std::istream&, uint8_t&, std::string*);
template std::istream& read_generic_ip_prefix_length<boost::asio::ip::address_v6>(std::istream&, uint8_t&, std::string*);

template <typename IPAddressType>
std::istream& read_generic_ip_endpoint(std::istream& is, IPAddressType& ip_address, PortNumber& port_number, std::string* buf);

template <>
std::istream& read_generic_ip_endpoint<IPv4Address>(std::istream& is, IPv4Address& ip_address, PortNumber& port_number, std::string* buf)
{
    if (is.good())
    {
        std::string ip_address_buf;

        if (IPv4Address::read_from(is, ip_address, &ip_address_buf))
        {
            if (!is_endpoint_separator(is.peek())) {
                putback(is, ip_address_buf);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            is.ignore();
            std::string port_number_buf;

            if (!PortNumber::read_from(is, port_number, &port_number_buf)) {
                putback(is, ip_address_buf + ':');
                is.setstate(std::ios_base::failbit);

                return is;
            }

            if (buf) {
                *buf = ip_address_buf + ':' + port_number_buf;
            }
        }
    }

    return is;
}

template <>
std::istream& read_generic_ip_endpoint<IPv6Address>(std::istream& is, IPv6Address& ip_address, PortNumber& port_number, std::string* buf)
{
    if (is.good())
    {
        if (!is_ipv6_endpoint_address_start_delimiter(is.peek())) {
            is.setstate(std::ios_base::failbit);

            return is;
        }

        is.ignore();
        std::string ip_address_buf;

        if (!IPv6Address::read_from(is, ip_address, &ip_address_buf))
        {
            putback(is, "[");
            is.setstate(std::ios_base::failbit);

            return is;
        }

        if (!is.good() || !is_ipv6_endpoint_address_stop_delimiter(is.peek())) {
            putback(is, '[' + ip_address_buf);
            is.setstate(std::ios_base::failbit);

            return is;
        }

        is.ignore();

        if (!is.good() || !is_endpoint_separator(is.peek())) {
            putback(is, '[' + ip_address_buf + ']');
            is.setstate(std::ios_base::failbit);

            return is;
        }

        is.ignore();
        std::string port_number_buf;

        if (!PortNumber::read_from(is, port_number, &port_number_buf)) {
            putback(is, '[' + ip_address_buf + ']' + ':');
            is.setstate(std::ios_base::failbit);

            return is;
        }

        if (buf) {
            *buf = '[' + ip_address_buf + ']' + ':' + port_number_buf;
        }
    }

    return is;
}

std::istream& read_hostname_endpoint(std::istream& is, Hostname& hostname, PortNumber& port_number, std::string* buf)
{
    if (is.good())
    {
        std::string hostname_buf;

        if (Hostname::read_from(is, hostname, &hostname_buf))
        {
            if (!is_endpoint_separator(is.peek())) {
                putback(is, hostname_buf);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            is.ignore();
            std::string port_number_buf;

            if (!PortNumber::read_from(is, port_number, &port_number_buf)) {
                putback(is, hostname_buf + ':');
                is.setstate(std::ios_base::failbit);

                return is;
            }

            if (buf) {
                *buf = hostname_buf + ':' + port_number_buf;
            }
        }
    }

    return is;
}

template <typename IPAddressType, typename IPPrefixLengthType>
std::istream& read_generic_ip_route(std::istream& is, IPAddressType& ip_address, IPPrefixLengthType& prefix_length, boost::optional<IPAddressType>& gateway, std::string* buf)
{
    static_cast<void>(gateway);

    if (is.good())
    {
        std::string ip_address_buf;

        if (IPAddressType::read_from(is, ip_address, &ip_address_buf))
        {
            if (!is_route_separator(is.peek())) {
                putback(is, ip_address_buf);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            const char sep = is.get();
            std::string prefix_length_buf;

            if (!IPPrefixLengthType::read_from(is, prefix_length, &prefix_length_buf)) {
                putback(is, ip_address_buf + sep);
                is.setstate(std::ios_base::failbit);

                return is;
            }

            if (is.good() && is_route_gateway_separator(is.peek())) {
                const char gsep = is.get();
                std::string gateway_buf;

                gateway = IPAddressType();

                if (!IPAddressType::read_from(is, *gateway, &gateway_buf))
                {
                    putback(is, ip_address_buf + sep + prefix_length_buf + gsep);
                    is.setstate(std::ios_base::failbit);

                    return is;
                }

                if (buf) {
                    *buf = ip_address_buf + sep + prefix_length_buf + gsep + gateway_buf;
                }
            } else {
                gateway = boost::none;

                if (buf) {
                    *buf = ip_address_buf + sep + prefix_length_buf;
                }
            }
        }
    }

    return is;
}

template std::istream& read_generic_ip_route<IPv4Address, IPv4PrefixLength>(std::istream&, IPv4Address&, IPv4PrefixLength&, boost::optional<IPv4Address>&, std::string*);
template std::istream& read_generic_ip_route<IPv6Address, IPv6PrefixLength>(std::istream&, IPv6Address&, IPv6PrefixLength&, boost::optional<IPv6Address>&, std::string*);

std::istream& read_ethernet_address(std::istream& is, EthernetAddress& value, std::string* buf) {
    static const size_t ethernet_address_string_size = 6 * 2 + 5;
    char data[ethernet_address_string_size];

    if (is.read(data, sizeof(data)))
    {
        // Check for separators.
        for (size_t i = 2; i < sizeof(data); i += 3)
        {
            if (!is_ethernet_address_separator(data[i]))
            {
                putback(is, std::string(data, sizeof(data)));
                is.setstate(std::ios_base::failbit);

                return is;
            }
        }

        EthernetAddressBytes bytes;

        // We check for digits to be hexadecimal
        for (size_t i = 0; i < sizeof(data); i += 3)
        {
            if (!std::isxdigit(data[i]) || !std::isxdigit(data[i + 1]))
            {
                putback(is, std::string(data, sizeof(data)));
                is.setstate(std::ios_base::failbit);

                return is;
            }
            else
            {
                bytes[i / 3] = (xdigit_to_numeric(data[i]) << 4) | xdigit_to_numeric(data[i + 1]);
            }
        }

        // Makes sure to set the eof flag if need be.
        is.peek();

        value = EthernetAddress(bytes);

        if (buf) {
            *buf = std::string(data, ethernet_address_string_size);
        }
    }

    return is;
}

}

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
	constexpr uint8_t get_ip_max_prefix_length();

	template <>
	constexpr uint8_t get_ip_max_prefix_length<boost::asio::ip::address_v4>() {
		return 32;
	}

	template <>
	constexpr uint8_t get_ip_max_prefix_length<boost::asio::ip::address_v6>() {
		return 128;
	}

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
		return std::isdigit(c);
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
		}
		else
		{
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
				}
				else
				{
					value = result;

					if (buf != nullptr) {
						*buf = value;
					}
				}
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
				}
				else
				{
					oss << label;
				}
			}

			if (is)
			{
				const std::string& result = oss.str();

				if (result.size() > HOSTNAME_MAX_SIZE)
				{
					putback(is, result);
					is.setstate(std::ios_base::failbit);
				}
				else
				{
					value = result;

					if (buf != nullptr) {
						*buf = value;
					}
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
		}
		else
		{
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
				} else {
					value = tmp_value;

					if (buf != nullptr) {
						*buf = result;
					}
				}
			}
		}
	}

	return is;
}

std::istream& read_port_number(std::istream& is, uint16_t& value, std::string* buf) {
	return read_unsigned_integer<uint16_t>(is, value, buf);
}

template <typename AddressType>
std::istream& read_generic_ip_prefix_length(std::istream& is, uint8_t& value, std::string* buf) {
	return read_unsigned_integer<uint8_t, 0, get_ip_max_prefix_length<AddressType>()>(is, value, buf);
}

template std::istream& read_generic_ip_prefix_length<boost::asio::ip::address_v4>(std::istream&, uint8_t&, std::string*);
template std::istream& read_generic_ip_prefix_length<boost::asio::ip::address_v6>(std::istream&, uint8_t&, std::string*);
}

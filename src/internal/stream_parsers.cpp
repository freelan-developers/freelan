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

	std::istream& putback(std::istream& is, const std::string& str)
	{
		std::ios::iostate state = is.rdstate();
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

}

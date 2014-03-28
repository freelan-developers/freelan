/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file ethernet_address.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Ethernet address class.
 */

#include "osi/ethernet_address.hpp"

#include <iomanip>

#include <boost/io/ios_state.hpp>
#include <boost/bind.hpp>

namespace asiotap
{
	namespace osi
	{
		namespace
		{
			std::istream& putback(std::istream& is, const char* data, size_t data_len)
			{
				std::ios::iostate state = is.rdstate();
				is.clear();

				std::for_each(data, data + data_len, boost::bind(&std::istream::putback, boost::ref(is), _1));

				is.setstate(state);

				return is;
			}

			bool is_ethernet_address_separator(char c)
			{
				return (c == ':') || (c == '-');
			}

			unsigned int xdigit_to_numeric(char c)
			{
				assert(isxdigit(c));

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
		}

		std::ostream& operator<<(std::ostream& os, const ethernet_address& value)
		{
			// This will save the flags and restore them when the function exits.
			boost::io::ios_flags_saver ifs(os);

			os << std::hex << std::setfill('0') << std::setw(2);

			os << static_cast<unsigned int>(value.data()[0]);

			for (size_t i = 1; i < value.data().size(); ++i)
			{
				os << ':' << std::setw(2) << static_cast<unsigned int>(value.data()[i]);
			}

			return os;
		}

		std::istream& operator>>(std::istream& is, ethernet_address& value)
		{
			static const size_t ethernet_address_string_size = 6 * 2 + 5;

			char data[ethernet_address_string_size];

			if (is.read(data, sizeof(data)))
			{
				// We check for separators
				for (size_t i = 2; i < sizeof(data); i += 3)
				{
					if (!is_ethernet_address_separator(data[i]))
					{
						putback(is, data, sizeof(data));
						is.setstate(std::ios_base::failbit);

						return is;
					}
				}

				ethernet_address::data_type raw;

				// We check for digits to be hexadecimal
				for (size_t i = 0; i < sizeof(data); i += 3)
				{
					if (!(isxdigit(data[i]) && isxdigit(data[i + 1])))
					{
						putback(is, data, sizeof(data));
						is.setstate(std::ios_base::failbit);

						return is;
					}
					else
					{
						raw[i / 3] = (xdigit_to_numeric(data[i]) << 4) | xdigit_to_numeric(data[i + 1]);
					}
				}

				value = ethernet_address(raw);
			}

			return is;
		}
	}
}

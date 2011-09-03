/*
 * libiconvplus - A lightweight C++ wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file converter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv converter class.
 */

#include "converter.hpp"

#include <boost/system/system_error.hpp>

#include <cstring>

namespace iconvplus
{
	bool converter::convert(const iconv_instance& ic, std::istream& is, std::ostream& os, boost::system::error_code& ec, size_t* non_reversible_conversions) const
	{
		size_t counter = 0;

		if (!non_reversible_conversions) non_reversible_conversions = &counter;

		ic.reset();

		size_t itmp_len = 0;
		const char* inbuf = NULL;
		size_t otmp_len = m_obuf.size();
		char* outbuf = &m_obuf[0];

		if (!ic.write_initial_state(&outbuf, &otmp_len, ec))
		{
			return false;
		}
		else
		{
			os.write(&m_obuf[0], m_obuf.size() - otmp_len);
		}

		size_t result;

		while (is)
		{
			is.read(&m_ibuf[itmp_len], m_ibuf.size() - itmp_len);

			if (is.good() || is.eof())
			{
				itmp_len += is.gcount();
				inbuf = &m_ibuf[0];

				do
				{
					otmp_len = m_obuf.size();
					outbuf = &m_obuf[0];

					result = ic.convert(&inbuf, &itmp_len, &outbuf, &otmp_len, ec);

					os.write(&m_obuf[0], m_obuf.size() - otmp_len);

					if (result == iconv_instance::ERROR_VALUE)
					{
						if (ec.value() == E2BIG)
						{
							// We check if the destination buffer will always be too small.
							if (otmp_len >= m_obuf.size())
							{
								return false;
							}
						}
						else if (ec.value() == EINVAL)
						{
							// An incomplete multi-byte sequence was cut. Lets copy the bytes to the beginning of the next input buffer and try again.
							std::memmove(&m_ibuf[0], inbuf, itmp_len);
						}
						else
						{
							return false;
						}
					}

					non_reversible_conversions += result;
				}
				while ((result == iconv_instance::ERROR_VALUE) && (ec.value() == E2BIG));
			}
		}

		return true;
	}

	void converter::convert(const iconv_instance& ic, std::istream& is, std::ostream& os, size_t* non_reversible_conversions) const
	{
		boost::system::error_code ec;

		if (!convert(ic, is, os, ec, non_reversible_conversions))
		{
			throw boost::system::system_error(ec);
		}
	}
}

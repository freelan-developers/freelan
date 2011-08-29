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
 * \file iconv_error_category.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv error category.
 */

#include "converter.hpp"

namespace iconvplus
{
	bool converter::convert(iconv& ic, boost::system::error_code& ec, size_t* non_reversible_conversions) const
	{
		size_t counter = 0;

		if (!non_reversible_conversions) non_reversible_conversions = &counter;

		ic.reset();

		while (m_is)
		{
			m_is.read(&m_ibuf[0], m_ibuf.size());

			if (m_is.good() || m_is.eof())
			{
				size_t itmp_len = m_is.gcount();
				const char* inbuf = &m_ibuf[0];
				size_t otmp_len = m_obuf.size();
				char* outbuf = &m_obuf[0];

				size_t result = ic.convert(&inbuf, &itmp_len, &outbuf, &otmp_len, ec);

				if ((result == iconv::ERROR_VALUE) && (ec.value() != E2BIG))
				{
					return false;
				}

				non_reversible_conversions += result;

				m_os.write(&m_obuf[0], m_obuf.size() - otmp_len);
			}
		}

		return true;
	}
	
	void converter::convert(iconv& ic, size_t* non_reversible_conversions) const
	{
		boost::system::error_code ec;

		if (!convert(ic, ec, non_reversible_conversions))
		{
			throw ec;
		}
	}
}

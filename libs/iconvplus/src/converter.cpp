/*
 * libiconvplus - A lightweight C++ wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libiconvplus.
 *
 * libiconvplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libiconvplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libiconvplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file converter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv converter class.
 */

#include "converter.hpp"

#include <cstring>

namespace iconvplus
{
	template <typename InputCharType, typename OutputCharType>
	bool converter<InputCharType, OutputCharType>::convert(input_stream_type& is, output_stream_type& os, boost::system::error_code& ec, size_t* non_reversible_conversions)
	{
		assert(m_output_buffer.size() > 0);
		assert(m_input_buffer.size() > 0);

		size_t counter = 0;

		if (!non_reversible_conversions) non_reversible_conversions = &counter;

		m_iconv_instance->reset();

		size_t itmp_size = 0;
		const char* inbuf = NULL;
		size_t otmp_size = m_output_buffer.size() * sizeof(output_char_type);
		char* outbuf = reinterpret_cast<char*>(&m_output_buffer[0]);

		if (!m_iconv_instance->write_initial_state(&outbuf, &otmp_size, ec))
		{
			return false;
		}
		else
		{
			os.write(&m_output_buffer[0], m_output_buffer.size() - otmp_size / sizeof(output_char_type));
		}

		size_t result;

		while (is)
		{
			is.read(&m_input_buffer[itmp_size / sizeof(input_char_type)], m_input_buffer.size() - itmp_size / sizeof(input_char_type));

			if (is.good() || is.eof())
			{
				itmp_size += static_cast<size_t>(is.gcount()) * sizeof(input_char_type);
				inbuf = reinterpret_cast<const char*>(&m_input_buffer[0]);

				do
				{
					otmp_size = static_cast<size_t>(m_output_buffer.size()) * sizeof(output_char_type);
					outbuf = reinterpret_cast<char*>(&m_output_buffer[0]);

					result = m_iconv_instance->convert(&inbuf, &itmp_size, &outbuf, &otmp_size, ec);

					os.write(&m_output_buffer[0], m_output_buffer.size() - otmp_size / sizeof(output_char_type));

					if (result == iconv_instance::ERROR_VALUE)
					{
						if (ec.value() == E2BIG)
						{
							// We check if the destination buffer will always be too small.
							if (otmp_size >= m_output_buffer.size() * sizeof(output_char_type))
							{
								return false;
							}
						}
						else if (ec.value() == EINVAL)
						{
							// An incomplete multi-byte sequence was cut. Lets copy the bytes to the beginning of the next input buffer and try again.
							std::memmove(reinterpret_cast<char*>(&m_input_buffer[0]), inbuf, itmp_size);
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

	template class converter<char, char>;
	template class converter<char, wchar_t>;
	template class converter<wchar_t, char>;
	template class converter<wchar_t, wchar_t>;
}

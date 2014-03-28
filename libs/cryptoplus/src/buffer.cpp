/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file buffer.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A buffer class.
 */

#include "buffer.hpp"

#include <sstream>
#include <cstring>
#include <iomanip>

#include <boost/io/ios_state.hpp>

namespace cryptoplus
{
	buffer::buffer(const void* _data, size_t data_len) :
		m_data(static_cast<const uint8_t*>(_data), static_cast<const uint8_t*>(_data) + data_len)
	{
	}

	buffer::buffer(const char* str) :
		m_data(static_cast<const uint8_t*>(static_cast<const void*>(str)), static_cast<const uint8_t*>(static_cast<const void*>(str)) + strlen(str))
	{
	}

	buffer::buffer(const std::string& str) :
		m_data(str.begin(), str.end())
	{
	}

	buffer::buffer(const storage_type& _data) :
		m_data(_data)
	{
	}

	std::ostream& operator<<(std::ostream& os, const buffer& buf)
	{
	    boost::io::ios_flags_saver ifs(os);

		for (buffer::storage_type::const_iterator i = buf.data().begin(); i != buf.data().end(); ++i)
		{
			os << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*i);
		}

		return os;
	}

	std::string hex(const buffer& buf)
	{
		std::ostringstream oss;

		oss << buf;

		return oss.str();
	}
}

/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
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
 * \file message.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The messages exchanged by the peers.
 */

#include "message.hpp"

#include <cassert>

namespace freelan
{
	size_t message::write(void* buf, size_t buf_len, message_type _type, size_t _length)
	{
		if (buf_len < HEADER_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		fscp::buffer_tools::set<uint8_t>(buf, 0, static_cast<uint8_t>(_type));
		fscp::buffer_tools::set<uint16_t>(buf, 1, htons(static_cast<uint16_t>(_length)));

		return HEADER_LENGTH + _length;
	}

	message::message(const void* buf, size_t buf_len) :
		m_data(buf)
	{
		if (buf_len < HEADER_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		if (buf_len < HEADER_LENGTH + length())
		{
			throw std::runtime_error("buf_len");
		}
	}
}

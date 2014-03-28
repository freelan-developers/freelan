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
 * \file routes_request_message.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The routes request messages exchanged by the peers.
 */

#include "routes_request_message.hpp"

#include <cassert>

namespace freelan
{
	size_t routes_request_message::write(void* buf, size_t buf_len)
	{
		return message::write(buf, buf_len, MT_ROUTES_REQUEST, 0);
	}

	routes_request_message::routes_request_message(const void* buf, size_t buf_len) :
		message(buf, buf_len)
	{
		if (length() != 0)
		{
			throw std::runtime_error("bad message length");
		}
	}

	routes_request_message::routes_request_message(const message& _message) :
		message(_message)
	{
		if (length() != 0)
		{
			throw std::runtime_error("bad message length");
		}
	}
}

/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file session_pair.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session pair class.
 */

#include "session_pair.hpp"

namespace fscp
{
	bool session_pair::renew_local_session(session_store::session_number_type session_number)
	{
		if (has_local_session())
		{
			if ((session_number > local_session().session_number()) || local_session().is_old())
			{
				m_local_session = boost::make_optional(session_store(std::max(local_session().session_number() + 1, session_number)));

				return true;
			}
			else
			{
				local_session().set_sequence_number(0);
			}
		}
		else
		{
			m_local_session = boost::make_optional(session_store(session_number));

			return true;
		}

		return false;
	}

	void session_pair::set_remote_session(const session_store& session)
	{
		keep_alive();

		m_remote_session = session;
	}
}

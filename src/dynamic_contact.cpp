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
 * \file dynamic_contact.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A dynamic contact class.
 */

#include "dynamic_contact.hpp"

namespace freelan
{
	const boost::posix_time::time_duration dynamic_contact::ENDPOINT_VALIDITY_DURATION = boost::posix_time::minutes(3);

	void dynamic_contact::add_candidate_endpoint(const ep_type& ep, boost::posix_time::ptime expiration_date)
	{
		if (m_candidate_endpoint_map[ep] < expiration_date)
		{
			m_candidate_endpoint_map[ep] = expiration_date;
		}
	}

	const std::vector<dynamic_contact::ep_type>& dynamic_contact::get_candidate_endpoint_list()
	{
		const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

		m_candidate_endpoint_list_cache.clear();

		for (std::map<ep_type, boost::posix_time::ptime>::iterator it = m_candidate_endpoint_map.begin(); it != m_candidate_endpoint_map.end(); )
		{
			if (it->second < now)
			{
				m_candidate_endpoint_map.erase(it++);
			}
			else
			{
				m_candidate_endpoint_list_cache.push_back(it->first);

				++it;
			}
		}

		return m_candidate_endpoint_list_cache;
	}
}

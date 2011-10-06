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
 * \file switch.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A switch class.
 */

#include "switch.hpp"

namespace
{
	const boost::array<uint8_t, 6> BROADCAST_ADDRESS = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
}

namespace freelan
{
	bool switch_::update_entry(const ethernet_address_type& address, const ep_type& endpoint)
	{
		if (is_broadcast(address))
		{
			return false;
		}

		const map_type::iterator entry = m_map.find(address);

		if (entry != m_map.end())
		{
			entry->second.endpoint = endpoint;
			entry->second.date = boost::posix_time::second_clock::local_time();
			return false;
		} else
		{
			if (m_map.size() >= m_max_entries)
			{
				remove_older_entry();
			}

			m_map.insert(std::make_pair(address, endpoint));

			return true;
		}
	}

	bool switch_::get_entry(const ethernet_address_type& address, ep_type& endpoint)
	{
		const map_type::const_iterator entry = m_map.find(address);

		if (entry != m_map.end())
		{
			endpoint = entry->second.endpoint;
			return true;
		}

		return false;
	}

	bool switch_::ethernet_address_comp::operator() (const ethernet_address_type& lhs, const ethernet_address_type& rhs) const
	{
		assert(boost::asio::buffer_size(lhs) == boost::asio::buffer_size(rhs));

		return (std::memcmp(boost::asio::buffer_cast<const void*>(lhs), boost::asio::buffer_cast<const void*>(rhs), boost::asio::buffer_size(lhs)) < 0);
	}

	bool switch_::is_broadcast(const ethernet_address_type& address)
	{
		assert(boost::asio::buffer_size(address) == ::BROADCAST_ADDRESS.size());

		return (std::memcmp(boost::asio::buffer_cast<const void*>(address), &::BROADCAST_ADDRESS[0], BROADCAST_ADDRESS.size()) == 0);
	}

	void switch_::remove_older_entry()
	{
		map_type::iterator entry = m_map.begin();

		if (m_map.size() > 1)
		{
			for (map_type::iterator it = m_map.begin(); it != m_map.end(); ++it)
			{
				if (it->second.date < entry->second.date)
				{
					entry = it;
				}
			}
		}

		if (entry != m_map.end())
		{
			m_map.erase(entry);
		}
	}

}

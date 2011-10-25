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

#include <cassert>

#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <boost/version.hpp>

#if BOOST_VERSION >= 104700
#include <boost/random/uniform_int_distribution.hpp>
#else
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#endif

#include <asiotap/osi/ethernet_helper.hpp>

#include "tap_adapter_switch_port.hpp"

namespace freelan
{
	const unsigned int switch_::MAX_ENTRIES_DEFAULT = 1024;

	void switch_::receive_data(port_type port, boost::asio::const_buffer data)
	{
		assert(port);

		switch (m_configuration.routing_method)
		{
			case switch_configuration::RM_HUB:
				{
					send_data_from(port, data);

					break;
				}
			case switch_configuration::RM_SWITCH:
				{
					asiotap::osi::const_helper<asiotap::osi::ethernet_frame> ethernet_helper(data);

					const ethernet_address_type target_address = to_ethernet_address(ethernet_helper.target());

					if (!is_multicast_address(target_address))
					{
						m_ethernet_address_map[to_ethernet_address(ethernet_helper.sender())] = port;

						// We exceeded the maximum count for entries: we delete random entries to fix it.
						while (m_ethernet_address_map.size() > m_max_entries)
						{
							ethernet_address_map_type::iterator entry = m_ethernet_address_map.begin();

#if BOOST_VERSION >= 104700
							boost::random::mt19937 gen;

							std::advance(entry, boost::random::uniform_int_distribution<>(0, m_ethernet_address_map.size() - 1)(gen));
#else
							boost::mt19937 gen;

							boost::variate_generator<boost::mt19937&, boost::uniform_int<> > vgen(gen, boost::uniform_int<>(0, m_ethernet_address_map.size() - 1));
							std::advance(entry, vgen());
#endif

							m_ethernet_address_map.erase(entry);
						}

						// We look in the ethernet address map

						const ethernet_address_map_type::iterator target_entry = m_ethernet_address_map.find(target_address);

						if (target_entry != m_ethernet_address_map.end())
						{
							port_type target_port = target_entry->second.lock();

							if (target_port)
							{
								send_data_from_to(port, target_port, data);
							}
							else
							{
								// The port is no longer valid: we delete the entry.
								m_ethernet_address_map.erase(target_entry);
							}
						}
						else
						{
							// No target entry: we send the message to everybody.
							send_data_from(port, data);
						}
					}
					else
					{
						// Address is multicast: we send to everybody.
						send_data_from(port, data);
					}
				}
		}
	}

	void switch_::send_data_from(port_type source_port, boost::asio::const_buffer data)
	{
		BOOST_FOREACH(port_list_type::value_type& entry, m_ports)
		{
			send_data_from_to(source_port, entry.first, data);
		}
	}

	void switch_::send_data_from_to(port_type source_port, port_type target_port, boost::asio::const_buffer data)
	{
		if (source_port != target_port)
		{
			if (m_configuration.relay_mode_enabled || (m_ports[source_port] != m_ports[target_port]))
			{
				target_port->write(data);
			}
		}
	}

	switch_::ethernet_address_type switch_::to_ethernet_address(boost::asio::const_buffer buf)
	{
		assert(boost::asio::buffer_size(buf) == ethernet_address_type::static_size);

		ethernet_address_type result;

		std::memcpy(result.c_array(), boost::asio::buffer_cast<const ethernet_address_type::value_type*>(buf), result.size());

		return result;
	}

	bool switch_::is_multicast_address(const switch_::ethernet_address_type& address)
	{
		return ((address[0] & 0x01) != 0x00);
	}
}

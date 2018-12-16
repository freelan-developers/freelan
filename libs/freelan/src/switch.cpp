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

#include <boost/thread/mutex.hpp>
#include <boost/make_shared.hpp>

#include <asiotap/osi/ethernet_helper.hpp>

namespace freelan
{
	namespace
	{
		template <typename KeyType, typename ValueType, typename Handler>
		class results_gatherer
		{
			public:

				typedef std::set<KeyType> set_type;
				typedef std::map<KeyType, ValueType> map_type;

				results_gatherer(Handler handler, const set_type& keys) :
					m_handler(handler),
					m_keys(keys)
				{
					if (m_keys.empty())
					{
						m_handler(m_results);
					}
				}

				void gather(const KeyType& key, const ValueType& value)
				{
					boost::mutex::scoped_lock lock(m_mutex);

					const size_t erased_count = m_keys.erase(key);

					// Ensure that gather was called only once for a given key.
					assert(erased_count == 1);

					m_results[key] = value;

					if (m_keys.empty())
					{
						m_handler(m_results);
					}
				}

			private:

				boost::mutex m_mutex;
				Handler m_handler;
				set_type m_keys;
				map_type m_results;
		};
	}

	const unsigned int switch_::MAX_ENTRIES_DEFAULT = 1024;

	void switch_::async_write(port_index_type index, boost::asio::const_buffer data, multi_write_handler_type handler)
	{
		typedef results_gatherer<port_index_type, boost::system::error_code, multi_write_handler_type> results_gatherer_type;

		const auto targets = get_targets_for(index, data);

#if FREELAN_DEBUG
		if (!targets.empty())
		{
			std::cerr << "Switching " << buffer_size(data) << " byte(s) of data from " << index << " to " << targets.size() << " host(s)." << std::endl;
		}
		else
		{
			std::cerr << "Switching " << buffer_size(data) << " byte(s) of data from " << index << ": no targets." << std::endl;
		}
#endif

		boost::shared_ptr<results_gatherer_type> rg = boost::make_shared<results_gatherer_type>(handler, targets);

		for (auto&& target : targets)
		{
#if FREELAN_DEBUG
			std::cerr << index << "-> " << target << std::endl;
#endif

			if(index == target)
			{
#if FREELAN_DEBUG
				std::cerr << "Index matching target forbidden (" << index << "-> " << target << ")" << std::endl;
#endif
			}
			else
			{
				m_ports[target].async_write(data, boost::bind(&results_gatherer_type::gather, rg, target, _1));
			}
		}
	}

	std::set<port_index_type> switch_::get_targets_for(port_index_type index, boost::asio::const_buffer data)
	{
		const port_list_type::iterator source_port_entry = m_ports.find(index);

		if (source_port_entry != m_ports.end())
		{
			switch (m_configuration.routing_method)
			{
				case switch_configuration::RM_HUB:
				{
					return get_targets_for(source_port_entry);
				}
				case switch_configuration::RM_SWITCH:
				{
					asiotap::osi::const_helper<asiotap::osi::ethernet_frame> ethernet_helper(data);

					const ethernet_address_type target_address = to_ethernet_address(ethernet_helper.target());

					if (is_multicast_address(target_address))
					{
						return get_targets_for(source_port_entry);
					}
					else
					{
						m_ethernet_address_map[to_ethernet_address(ethernet_helper.sender())] = index;

						// We exceeded the maximum count for entries: we delete random entries to fix it.
						while (m_ethernet_address_map.size() > m_max_entries)
						{
							ethernet_address_map_type::iterator entry = m_ethernet_address_map.begin();

#if BOOST_VERSION >= 104700
							boost::random::mt19937 gen;

							std::advance(entry, boost::random::uniform_int_distribution<>(0, static_cast<int>(m_ethernet_address_map.size()) - 1)(gen));
#else
							boost::mt19937 gen;

							boost::variate_generator<boost::mt19937&, boost::uniform_int<> > vgen(gen, boost::uniform_int<>(0, m_ethernet_address_map.size() - 1));
							std::advance(entry, vgen());
#endif

							m_ethernet_address_map.erase(entry);
						}

						// We look in the ethernet address map

						const ethernet_address_map_type::iterator target_entry = m_ethernet_address_map.find(target_address);

						if (target_entry == m_ethernet_address_map.end())
						{
							// No target entry: we send the message to everybody.
							return get_targets_for(source_port_entry);
						}

						const port_index_type target_port_index = target_entry->second;

						if (!is_registered(target_port_index))
						{
							// The port does not exist: we delete the entry and send to everybody.
							m_ethernet_address_map.erase(target_entry);

							return get_targets_for(source_port_entry);
						}

						std::set<port_index_type> targets;

						targets.insert(target_port_index);

						return targets;
					}
				}
			}
		}

		return std::set<port_index_type>();
	}

	std::set<port_index_type> switch_::get_targets_for(port_list_type::const_iterator source_port_entry)
	{
		std::set<port_index_type> targets;

		for (port_list_type::const_iterator port_entry = m_ports.begin(); port_entry != m_ports.end(); ++port_entry)
		{
			if (source_port_entry != port_entry)
			{
				if (m_configuration.relay_mode_enabled || (source_port_entry->second.group() != port_entry->second.group()))
				{
					targets.insert(port_entry->first);
				}
			}
		}

		return targets;
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

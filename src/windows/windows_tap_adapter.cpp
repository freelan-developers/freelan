/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file windows_tap_adapter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows tap adapter class.
 */

#include "windows/windows_tap_adapter.hpp"

#include "registry.hpp"

#include <boost/optional.hpp>

#include <vector>
#include <iterator>

#include <winioctl.h>
#include <iphlpapi.h>
#include <winbase.h>
#include <shellapi.h>

#include "../windows/common.h"

namespace asiotap
{
	namespace
	{
		typedef std::vector<std::string> guid_array_type;
		typedef std::map<std::string, std::string> guid_map_type;
		typedef std::pair<std::string, std::string> guid_pair_type;

		guid_array_type enumerate_tap_adapters_guid()
		{
			guid_array_type tap_adapters_list;

			HKEY adapter_key;
			LONG status;

			const std::string tap_component_id(TAP_COMPONENT_ID);
			const registry_key adapter_key(HKEY_LOCAL_MACHINE, ADAPTER_KEY);

			for (registry_key network_adapter_key : adapter_key)
			{
				const std::string component_id_str = network_adapter_key.query_string("ComponentId");

				if (tap_component_id == component_id_str)
				{
					const std::string net_cfg_instance_id_str = network_adapter_key.query_string("NetCfgInstanceId");

					tap_adapters_list.push_back(net_cfg_instance_id_str);
				}
			}

			return tap_adapters_list;
		}

		guid_map_type enumerate_network_connections()
		{
			guid_map_type network_connections_map;

			const registry_key network_connections_key(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY);

			for (registry_key network_connection_key : network_connections_key)
			{
				const registry_key connection_key(network_connection_key, "Connection");
				const std::string name = connection_key.query_string("Name");

				network_connections_map[network_connection_key.name()] = name;
			}

			return network_connections_map;
		}

		guid_map_type enumerate_tap_adapters()
		{
			guid_map_type network_connections_map = enumerate_network_connections();

			const guid_array_type tap_adapters_list = enumerate_tap_adapters_guid();

			guid_map_type tap_adapters_map;

			for (const std::string& guid : tap_adapters_list)
			{
				if (network_connections_map.find(guid) != network_connections_map.end())
				{
					tap_adapters_map[guid] = network_connections_map[guid];
				}
			}

			return tap_adapters_map;
		}

		guid_pair_type find_tap_adapter_by_guid(const std::string& guid)
		{
			const guid_map_type tap_adapters_map = enumerate_tap_adapters();

			const guid_map_type::const_iterator it = tap_adapters_map.find(guid);

			if (it == tap_adapters_map.end())
			{
				throw std::runtime_error("No such tap adapter: " + guid);
			}

			return *it;
		}
	}

	std::map<std::string, std::string> windows_tap_adapter::enumerate(tap_adapter_layer)
	{
		return enumerate_tap_adapters();
	}
}

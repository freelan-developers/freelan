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
 * \file tap_adapter_router_port.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A tap adapter router port class.
 */

#include "tap_adapter_router_port.hpp"

#include <boost/foreach.hpp>

namespace freelan
{
	bool tap_adapter_router_port::equals(const router_port& other) const
	{
		const tap_adapter_router_port* casted_other = dynamic_cast<const tap_adapter_router_port*>(&other);

		if (casted_other)
		{
			return (*this == *casted_other);
		}

		return false;
	}

	routes_type tap_adapter_router_port::add_tap_adapter_route(const asiotap::tap_adapter& tap_adapter, const routes_type& routes)
	{
		routes_type result = routes;

		const asiotap::tap_adapter::ip_address_list tap_addresses = tap_adapter.get_ip_addresses();

		BOOST_FOREACH(const asiotap::tap_adapter::ip_address& tap_address, tap_addresses)
		{
			result.insert(to_network_address(tap_address.address));
		}

		return result;
	}
}

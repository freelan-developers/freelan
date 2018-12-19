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
 * \file tap_adapter_configuration.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A tap adapter configuration class.
 */

#ifndef ASIOTAP_TAP_ADAPTER_CONFIGURATION_HPP
#define ASIOTAP_TAP_ADAPTER_CONFIGURATION_HPP

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include "types/ip_network_address.hpp"

namespace asiotap
{
	struct tap_adapter_configuration
	{
		struct ipv4_configuration
		{
			boost::optional<ipv4_network_address> network_address;
			boost::optional<boost::asio::ip::address_v4> remote_address;
			bool dhcp;
		};

		struct ipv6_configuration
		{
			boost::optional<ipv6_network_address> network_address;
		};

		tap_adapter_configuration() : mtu(0) {}

		size_t mtu;
		ipv4_configuration ipv4;
		ipv6_configuration ipv6;
	};
}

#endif /* ASIOTAP_TAP_ADAPTER_CONFIGURATION_HPP */

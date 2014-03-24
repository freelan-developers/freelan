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
 * \file windows_system.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows system functions.
 */

#ifndef ASIOTAP_WINDOWS_SYSTEM_HPP
#define ASIOTAP_WINDOWS_SYSTEM_HPP

#include <vector>
#include <string>

#include <boost/system/system_error.hpp>
#include <boost/optional.hpp>

#include "types/ip_network_address.hpp"
#include "os.hpp"

#include <Iphlpapi.h>

namespace asiotap
{
	// These functions are common to all operating systems.
#ifdef UNICODE
	int execute(const std::vector<std::wstring>& args, boost::system::error_code& ec);
	int execute(const std::vector<std::wstring>& args);
	void checked_execute(const std::vector<std::wstring>& args);
#else
	int execute(const std::vector<std::string>& args, boost::system::error_code& ec);
	int execute(const std::vector<std::string>& args);
	void checked_execute(const std::vector<std::string>& args);
#endif

	// These functions are OS specific.
#ifdef UNICODE
	void netsh(const std::vector<std::wstring>& args);
#else
	void netsh(const std::vector<std::string>& args);
#endif
	void netsh_interface_ip_set_address(const std::string& interface_name, const ip_network_address& address, bool persistent = false);

	void register_route(const NET_LUID& interface_luid, const ip_network_address& route, const boost::optional<boost::asio::ip::address>& gateway, unsigned int metric);
	void unregister_route(const NET_LUID& interface_luid, const ip_network_address& route, const boost::optional<boost::asio::ip::address>& gateway, unsigned int metric);
	void set_unicast_address(const NET_LUID& interface_luid, const ip_network_address& network_address);
}

#endif /* ASIOTAP_WINDOWS_SYSTEM_HPP */

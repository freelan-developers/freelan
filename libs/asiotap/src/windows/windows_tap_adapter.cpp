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

#include "windows/registry.hpp"

#include <boost/optional.hpp>

#include <vector>
#include <iterator>

#include <winioctl.h>
#include <iphlpapi.h>
#include <winbase.h>
#include <shellapi.h>

#include <executeplus/error.hpp>

#include "../../windows/tap-windows.h"

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

			const std::string tap_component_id(TAP_ID);
			const registry_key adapter_key(HKEY_LOCAL_MACHINE, ADAPTER_KEY);

			for (registry_key network_adapter_key : adapter_key.available_keys())
			{
				if (network_adapter_key.is_open())
				{
					try
					{
						const std::string component_id_str = network_adapter_key.query_string("ComponentId");

						if (tap_component_id == component_id_str)
						{
							const std::string net_cfg_instance_id_str = network_adapter_key.query_string("NetCfgInstanceId");

							tap_adapters_list.push_back(net_cfg_instance_id_str);
						}
					}
					catch (const boost::system::system_error&)
					{
					}
				}
			}

			return tap_adapters_list;
		}

		guid_map_type enumerate_network_connections()
		{
			guid_map_type network_connections_map;

			const registry_key network_connections_key(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY);

			for (registry_key network_connection_key : network_connections_key.available_keys())
			{
				if (network_connection_key.is_open())
				{
					try
					{
						const registry_key connection_key(network_connection_key, "Connection");
						const std::string name = connection_key.query_string("Name");

						network_connections_map[network_connection_key.name()] = name;
					}
					catch (const boost::system::system_error&)
					{
					}
				}
			}

			return network_connections_map;
		}

		guid_map_type enumerate_tap_adapters()
		{
			guid_map_type tap_adapters_map;

			try
			{
				guid_map_type network_connections_map = enumerate_network_connections();
				const guid_array_type tap_adapters_list = enumerate_tap_adapters_guid();

				for (const std::string& guid : tap_adapters_list)
				{
					if (network_connections_map.find(guid) != network_connections_map.end())
					{
						tap_adapters_map[guid] = network_connections_map[guid];
					}
				}
			}
			catch (const boost::system::system_error&)
			{
			}

			return tap_adapters_map;
		}

		guid_pair_type find_tap_adapter_by_guid(const std::string& guid)
		{
			const guid_map_type tap_adapters_map = enumerate_tap_adapters();

			const guid_map_type::const_iterator it = tap_adapters_map.find(guid);

			if (it == tap_adapters_map.end())
			{
				throw make_error_code(asiotap_error::no_such_tap_adapter);
			}

			return *it;
		}
	}

	std::map<std::string, std::string> windows_tap_adapter::enumerate(tap_adapter_layer)
	{
		return enumerate_tap_adapters();
	}

	void windows_tap_adapter::open(boost::system::error_code& ec)
	{
		const guid_map_type tap_adapters_map = enumerate_tap_adapters();

		if (tap_adapters_map.empty()) {
			throw std::runtime_error("No tap adapter found. You may want to relaunch the installer to install one.");
		}

		for (auto&& tap_adapter : tap_adapters_map)
		{
			if (!tap_adapter.first.empty())
			{
				open(tap_adapter.first, ec);

				if (!ec)
				{
					return;
				}
			}
		}

		throw std::runtime_error("Some tap adapters were found but none could be opened. Are they all in use already ?");
	}

	void windows_tap_adapter::open(const std::string& _name, boost::system::error_code& ec)
	{
		ec = boost::system::error_code();

		if (_name.empty())
		{
			return open(ec);
		}

		PIP_ADAPTER_INFO piai = NULL;
		ULONG size = 0;
		DWORD status;

		status = GetAdaptersInfo(piai, &size);

		if (status != ERROR_BUFFER_OVERFLOW)
		{
			ec = boost::system::error_code(status, boost::system::system_category());

			return;
		}

		assert(size > 0);

		std::vector<unsigned char> piai_data(size);
		piai = reinterpret_cast<PIP_ADAPTER_INFO>(&piai_data[0]);

		status = GetAdaptersInfo(piai, &size);

		if (status != ERROR_SUCCESS)
		{
			ec = boost::system::error_code(status, boost::system::system_category());

			return;
		}

		piai_data.resize(size);

		try
		{
			const guid_pair_type adapter = find_tap_adapter_by_guid(_name);

			for (PIP_ADAPTER_INFO pi = piai; pi; pi = pi->Next)
			{
				if (adapter.first == std::string(pi->AdapterName))
				{
					const HANDLE handle = CreateFileA(
						(USERMODEDEVICEDIR + adapter.first + TAP_WIN_SUFFIX).c_str(),
						GENERIC_READ | GENERIC_WRITE,
						0,
						0,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
						0
					);

					if (handle == INVALID_HANDLE_VALUE)
					{
						ec = boost::system::error_code(::GetLastError(), boost::system::system_category());

						return;
					}

					if (descriptor().assign(handle, ec))
					{
						return;
					}

					set_name(adapter.first);
					m_display_name = adapter.second;
					m_interface_index = pi->Index;

					if (::ConvertInterfaceIndexToLuid(m_interface_index, &m_interface_luid) != NO_ERROR)
					{
						ec = boost::system::error_code(::GetLastError(), boost::system::system_category());

						return;
					}

					if (pi->AddressLength != ethernet_address().data().size())
					{
						if (close(ec))
						{
							return;
						}

						ec = make_error_code(asiotap_error::no_ethernet_address);

						return;
					}

					osi::ethernet_address _ethernet_address;
					std::memcpy(_ethernet_address.data().data(), pi->Address, pi->AddressLength);
					set_ethernet_address(_ethernet_address);

					DWORD read_mtu;
					DWORD len;

					if (!DeviceIoControl(descriptor().native_handle(), TAP_WIN_IOCTL_GET_MTU, &read_mtu, sizeof(read_mtu), &read_mtu, sizeof(read_mtu), &len, NULL))
					{
						ec = boost::system::error_code(::GetLastError(), boost::system::system_category());

						return;
					}

					set_mtu(static_cast<size_t>(read_mtu));

					break;
				}
			}
		}
		catch (const boost::system::system_error& ex)
		{
			ec = ex.code();

			return;
		}

		if (!is_open())
		{
			ec = make_error_code(asiotap_error::no_such_tap_adapter);
		}
	}

	void windows_tap_adapter::open(const std::string& _name)
	{
		boost::system::error_code ec;

		open(_name, ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}
	}

	void windows_tap_adapter::set_connected_state(bool connected)
	{
		ULONG status = connected ? TRUE : FALSE;
		DWORD len;

		if (!::DeviceIoControl(descriptor().native_handle(), TAP_WIN_IOCTL_SET_MEDIA_STATUS,
					&status, sizeof(status),
					&status, sizeof(status), &len, NULL))
		{
			throw boost::system::system_error(::GetLastError(), boost::system::system_category());
		}
	}

	ip_network_address_list windows_tap_adapter::get_ip_addresses()
	{
		ip_network_address_list result;

		DWORD status;

		std::vector<uint8_t> buffer;
		ULONG buffer_size = 0;

		do
		{
			buffer.resize(buffer_size);
			buffer_size = static_cast<ULONG>(buffer.size());

			status = ::GetAdaptersAddresses(
				AF_UNSPEC,
				GAA_FLAG_INCLUDE_ALL_INTERFACES | GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST,
				NULL, /* reserved */
				reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.empty() ? nullptr : &buffer.front()),
				&buffer_size
			);

			if ((status != ERROR_BUFFER_OVERFLOW) && (status != ERROR_SUCCESS))
			{
				throw boost::system::system_error(status, boost::system::system_category());
			}
		}
		while (status == ERROR_BUFFER_OVERFLOW);

		for (PIP_ADAPTER_ADDRESSES adapter = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer.front()); adapter; adapter = adapter->Next)
		{
			const std::string ifname(adapter->AdapterName);

			if (ifname == name())
			{
				for (const IP_ADAPTER_UNICAST_ADDRESS* unicast_address = adapter->FirstUnicastAddress; unicast_address; unicast_address = unicast_address->Next)
				{
					if (unicast_address->Address.lpSockaddr->sa_family == AF_INET)
					{
						struct sockaddr_in* sai = reinterpret_cast<struct sockaddr_in*>(unicast_address->Address.lpSockaddr);
						boost::asio::ip::address_v4::bytes_type bytes;

						std::memcpy(bytes.data(), &sai->sin_addr, bytes.size());
						boost::asio::ip::address_v4 address(bytes);
						const unsigned int prefix_len = unicast_address->OnLinkPrefixLength;

						result.push_back(ipv4_network_address{ address, prefix_len });
					}
					else
					{
						struct sockaddr_in6* sai = reinterpret_cast<struct sockaddr_in6*>(unicast_address->Address.lpSockaddr);
						boost::asio::ip::address_v6::bytes_type bytes;

						memcpy(bytes.data(), &sai->sin6_addr, bytes.size());
						boost::asio::ip::address_v6 address(bytes);
						const unsigned int prefix_len = unicast_address->OnLinkPrefixLength;

						result.push_back(ipv6_network_address{ address, prefix_len });
					}
				}
			}
		}

		return result;
	}

	void windows_tap_adapter::configure(const configuration_type& configuration)
	{
		if (::FlushIpNetTable(m_interface_index) != NO_ERROR)
		{
			// Not able to flush the ARP table.
			//
			// This is non fatal.
		}

		if (layer() == tap_adapter_layer::ip)
		{
			if (!configuration.ipv4.network_address)
			{
				throw boost::system::system_error(make_error_code(asiotap_error::invalid_ip_configuration));
			}

			DWORD len = 0;

			const unsigned int plen = configuration.ipv4.network_address->prefix_length();

			const boost::asio::ip::address_v4::bytes_type addr = configuration.ipv4.network_address->address().to_bytes();
			const uint32_t netmask = htonl(plen > 0 ? 0 - (1 << (32 - plen)) : 0xFFFFFFFF);

			if (configuration.ipv4.remote_address)
			{
				const uint32_t network = htonl(configuration.ipv4.remote_address->to_ulong()) & netmask;

				uint8_t param[3 * sizeof(uint32_t)];

				// address
				std::memcpy(param, addr.data(), addr.size());
				// network
				std::memcpy(param + sizeof(uint32_t), &network, sizeof(uint32_t));
				// netmask
				std::memcpy(param + 2 * sizeof(uint32_t), &netmask, sizeof(uint32_t));

				if (!::DeviceIoControl(descriptor().native_handle(), TAP_WIN_IOCTL_CONFIG_TUN, param, sizeof(param), param, sizeof(param), &len, NULL))
				{
					throw boost::system::system_error(::GetLastError(), boost::system::system_category());
				}
			}
			else
			{
				uint8_t param[3 * sizeof(uint32_t)];

				// address
				std::memcpy(param, addr.data(), addr.size());
				// netmask
				std::memcpy(param + 2 * sizeof(uint32_t), &netmask, sizeof(uint32_t));

				if (!::DeviceIoControl(descriptor().native_handle(), TAP_WIN_IOCTL_CONFIG_POINT_TO_POINT, param, sizeof(param), param, sizeof(param), &len, NULL))
				{
					throw boost::system::system_error(::GetLastError(), boost::system::system_category());
				}
			}
		}
		else
		{
			if (configuration.ipv4.remote_address)
			{
				throw boost::system::system_error(make_error_code(asiotap_error::invalid_ip_configuration));
			}
		}

		if (configuration.ipv4.network_address)
		{
			try
			{
				// Depending on the TAP adapter version this may not be supported.
				m_route_manager.netsh_interface_ip_set_address(display_name(), *configuration.ipv4.network_address);
			}
			catch (const boost::system::system_error& ex)
			{
				if (ex.code() != executeplus::executeplus_error::external_process_failed)
				{
					throw;
				}
			}
		}

		if (configuration.ipv4.dhcp)
		{
			try
			{
				m_route_manager.netsh_interface_ip_set_dhcp(display_name());
			}
			catch (const boost::system::system_error& ex)
			{
				if (ex.code() != executeplus::executeplus_error::external_process_failed)
				{
					throw;
				}
			}
		}

		if (configuration.ipv6.network_address)
		{
			try
			{
				// Depending on the TAP adapter version this may not be supported.
				m_route_manager.netsh_interface_ip_set_address(display_name(), *configuration.ipv6.network_address);
			}
			catch (const boost::system::system_error& ex)
			{
				if (ex.code() != executeplus::executeplus_error::external_process_failed)
				{
					throw;
				}
			}
		}
	}

	void windows_tap_adapter::set_metric(unsigned int metric)
	{
		MIB_IPINTERFACE_ROW row{};

		::InitializeIpInterfaceEntry(&row);
		row.InterfaceLuid = m_interface_luid;
		row.Family = AF_INET;

		auto error = ::GetIpInterfaceEntry(&row);

		if (error != NO_ERROR)
		{
			throw boost::system::system_error(error, boost::system::system_category());
		}

		row.Metric = static_cast<ULONG>(metric);
		row.UseAutomaticMetric = FALSE;

		// This is needed before a call to SetIpInterfaceEntry with AF_INET as the family.
		row.SitePrefixLength = 0;

		error = ::SetIpInterfaceEntry(&row);

		if (error != NO_ERROR)
		{
			throw boost::system::system_error(error, boost::system::system_category());
		}
	}

}

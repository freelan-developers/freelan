/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

#include "tap_adapter.hpp"

#include "../../log.hpp"

#include "../../platform.hpp"
#include "../../windows/registry.hpp"

#include <winioctl.h>
#include "extra/tap-windows.h"

namespace freelan {

using namespace boost::asio;

namespace {
	std::vector<std::string> enumerate_tap_adapters_guid() {
		LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters_guid", "start");

		std::vector<std::string> tap_adapters_list;

		const std::string tap_component_id(TAP_ID);
		const RegistryKey adapter_key(HKEY_LOCAL_MACHINE, ADAPTER_KEY);

		for (RegistryKey network_adapter_key : adapter_key.available_keys()) {
			if (network_adapter_key.is_open()) {
				try {
					const std::string component_id_str = network_adapter_key.query_string("ComponentId");

					if (tap_component_id == component_id_str) {
						const std::string net_cfg_instance_id_str = network_adapter_key.query_string("NetCfgInstanceId");

						tap_adapters_list.push_back(net_cfg_instance_id_str);

						LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters_guid", "value") \
							.attach("guid", net_cfg_instance_id_str);
					}
				} catch (const boost::system::system_error&) {
				}
			}
		}

		LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters_guid", "stop");

		return tap_adapters_list;
	}

	std::map<std::string, std::string> enumerate_network_connections() {
		LOG(LogLevel::TRACE, "tap_adapter::enumerate_network_connections", "start");

		std::map<std::string, std::string> network_connections_map;

		const RegistryKey network_connections_key(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY);

		for (RegistryKey network_connection_key : network_connections_key.available_keys()) {
			if (network_connection_key.is_open()) {
				try {
					const RegistryKey connection_key(network_connection_key, "Connection");
					const std::string name = connection_key.query_string("Name");

					network_connections_map[network_connection_key.name()] = name;

					LOG(LogLevel::TRACE, "tap_adapter::enumerate_network_connections", "value") \
						.attach("guid", network_connection_key.name()) \
						.attach("display_name", name);
				} catch (const boost::system::system_error&) {
				}
			}
		}

		LOG(LogLevel::TRACE, "tap_adapter::enumerate_network_connections", "stop");

		return network_connections_map;
	}

	std::map<std::string, std::string> enumerate_tap_adapters() {
		LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters", "start");

		std::map<std::string, std::string> tap_adapters_map;

		try {
			const auto network_connections_map = enumerate_network_connections();
			const auto tap_adapters_list = enumerate_tap_adapters_guid();

			for (const std::string& guid : tap_adapters_list) {
				const auto it = network_connections_map.find(guid);

				if (it != network_connections_map.end()) {
					tap_adapters_map[guid] = it->second;

					LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters", "value") \
						.attach("guid", guid) \
						.attach("display_name", it->second);
				}
			}
		} catch (const boost::system::system_error&) {
		}

		LOG(LogLevel::TRACE, "tap_adapter::enumerate_tap_adapters", "stop");

		return tap_adapters_map;
	}

	boost::system::error_code find_tap_adapter_by_guid(std::string& guid, std::string& display_name, boost::system::error_code& ec) {
		const auto tap_adapters_map = enumerate_tap_adapters();
		const auto it = guid.empty() ? tap_adapters_map.begin() : tap_adapters_map.find(guid);

		if (it == tap_adapters_map.end()) {
			LOG(LogLevel::ERROR, "tap_adapter::find_tap_adapter_by_guid", "no_tap_adapter_in_registry") \
				.attach("guid", guid);

			return (ec = boost::system::error_code(ERROR_FILE_NOT_FOUND, boost::system::system_category()));
		}

		guid = it->first;
		display_name = it->second;

		return ec;
	}
}

boost::system::error_code TapAdapter::open(std::string _name, boost::system::error_code& ec) {
	ec = boost::system::error_code();

    LOG(LogLevel::DEBUG, "tap_adapter::open", "start") \
        .attach("name", _name);

	PIP_ADAPTER_INFO piai = NULL;
	ULONG size = 0;
	DWORD status;

	status = GetAdaptersInfo(piai, &size);

	if (status != ERROR_BUFFER_OVERFLOW) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "get_adapters_info_buffer_to_small") \
			.attach("name", _name);

		return (ec = boost::system::error_code(status, boost::system::system_category()));
	}

	assert(size > 0);

	std::vector<unsigned char> piai_data(size);
	piai = reinterpret_cast<PIP_ADAPTER_INFO>(&piai_data[0]);

	status = GetAdaptersInfo(piai, &size);

	if (status != ERROR_SUCCESS) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "get_adapters_info_failed") \
			.attach("name", _name);

		return (ec = boost::system::error_code(status, boost::system::system_category()));
	}

	piai_data.resize(size);

	std::string display_name;

	if (find_tap_adapter_by_guid(_name, display_name, ec)) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "no_tap_adapter_in_registry") \
			.attach("name", _name);

		return ec;
	}

	PIP_ADAPTER_INFO pi = piai;

	for (; pi; pi = pi->Next) {
		if (_name == std::string(pi->AdapterName)) {
			break;
		}
	}

	if (!pi) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "no_tap_adapter") \
			.attach("name", _name);

		return (ec = make_error_code(boost::system::errc::no_such_device));
	}

	const HANDLE handle = CreateFileA(
		(USERMODEDEVICEDIR + _name + TAP_WIN_SUFFIX).c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
		0
	);

	if (handle == INVALID_HANDLE_VALUE) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "create_file_failed") \
			.attach("name", _name);

		return (ec = boost::system::error_code(::GetLastError(), boost::system::system_category()));
	}

	if (descriptor().assign(handle, ec)) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "descriptor_assignation_failed") \
			.attach("name", _name);

		return ec;
	}

	set_name(_name);
	set_display_name(display_name);
	m_interface_index = pi->Index;

	if (::ConvertInterfaceIndexToLuid(m_interface_index, &m_interface_luid) != NO_ERROR) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "convert_interface_to_luid_failed") \
			.attach("name", _name);

		return (ec = boost::system::error_code(::GetLastError(), boost::system::system_category()));
	}

	if (pi->AddressLength != ethernet_address().to_raw_value().size()) {
		boost::system::error_code ec2;
		close(ec2);

		LOG(LogLevel::ERROR, "tap_adapter::open", "get_interface_hardware_address_failed") \
			.attach("name", _name);

		return (ec = make_error_code(boost::system::errc::bad_address));
	}

	const auto ethernet_address = EthernetAddress::from_bytes(pi->Address, pi->AddressLength);
	set_ethernet_address(ethernet_address);

	DWORD read_mtu;
	DWORD len;

	if (!DeviceIoControl(descriptor().native_handle(), TAP_WIN_IOCTL_GET_MTU, &read_mtu, sizeof(read_mtu), &read_mtu, sizeof(read_mtu), &len, NULL)) {
		LOG(LogLevel::ERROR, "tap_adapter::open", "get_interface_mtu_failed") \
			.attach("name", _name);

		return (ec = boost::system::error_code(::GetLastError(), boost::system::system_category()));
	}

	set_mtu(static_cast<size_t>(read_mtu));

	return ec;
}

}

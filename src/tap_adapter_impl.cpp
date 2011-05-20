/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
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
 * \file tap_adapter_impl.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The tap adapter implementation class.
 */

#include "tap_adapter_impl.hpp"

#include <boost/foreach.hpp>

#include <vector>
#include <map>
#include <stdexcept>

namespace asiotap
{
	namespace
	{
#ifdef WINDOWS
		const std::string ADAPTER_KEY = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
		const std::string NETWORK_CONNECTIONS_KEY = "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
		const std::string COMPONENT_ID = "tap0901";

		void throw_system_error_if_not(LONG error)
		{
			if (error != ERROR_SUCCESS)
			{
				LPSTR msgbuf = NULL;

				FormatMessageA(
						FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						error,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPSTR)&msgbuf,
						0,
						NULL
						);

				try
				{
					throw std::runtime_error(msgbuf);
				}
				catch (...)
				{
					LocalFree(msgbuf);

					throw;
				}
			}
		}

		std::vector<std::string> enumerate_tap_adapters_guid()
		{
			std::vector<std::string> tap_adapters_list;

			HKEY adapter_key;
			LONG status;

			status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, ADAPTER_KEY.c_str(), 0, KEY_READ, &adapter_key);

			throw_system_error_if_not(status);

			try
			{
				DWORD index = 0;
				char name[256];
				DWORD name_len = sizeof(name);

				do
				{
					status = RegEnumKeyExA(adapter_key, index, name, &name_len, NULL, NULL, NULL, NULL);
					++index;

					if (status != ERROR_NO_MORE_ITEMS)
					{
						throw_system_error_if_not(status);
					}

					const std::string network_adapter_key_name = ADAPTER_KEY + "\\" + std::string(name, name_len);
					HKEY network_adapter_key;

					status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, network_adapter_key_name.c_str(), 0, KEY_READ, &network_adapter_key);

					throw_system_error_if_not(status);

					try
					{
						DWORD type;
						char component_id[256];
						DWORD component_id_len = sizeof(component_id);

						status = RegQueryValueExA(network_adapter_key, "ComponentId", NULL, &type, reinterpret_cast<LPBYTE>(component_id), &component_id_len);

						throw_system_error_if_not(status);

						if (type == REG_SZ)
						{
							if (COMPONENT_ID == std::string(component_id, component_id_len))
							{
								char net_cfg_instance_id[256];
								DWORD net_cfg_instance_id_len = sizeof(net_cfg_instance_id);

								status = RegQueryValueExA(network_adapter_key, "NetCfgInstanceId", NULL, &type, reinterpret_cast<LPBYTE>(net_cfg_instance_id), &net_cfg_instance_id_len);

								throw_system_error_if_not(status);

								if (type == REG_SZ)
								{
									tap_adapters_list.push_back(std::string(net_cfg_instance_id, net_cfg_instance_id_len));
								}
							}
						}
					}
					catch (...)
					{
						RegCloseKey(network_adapter_key);

						throw;
					}

					RegCloseKey(network_adapter_key);

				} while (status != ERROR_NO_MORE_ITEMS);
			}
			catch (...)
			{
				RegCloseKey(adapter_key);

				throw;
			}

			RegCloseKey(adapter_key);

			return tap_adapters_list;
		}

		std::map<std::string, std::string> enumerate_network_connections()
		{
			std::map<std::string, std::string> network_connections_map;

			HKEY network_connections_key;
			LONG status;

			status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY.c_str(), 0, KEY_READ, &network_connections_key);

			throw_system_error_if_not(status);

			try
			{
				DWORD index = 0;
				char name[256];
				DWORD name_len = sizeof(name);

				do
				{
					status = RegEnumKeyExA(network_connections_key, index, name, &name_len, NULL, NULL, NULL, NULL);
					++index;

					if (status != ERROR_NO_MORE_ITEMS)
					{
						throw_system_error_if_not(status);
					}

					const std::string connection_key_name = NETWORK_CONNECTIONS_KEY + "\\" + std::string(name, name_len) + "\\Connection";
					HKEY connection_key;

					status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, connection_key_name.c_str(), 0, KEY_READ, &connection_key);

					throw_system_error_if_not(status);

					try
					{
						DWORD type;
						char cname[256];
						DWORD cname_len = sizeof(cname);

						status = RegQueryValueExA(connection_key, "Name", NULL, &type, reinterpret_cast<LPBYTE>(cname), &cname_len);

						throw_system_error_if_not(status);

						if (type == REG_SZ)
						{
							network_connections_map[std::string(name, name_len)] = std::string(cname, cname_len);
						}
					}
					catch (...)
					{
						RegCloseKey(connection_key);

						throw;
					}

					RegCloseKey(connection_key);

				} while (status != ERROR_NO_MORE_ITEMS);
			}
			catch (...)
			{
				RegCloseKey(network_connections_key);

				throw;
			}

			RegCloseKey(network_connections_key);

			return network_connections_map;
		}

		std::map<std::string, std::string> enumerate_tap_adapters()
		{
			std::map<std::string, std::string> network_connections_map = enumerate_network_connections();

			std::vector<std::string> tap_adapters_list = enumerate_tap_adapters_guid();

			std::map<std::string, std::string> tap_adapters_map;

			BOOST_FOREACH(const std::string& guid, tap_adapters_list)
			{
				if (network_connections_map.find(guid) != network_connections_map.end())
				{
					tap_adapters_map[guid] = network_connections_map[guid];
				}
			}

			return tap_adapters_map;
		}
#endif
	}

	tap_adapter_impl::tap_adapter_impl() :
#ifdef WINDOWS
		m_handle(INVALID_HANDLE_VALUE)
#else
#endif
	{
	}
	
	void tap_adapter_impl::open(const std::string& name)
	{
		close();

#ifdef WINDOWS

		if (name.empty())
		{

		} else
		{
		}
#else
#endif
	}

	void tap_adapter_impl::close()
	{
#ifdef WINDOWS
		if (m_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
#else
#endif
	}
}

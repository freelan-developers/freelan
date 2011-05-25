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
#include <boost/system/system_error.hpp>

#include <vector>
#include <map>
#include <stdexcept>
#include <cassert>
#include <cstring>

#ifdef WINDOWS
#include <winioctl.h>
#include <iphlpapi.h>
#include <winbase.h>
#include "../windows/common.h"
#endif

namespace asiotap
{
	namespace
	{
#ifdef WINDOWS
		typedef std::vector<std::string> guid_array_type;
		typedef std::map<std::string, std::string> guid_map_type;
		typedef std::pair<std::string, std::string> guid_pair_type;

		void throw_system_error(LONG error)
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
				throw boost::system::system_error(error, boost::system::system_category(), msgbuf);
			}
			catch (...)
			{
				LocalFree(msgbuf);

				throw;
			}
		}

		void throw_system_error_if_not(LONG error)
		{
			if (error != ERROR_SUCCESS)
			{
				throw_system_error(error);
			}
		}

		void throw_last_system_error()
		{
			throw_system_error_if_not(::GetLastError());
		}

		guid_array_type enumerate_tap_adapters_guid()
		{
			guid_array_type tap_adapters_list;

			HKEY adapter_key;
			LONG status;

			status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, ADAPTER_KEY, 0, KEY_READ, &adapter_key);

			throw_system_error_if_not(status);

			try
			{
				DWORD index = 0;
				char name[256];
				DWORD name_len;

				do
				{
					name_len = sizeof(name);
					status = RegEnumKeyExA(adapter_key, index, name, &name_len, NULL, NULL, NULL, NULL);
					++index;

					if (status != ERROR_NO_MORE_ITEMS)
					{
						throw_system_error_if_not(status);

						const std::string network_adapter_key_name = std::string(ADAPTER_KEY) + "\\" + std::string(name, name_len);
						HKEY network_adapter_key;

						status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, network_adapter_key_name.c_str(), 0, KEY_READ, &network_adapter_key);

						if (status == ERROR_SUCCESS)
						{
							try
							{
								DWORD type;
								char component_id[256];
								DWORD component_id_len = sizeof(component_id);

								status = RegQueryValueExA(network_adapter_key, "ComponentId", NULL, &type, reinterpret_cast<LPBYTE>(component_id), &component_id_len);

								if (status == ERROR_SUCCESS)
								{
									if (type == REG_SZ)
									{
										const std::string tap_component_id(TAP_COMPONENT_ID);
										const std::string component_id_str(component_id, component_id_len - 1);

										if (tap_component_id == component_id_str)
										{
											char net_cfg_instance_id[256];
											DWORD net_cfg_instance_id_len = sizeof(net_cfg_instance_id);

											status = RegQueryValueExA(network_adapter_key, "NetCfgInstanceId", NULL, &type, reinterpret_cast<LPBYTE>(net_cfg_instance_id), &net_cfg_instance_id_len);

											if (status == ERROR_SUCCESS)
											{
												if (type == REG_SZ)
												{
													const std::string net_cfg_instance_id_str(net_cfg_instance_id, net_cfg_instance_id_len - 1);
													tap_adapters_list.push_back(net_cfg_instance_id_str);
												}
											}
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
						}
					}
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

		guid_map_type enumerate_network_connections()
		{
			guid_map_type network_connections_map;

			HKEY network_connections_key;
			LONG status;

			status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY, 0, KEY_READ, &network_connections_key);

			throw_system_error_if_not(status);

			try
			{
				DWORD index = 0;
				char name[256];
				DWORD name_len;

				do
				{
					name_len = sizeof(name);
					status = RegEnumKeyExA(network_connections_key, index, name, &name_len, NULL, NULL, NULL, NULL);
					++index;

					const std::string name_str(name, name_len);

					if (status != ERROR_NO_MORE_ITEMS)
					{
						throw_system_error_if_not(status);

						const std::string connection_key_name = std::string(NETWORK_CONNECTIONS_KEY) + "\\" + std::string(name, name_len) + "\\Connection";
						HKEY connection_key;

						status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, connection_key_name.c_str(), 0, KEY_READ, &connection_key);

						if (status == ERROR_SUCCESS)
						{
							try
							{
								DWORD type;
								char cname[256];
								DWORD cname_len = sizeof(cname);

								status = RegQueryValueExA(connection_key, "Name", NULL, &type, reinterpret_cast<LPBYTE>(cname), &cname_len);

								const std::string cname_str(cname, cname_len - 1);

								if (status == ERROR_SUCCESS)
								{
									if (type == REG_SZ)
									{
										network_connections_map[name_str] = cname_str;
									}
								}
							}
							catch (...)
							{
								RegCloseKey(connection_key);

								throw;
							}

							RegCloseKey(connection_key);
						}
					}
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

		guid_map_type enumerate_tap_adapters()
		{
			guid_map_type network_connections_map = enumerate_network_connections();

			guid_array_type tap_adapters_list = enumerate_tap_adapters_guid();

			guid_map_type tap_adapters_map;

			BOOST_FOREACH(const std::string& guid, tap_adapters_list)
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
			guid_map_type tap_adapters_map = enumerate_tap_adapters();

			guid_map_type::const_iterator it = tap_adapters_map.find(guid);

			if (it == tap_adapters_map.end())
			{
				throw std::runtime_error("No such tap adapter: " + guid);
			}

			return *it;
		}

		void cancel_io_ex(HANDLE handle, LPOVERLAPPED poverlapped)
		{
			FARPROC cancel_io_ex_ptr = ::GetProcAddress(::GetModuleHandleA("KERNEL32"), "CancelIoEx");

			if (!cancel_io_ex_ptr)
			{
				throw boost::system::system_error(boost::asio::error::operation_not_supported, boost::system::system_category());
			}

			typedef BOOL (WINAPI* cancel_io_ex_t)(HANDLE, LPOVERLAPPED);
			cancel_io_ex_t cancel_io_ex = reinterpret_cast<cancel_io_ex_t>(cancel_io_ex_ptr);

			if (!cancel_io_ex(handle, poverlapped))
			{
				DWORD last_error = ::GetLastError();

				if (last_error != ERROR_NOT_FOUND)
				{
					throw_system_error(last_error);
				}
			}
		}
#endif
	}

	tap_adapter_impl::tap_adapter_impl() :
		m_mtu(0),
#ifdef WINDOWS
		m_handle(INVALID_HANDLE_VALUE),
		m_interface_index(0)
#else
#endif
	{
	}
	
	bool tap_adapter_impl::is_open() const
	{
#ifdef WINDOWS
		return m_handle != INVALID_HANDLE_VALUE;
#else
#endif
	}

	void tap_adapter_impl::open(const std::string& _name)
	{
		close();

#ifdef WINDOWS

		if (_name.empty())
		{
			guid_map_type tap_adapters_map = enumerate_tap_adapters();

			for (guid_map_type::const_iterator tap_adapter = tap_adapters_map.begin(); !is_open() && tap_adapter != tap_adapters_map.end(); ++tap_adapter)
			{
				try
				{
					open(tap_adapter->first);
				}
				catch (const std::exception&)
				{
					// This is not as ugly as it seems :)
				}
			}

			if (!is_open())
			{
				throw std::runtime_error("No suitable tap adapter found.");
			}
		} else
		{
			PIP_ADAPTER_INFO piai = NULL;
			ULONG size = 0;
			DWORD status;

			status = GetAdaptersInfo(piai, &size);

			if (status != ERROR_BUFFER_OVERFLOW)
			{
				throw_system_error_if_not(status);
			}

			std::vector<unsigned char> piai_data(size);
			piai = reinterpret_cast<PIP_ADAPTER_INFO>(&piai_data[0]);

			status = GetAdaptersInfo(piai, &size);

			throw_system_error_if_not(status);

			piai_data.resize(size);

			guid_pair_type adapter = find_tap_adapter_by_guid(_name);

			for (PIP_ADAPTER_INFO pi = piai; pi; pi = pi->Next)
			{
				if (adapter.first == std::string(pi->AdapterName))
				{
					m_handle = CreateFileA(
							(USERMODEDEVICEDIR + adapter.first + TAPSUFFIX).c_str(),
							GENERIC_READ | GENERIC_WRITE,
							0,
							0,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
							0
							);

					if (m_handle == INVALID_HANDLE_VALUE)
					{
						throw_last_system_error();
					}

					m_name = adapter.first;
					m_display_name = adapter.second;
					m_interface_index = pi->Index;

					if (pi->AddressLength != m_ethernet_address.size())
					{
						close();

						throw std::runtime_error("Unexpected Ethernet address size");
					}

					std::memcpy(m_ethernet_address.c_array(), pi->Address, pi->AddressLength);

					DWORD len;

					if (!DeviceIoControl(m_handle, TAP_IOCTL_GET_MTU, &m_mtu, sizeof(m_mtu), &m_mtu, sizeof(m_mtu), &len, NULL))
					{
						close();

						throw_last_system_error();
					}

					memset(&m_read_overlapped, 0, sizeof(m_read_overlapped));
					m_read_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					memset(&m_write_overlapped, 0, sizeof(m_write_overlapped));
					m_write_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

					break;
				}
			}

			if (!is_open())
			{
				throw std::runtime_error("Unable to open the specified tap adapter: " + _name);
			}
		}
#else
#endif
	}

	void tap_adapter_impl::close()
	{
#ifdef WINDOWS
		if (is_open())
		{
			cancel();
			CloseHandle(m_write_overlapped.hEvent);
			CloseHandle(m_read_overlapped.hEvent);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
#else
#endif
	}

	void tap_adapter_impl::set_connected_state(bool connected)
	{
		assert(is_open());

#ifdef WINDOWS
		ULONG status = connected ? TRUE : FALSE;
		DWORD len;

		if (!DeviceIoControl(m_handle, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), NULL, 0, &len, NULL))
		{
			throw_last_system_error();
		}

#else
		connected = connected; // Avoid unused parameters warnings
#endif
	}
	
	void tap_adapter_impl::begin_read(void* buf, size_t buf_len)
	{
		assert(buf);

#ifdef WINDOWS
		assert(m_handle != INVALID_HANDLE_VALUE);

		bool success = (ReadFile(m_handle, buf, static_cast<DWORD>(buf_len), NULL, &m_read_overlapped) != 0);

		if (!success)
		{
			DWORD last_error = ::GetLastError();

			if (last_error != ERROR_IO_PENDING)
			{
				throw_system_error_if_not(last_error);
			}
		}
#else
#endif
	}
	
	bool tap_adapter_impl::end_read(size_t& _cnt, const boost::posix_time::time_duration& timeout)
	{
#ifdef WINDOWS
		assert(m_handle != INVALID_HANDLE_VALUE);

		DWORD _timeout = timeout.is_special() ? INFINITE : timeout.total_milliseconds();

		if (WaitForSingleObject(m_read_overlapped.hEvent, _timeout) == WAIT_OBJECT_0)
		{
			DWORD cnt = 0;

			if (GetOverlappedResult(m_handle, &m_read_overlapped, &cnt, true))
			{
				_cnt = cnt;
				return true;
			}
			else
			{
				throw_last_system_error();
			}
		}

		return false;
#else
#endif
	}
	
	void tap_adapter_impl::begin_write(const void* buf, size_t buf_len)
	{
		assert(buf);

#ifdef WINDOWS
		assert(m_handle != INVALID_HANDLE_VALUE);

		bool success = (WriteFile(m_handle, buf, static_cast<DWORD>(buf_len), NULL, &m_write_overlapped) != 0);

		if (!success)
		{
			DWORD last_error = ::GetLastError();

			if (last_error != ERROR_IO_PENDING)
			{
				throw_system_error_if_not(last_error);
			}
		}
#else
#endif
	}
	
	bool tap_adapter_impl::end_write(size_t& _cnt, const boost::posix_time::time_duration& timeout)
	{
#ifdef WINDOWS
		assert(m_handle != INVALID_HANDLE_VALUE);

		DWORD _timeout = timeout.is_special() ? INFINITE : timeout.total_milliseconds();

		if (WaitForSingleObject(m_write_overlapped.hEvent, _timeout) == WAIT_OBJECT_0)
		{
			DWORD cnt = 0;

			if (GetOverlappedResult(m_handle, &m_write_overlapped, &cnt, true))
			{
				_cnt = cnt;
				return true;
			}
			else
			{
				throw_last_system_error();
			}
		}

		return false;
#else
#endif
	}
	
	void tap_adapter_impl::cancel_read()
	{
#ifdef WINDOWS
		if (is_open())
		{
			cancel_io_ex(m_handle, &m_read_overlapped);
		}
#else
#endif
	}

	void tap_adapter_impl::cancel_write()
	{
#ifdef WINDOWS
		if (is_open())
		{
			cancel_io_ex(m_handle, &m_write_overlapped);
		}
#else
#endif
	}
	
	size_t tap_adapter_impl::read(void* buf, size_t buf_len)
	{
		assert(buf);

#ifdef WINDOWS
		begin_read(buf, buf_len);

		size_t cnt;

		while (!end_read(cnt));

		return cnt;
#else
#endif
	}
	
	size_t tap_adapter_impl::write(const void* buf, size_t buf_len)
	{
		assert(buf);

#ifdef WINDOWS
		begin_write(buf, buf_len);

		size_t cnt;

		while (!end_write(cnt));

		return cnt;
#else
#endif
	}
}

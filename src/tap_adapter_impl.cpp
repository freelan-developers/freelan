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
 * \file tap_adapter_impl.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \author Sebastien Vincent <sebastien.vincent@freelan.org>
 * \brief The tap adapter implementation class.
 */

#include "tap_adapter_impl.hpp"

#include <boost/foreach.hpp>
#include <boost/system/system_error.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <stdexcept>
#include <cassert>
#include <cstring>

#ifdef WINDOWS
#include <winioctl.h>
#include <iphlpapi.h>
#include <winbase.h>
#include <shellapi.h>
#include "../windows/common.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <ifaddrs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef LINUX
#include <linux/if_tun.h>
/**
 * \struct in6_ifreq
 * \brief Replacement structure since the include of linux/ipv6.h introduces conflicts.
 *
 * If someone comes up with a better solution, feel free to contribute :)
 */
struct in6_ifreq
{
	struct in6_addr ifr6_addr; /**< IPv6 address */
	uint32_t ifr6_prefixlen; /**< Length of the prefix */
	int ifr6_ifindex; /**< Interface index */
};
#elif defined(MACINTOSH) || defined(BSD)
/* Note for Mac OS X users : you have to download and install the tun/tap driver from
 * http://tuntaposx.sourceforge.net/
 */
#include <net/if_var.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#endif
#endif

namespace asiotap
{
	namespace
	{
		unsigned int netmask_to_prefix_len(in_addr netmask)
		{
			uint32_t bits = ~ntohl(netmask.s_addr);

			unsigned int result = 0;

			while (bits > 0) {
				bits >>= 1;
				++result;
			}

			return (sizeof(in_addr)* 8) - result;
		}

		unsigned int netmask_to_prefix_len(in6_addr netmask)
		{
			unsigned int result = 0;

			for (size_t i = 0; i < sizeof(netmask.s6_addr); ++i)
			{
				uint8_t bits = ~netmask.s6_addr[i];

				while (bits > 0) {
					bits >>= 1;
					++result;
				}
			}

			return (sizeof(in6_addr)* 8) - result;
		}

#ifdef WINDOWS
		typedef std::vector<std::string> guid_array_type;
		typedef std::map<std::string, std::string> guid_map_type;
		typedef std::pair<std::string, std::string> guid_pair_type;

		void throw_system_error(LONG error)
		{
			throw boost::system::system_error(error, boost::system::system_category());
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
				}
				while (status != ERROR_NO_MORE_ITEMS);
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
				}
				while (status != ERROR_NO_MORE_ITEMS);
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

		DWORD shell_execute(const std::string& cmd, const std::string& params)
		{
			SHELLEXECUTEINFO sei;
			memset(&sei, 0x00, sizeof(sei));
			sei.cbSize = sizeof(sei);
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpVerb = NULL;
			sei.lpFile = cmd.c_str();
			sei.lpParameters = params.c_str();
			sei.nShow = SW_HIDE;

			if (ShellExecuteEx(&sei) != TRUE)
			{
				throw_last_system_error();
			}

			if (!sei.hProcess)
			{
				throw std::runtime_error("A process handle was expected");
			}

			if (WaitForSingleObject(sei.hProcess, INFINITE) != WAIT_OBJECT_0)
			{
				throw_last_system_error();
			}

			DWORD exit_code = 0;

			if (GetExitCodeProcess(sei.hProcess, &exit_code) == 0)
			{
				throw_last_system_error();
			}

			return exit_code;
		}

		DWORD netsh_execute(const std::string& params)
		{
			return shell_execute("netsh.exe", params);
		}

		DWORD netsh_add_address(const std::string& address_family, DWORD interface_index, const std::string& address, unsigned int prefix_len)
		{
			std::ostringstream oss;

			oss << "int " << address_family << " add address " << interface_index << " " << address;

			OSVERSIONINFO os_version;
			os_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&os_version);

			// The /prefix parameter is only supported after Windows XP
			if (os_version.dwMajorVersion >= 6)
			{
				oss << "/" << prefix_len;
			}

			oss << " store=active";

			return netsh_execute(oss.str());
		}

		DWORD netsh_remove_address(const std::string& address_family, DWORD interface_index, const std::string& address)
		{
			std::ostringstream oss;

			oss << "int " << address_family << " delete address \"" << interface_index << "\" " << address;

			return netsh_execute(oss.str());
		}
#else
#ifndef AIO_RESOLUTION
#define AIO_RESOLUTION 500
#endif

		const boost::posix_time::time_duration AIO_RESOLUTION_DURATION = boost::posix_time::milliseconds(AIO_RESOLUTION);

		void throw_system_error(int error)
		{
			char error_str[256] = { 0 };

			if (strerror_r(error, error_str, sizeof(error_str)) != 0)
			{
				throw boost::system::system_error(error, boost::system::system_category());
			}
			else
			{
				throw boost::system::system_error(error, boost::system::system_category(), error_str);
			}
		}

		void throw_last_system_error()
		{
			throw_system_error(errno);
		}

#ifndef MACINTOSH
		timespec time_duration_to_timespec(const boost::posix_time::time_duration& duration)
		{
			timespec result = { 0, 0 };

			result.tv_sec = duration.total_seconds();
			result.tv_nsec = static_cast<long>(duration.fractional_seconds() * (1000000001LL / duration.ticks_per_second()));

			return result;
		}
#endif /* MACINTOSH */

#endif
	}

	std::map<std::string, std::string> tap_adapter_impl::enumerate()
	{
#ifdef WINDOWS
		return enumerate_tap_adapters();
#else
		std::map<std::string, std::string> result;

		struct ifaddrs* addrs = NULL;

		if (getifaddrs(&addrs) != -1)
		{
			boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

			for (struct ifaddrs* ifa = paddrs.get(); ifa != NULL ; ifa = ifa->ifa_next)
			{
				const std::string name(ifa->ifa_name);

				if ((name.substr(0, 3) == "tap") || (name.substr(0, 3) == "tun"))
				{
					result[name] = name;
				}
			}
		}

		return result;
#endif
	}

	tap_adapter_impl::tap_adapter_impl() :
		m_mtu(0),
#ifdef WINDOWS
		m_handle(INVALID_HANDLE_VALUE),
		m_interface_index(0)
#else
		m_device(-1)
#endif
	{
#ifndef WINDOWS
		std::memset(&m_read_aio, 0, sizeof(m_read_aio));
		std::memset(&m_write_aio, 0, sizeof(m_write_aio));
#endif
	}

	bool tap_adapter_impl::is_open() const
	{
#ifdef WINDOWS
		return m_handle != INVALID_HANDLE_VALUE;
#else
		return m_device >= 0;
#endif
	}

	void tap_adapter_impl::open(const std::string& _name, unsigned _mtu, tap_adapter_impl::adapter_type _type)
	{
		close();

		m_type = _type;

#ifdef WINDOWS

		/* TODO: Implement TUN logic for Windows. */

		if (_name.empty())
		{
			guid_map_type tap_adapters_map = enumerate_tap_adapters();

			for (guid_map_type::const_iterator tap_adapter = tap_adapters_map.begin(); !is_open() && tap_adapter != tap_adapters_map.end(); ++tap_adapter)
			{
				try
				{
					open(tap_adapter->first, _mtu, _type);
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
		}
		else
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

					std::memset(&m_read_overlapped, 0, sizeof(m_read_overlapped));
					m_read_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					std::memset(&m_write_overlapped, 0, sizeof(m_write_overlapped));
					m_write_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

					break;
				}
			}

			if (!is_open())
			{
				throw std::runtime_error("Unable to open the specified tap adapter: " + _name);
			}
		}
#elif defined(LINUX)
		struct ifreq ifr;
		std::memset(&ifr, 0x00, sizeof(struct ifreq));

		const std::string dev_name = (m_type == AT_TAP_ADAPTER) ? "/dev/net/tap" : "/dev/net/tun";

		ifr.ifr_flags = IFF_NO_PI;

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
		ifr.ifr_flags |= IFF_ONE_QUEUE;
#endif

		if (m_type == AT_TAP_ADAPTER)
		{
			ifr.ifr_flags |= IFF_TAP;
		}
		else
		{
			ifr.ifr_flags |= IFF_TUN;
		}

		{
			if (::access(dev_name.c_str(), F_OK) == -1)
			{
				if (errno == ENOENT)
				{
					// No tap found, create one.
					if (::mknod(dev_name.c_str(), S_IFCHR | S_IRUSR | S_IWUSR, ::makedev(10, 200)) == -1)
					{
						throw_last_system_error();
					}
				}
				else
				{
					throw_last_system_error();
				}
			}

			m_device = ::open(dev_name.c_str(), O_RDWR);

			if (m_device == -1)
			{
				throw_last_system_error();
			}


			try
			{
				if (!_name.empty())
				{
					strncpy(ifr.ifr_name, _name.c_str(), IFNAMSIZ);
				}

				// Set the parameters on the tun device
				if (::ioctl(m_device, TUNSETIFF, (void *)&ifr) < 0)
				{
					throw_last_system_error();
				}

				int ctl_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

				if (ctl_fd >= 0)
				{
					try
					{
						struct ifreq netifr;

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
						std::memset(&netifr, 0x00, sizeof(struct ifreq));
						std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);
						netifr.ifr_qlen = 100; // 100 is the default value

						if (::ioctl(ctl_fd, SIOCSIFTXQLEN, (void *)&netifr) < 0)
						{
							throw_last_system_error();
						}
#endif
						// Set the MTU
						std::memset(&netifr, 0x00, sizeof(struct ifreq));
						std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);

						if (_mtu > 0)
						{
							netifr.ifr_mtu = _mtu;

							::ioctl(ctl_fd, SIOCSIFMTU, (void*)&netifr);
						}

						if (::ioctl(ctl_fd, SIOCGIFMTU, (void*)&netifr) >= 0)
						{
							m_mtu = netifr.ifr_mtu;
						}
						else
						{
							throw_last_system_error();
						}

						std::memset(&netifr, 0x00, sizeof(struct ifreq));
						std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);

						// Get the interface hwaddr
						if (::ioctl(ctl_fd, SIOCGIFHWADDR, (void*)&netifr) < 0)
						{
							throw_last_system_error();
						}

						std::memcpy(m_ethernet_address.c_array(), netifr.ifr_hwaddr.sa_data, m_ethernet_address.size());
					}
					catch (...)
					{
						::close(ctl_fd);
						throw;
					}

					::close(ctl_fd);
				}
				else
				{
					throw_last_system_error();
				}
			}
			catch (...)
			{
				::close(m_device);
				m_device = -1;

				throw;
			}
		}

		m_name = ifr.ifr_name;

#else /* *BSD and Mac OS X */

		const std::string dev_name = (m_type == AT_TAP_ADAPTER) ? "/dev/tap" : "/dev/tun";
		std::string dev = "/dev/";

		if (!_name.empty())
		{
			dev += _name;
			m_device = ::open(dev.c_str(), O_RDWR);
		}
		else
		{
			m_device = ::open(dev_name.c_str(), O_RDWR);

			if (m_device < 0)
			{
				if (errno == ENOENT)
				{
					if (_name.empty())
					{
						for (unsigned int i = 0 ; m_device < 0; ++i)
						{
							if (m_type == AT_TAP_ADAPTER)
							{
								dev = "/dev/tap" + boost::lexical_cast<std::string>(i);
							}
							else
							{
								dev = "/dev/tun" + boost::lexical_cast<std::string>(i);
							}

							m_device = ::open(dev.c_str(), O_RDWR);

							if ((m_device < 0) && (errno == ENOENT))
							{
								// We reached the end of the available tap adapters.
								break;
							}
						}
					}
				}
				else
				{
					throw_last_system_error();
				}
			}
		}

		if (m_device < 0)
		{
			throw_last_system_error();
		}

		try
		{
			struct stat st;

			::fstat(m_device, &st);

			m_name = ::devname(st.st_rdev, S_IFCHR);

			if (if_nametoindex(m_name.c_str()) == 0)
			{
				throw std::runtime_error("if_nametoindex failed");
			}

			// Do not pass the descriptor to child
			::fcntl(m_device, F_SETFD, FD_CLOEXEC);

			int ctl_fd = socket(AF_INET, SOCK_DGRAM, 0);

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			try
			{
				struct ifreq netifr;

				// Set the MTU
				memset(&netifr, 0x00, sizeof(struct ifreq));
				strncpy(netifr.ifr_name, m_name.c_str(), IFNAMSIZ);

				if (_mtu > 0)
				{
					netifr.ifr_mtu = _mtu;

					::ioctl(ctl_fd, SIOCSIFMTU, (void*)&netifr);
				}

				if (::ioctl(ctl_fd, SIOCGIFMTU, (void*)&netifr) >= 0)
				{
					m_mtu = netifr.ifr_mtu;
				}
				else
				{
					throw_last_system_error();
				}

				memset(&netifr, 0x00, sizeof(struct ifreq));
				strncpy(netifr.ifr_name, m_name.c_str(), IFNAMSIZ);

				struct ifaddrs* addrs = NULL;

				/* find the hardware address of tap inteface */
				if (getifaddrs(&addrs) != -1)
				{
					boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

					for(struct ifaddrs* ifa = addrs; ifa != NULL; ifa = ifa->ifa_next)
					{
						if ((ifa->ifa_addr->sa_family == AF_LINK) && !std::memcmp(ifa->ifa_name, m_name.c_str(), m_name.length()))
						{
							struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;

							if (sdl->sdl_type == IFT_ETHER)
							{
								std::memcpy(m_ethernet_address.c_array(), LLADDR(sdl), 6);

								break;
							}
						}
					}
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);
		}
		catch (...)
		{
			::close(m_device);
			m_device = -1;

			throw;
		}
#endif
	}

	void tap_adapter_impl::close()
	{
		if (is_open())
		{
#ifdef WINDOWS
			cancel();
			CloseHandle(m_write_overlapped.hEvent);
			CloseHandle(m_read_overlapped.hEvent);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#else
#if defined(MACINTOSH) || defined(BSD)
			int ctl_fd = socket(AF_INET, SOCK_DGRAM, 0);

			if (ctl_fd >= 0)
			{
				struct ifreq ifr;

				memset(&ifr, 0x00, sizeof(ifr));
				strncpy(ifr.ifr_name, m_name.c_str(), IFNAMSIZ);

				// Destroy the virtual tap device
				if (ioctl(ctl_fd, SIOCIFDESTROY, &ifr) < 0)
				{
					// Oops ! The destruction failed. There is nothing much we can do.
				}

				::close(ctl_fd);
			}
#endif
			::close(m_device);
			m_device = -1;
#endif
		}
	}

	void tap_adapter_impl::set_connected_state(bool connected)
	{
		if (is_open())
		{
#ifdef WINDOWS
			ULONG status = connected ? TRUE : FALSE;
			DWORD len;

			if (!DeviceIoControl(m_handle, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), NULL, 0, &len, NULL))
			{
				throw_last_system_error();
			}

#else
			int ctl_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

			if (ctl_fd >= 0)
			{
				try
				{
					struct ifreq netifr;
					memset(&netifr, 0x00, sizeof(struct ifreq));
					strncpy(netifr.ifr_name, m_name.c_str(), IFNAMSIZ);

					// Set the interface UP
					if (::ioctl(ctl_fd, SIOCGIFFLAGS, (void*)&netifr) >= 0)
					{
						if (connected)
						{
#ifdef MACINTOSH
							netifr.ifr_flags |= IFF_UP;
#else
							netifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
#endif
						}
						else
						{
#ifdef MACINTOSH
							// Mac OS X: set_connected_state(false) seems to confuse the TAP
							// so do nothing for the moment
							::close(ctl_fd);
							return;
#else
							netifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
#endif
						}

						if (::ioctl(ctl_fd, SIOCSIFFLAGS, (void*)&netifr) < 0)
						{
							throw_last_system_error();
						}
					}
					else
					{
						throw_last_system_error();
					}
				}
				catch (...)
				{
					::close(ctl_fd);

					throw;
				}

				::close(ctl_fd);
			}
#endif
		}
	}

#if (!defined(MACINTOSH))

	void tap_adapter_impl::begin_read(void* buf, size_t buf_len)
	{
		assert(buf);

		if (is_open())
		{
#ifdef WINDOWS
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
			::memset(&m_read_aio, 0, sizeof(m_read_aio));
			m_read_aio.aio_fildes = m_device;
			m_read_aio.aio_buf = buf;
			m_read_aio.aio_nbytes = buf_len;
			m_read_aio.aio_offset = 0;

			if (::aio_read(&m_read_aio) != 0)
			{
				throw_last_system_error();
			}
#endif
		}
	}

	bool tap_adapter_impl::end_read(size_t& _cnt, const boost::posix_time::time_duration& timeout)
	{
		if (is_open())
		{
#ifdef WINDOWS
			DWORD _timeout = timeout.is_special() ? INFINITE : static_cast<DWORD>(timeout.total_milliseconds());

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
			// This is ugly, but aio_cancel somehow fails on tap interfaces and does never cancel the call so we have no choice...
			if (timeout.is_special())
			{
				while (is_open() && !end_read(_cnt, AIO_RESOLUTION_DURATION)) ;

				return is_open();
			}
			else
			{
				const timespec timeout_ts = time_duration_to_timespec(timeout);
				const aiocb* const _read_aio[] = { &m_read_aio };

				if (::aio_suspend(_read_aio, 1, &timeout_ts) == 0)
				{
					int error = ::aio_error(&m_read_aio);

					if (error == 0)
					{
						_cnt = ::aio_return(&m_read_aio);
						return true;
					}

					throw_system_error(error);
				}

				if ((errno != EAGAIN) && (errno != EINTR))
				{
					throw_last_system_error();
				}

				return false;
			}
#endif
		}
		else
		{
			return false;
		}
	}

	void tap_adapter_impl::begin_write(const void* buf, size_t buf_len)
	{
		assert(buf);

		if (is_open())
		{
#ifdef WINDOWS
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

			::memset(&m_write_aio, 0, sizeof(m_write_aio));
			m_write_aio.aio_fildes = m_device;
			m_write_aio.aio_buf = const_cast<void*>(buf); // This is safe, since no attempt to modify the memory pointed by buf will be made.
			m_write_aio.aio_nbytes = buf_len;
			m_write_aio.aio_offset = 0;

			if (::aio_write(&m_write_aio) != 0)
			{
				throw_last_system_error();
			}
#endif
		}
	}

	bool tap_adapter_impl::end_write(size_t& _cnt, const boost::posix_time::time_duration& timeout)
	{
		if (is_open())
		{
#ifdef WINDOWS
			DWORD _timeout = timeout.is_special() ? INFINITE : static_cast<DWORD>(timeout.total_milliseconds());

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
			// This is ugly, but aio_cancel somehow fails on tap interfaces and does never cancel the call so we have no choice...
			if (timeout.is_special())
			{
				while (is_open() && !end_write(_cnt, AIO_RESOLUTION_DURATION)) ;

				return is_open();
			}
			else
			{
				const timespec timeout_ts = time_duration_to_timespec(timeout);
				const aiocb* const _write_aio[] = { &m_write_aio };

				if (::aio_suspend(_write_aio, 1, &timeout_ts) == 0)
				{
					int error = ::aio_error(&m_write_aio);

					if (error == 0)
					{
						_cnt = ::aio_return(&m_write_aio);
						return true;
					}

					throw_system_error(error);
				}

				if ((errno != EAGAIN) && (errno != EINTR))
				{
					throw_last_system_error();
				}

				return false;
			}
#endif
		}
		else
		{
			return false;
		}
	}
#endif

	void tap_adapter_impl::cancel_read()
	{
#ifdef MACINTOSH
		throw std::runtime_error("Cancelling an I/O operation is not supported on OSX");
#else
		if (is_open())
		{
#ifdef WINDOWS
			cancel_io_ex(m_handle, &m_read_overlapped);
#else
			int error = ::aio_cancel(m_device, &m_read_aio);

			if (error == AIO_NOTCANCELED)
			{
				error = aio_error(&m_read_aio);

				switch (error)
				{
					case EINPROGRESS:
					case ECANCELED:
					case 0:
						break;
					default:
						throw_system_error(error);
				}
			}
#endif
		}
#endif
	}

	void tap_adapter_impl::cancel_write()
	{
#ifdef MACINTOSH
		throw std::runtime_error("Cancelling an I/O operation is not supported on OSX");
#else
		if (is_open())
		{
#ifdef WINDOWS
			cancel_io_ex(m_handle, &m_write_overlapped);
#else
			int error = ::aio_cancel(m_device, &m_write_aio);

			if (error == AIO_NOTCANCELED)
			{
				error = aio_error(&m_read_aio);

				switch (error)
				{
					case EINPROGRESS:
					case ECANCELED:
					case 0:
						break;
					default:
						throw_system_error(error);
				}
			}
#endif
		}
#endif
	}

	size_t tap_adapter_impl::read(void* buf, size_t buf_len)
	{
		assert(buf);

		if (is_open())
		{
#ifdef WINDOWS
			begin_read(buf, buf_len);

			size_t cnt;

			while (!end_read(cnt));

			return cnt;
#else
			ssize_t result = ::read(m_device, buf, buf_len);

			if (result <= 0)
			{
				throw_last_system_error();
			}

			return static_cast<size_t>(result);
#endif
		}
		else
		{
			return 0;
		}
	}

	size_t tap_adapter_impl::write(const void* buf, size_t buf_len)
	{
		assert(buf);

		if (is_open())
		{
#ifdef WINDOWS
			begin_write(buf, buf_len);

			size_t cnt;

			while (!end_write(cnt));

			return cnt;
#else
			ssize_t result = ::write(m_device, buf, buf_len);

			if (result <= 0)
			{
				throw_last_system_error();
			}

			return static_cast<size_t>(result);
#endif
		}
		else
		{
			return 0;
		}
	}

	tap_adapter_impl::ip_address_list tap_adapter_impl::get_ip_addresses() const
	{
		ip_address_list result;

#ifdef WINDOWS
		IP_ADAPTER_ADDRESSES* allAdapters = NULL;
		IP_ADAPTER_ADDRESSES* adapter = NULL;
		ULONG size = 16384;
		DWORD ret = 0;

		do
		{
			/* we should loop only if host has more than
			 * (size / sizeof(IP_ADAPTER_ADDRESSES)) interfaces
			 */
			allAdapters = (IP_ADAPTER_ADDRESSES*)malloc(size);

			if(!allAdapters)
			{
				/* out of memory */
				throw_last_system_error();
			}

			/* get the list of host addresses and try to find
			 * the index
			 */
			ret = GetAdaptersAddresses(AF_UNSPEC,
					GAA_FLAG_INCLUDE_ALL_INTERFACES | GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_DNS_SERVER |
					GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST,
					NULL, /* reserved */
					allAdapters,
					&size);

			if(ret == ERROR_BUFFER_OVERFLOW)
			{
				/* free memory as the loop will allocate again with
				 * proper size
				 */
				free(allAdapters);
			}
		}while(ret == ERROR_BUFFER_OVERFLOW);

		if(ret != ERROR_SUCCESS)
		{
			free(allAdapters);
			return result;
		}

		adapter = allAdapters;

		while(adapter)
		{
			const std::string ifname(adapter->AdapterName);

			if(ifname == name())
			{
				IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;

				while(unicast)
				{
					if(unicast->Address.lpSockaddr->sa_family == AF_INET)
					{
						struct sockaddr_in* sai = (struct sockaddr_in*)unicast->Address.lpSockaddr;
						boost::asio::ip::address_v4::bytes_type bytes;
						unsigned int prefix_len = sizeof(in_addr) * 8;

						std::memcpy(bytes.data(), &sai->sin_addr, bytes.size());
						boost::asio::ip::address_v4 address(bytes);
						prefix_len = unicast->OnLinkPrefixLength;

						ip_address item = { address, prefix_len };

						result.push_back(item);
					}
					else
					{
						struct sockaddr_in6* sai = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
						boost::asio::ip::address_v6::bytes_type bytes;
						unsigned int prefix_len = sizeof(in6_addr) * 8;

						memcpy(bytes.data(), &sai->sin6_addr, bytes.size());
						boost::asio::ip::address_v6 address(bytes);
						prefix_len = unicast->OnLinkPrefixLength;

						ip_address item = { address, prefix_len };

						result.push_back(item);
					}

					/* next address/prefix */
					unicast = unicast->Next;
				}
			}

			adapter = adapter->Next;
		}

		/* cleanup */
		free(allAdapters);
#else
		struct ifaddrs* addrs = NULL;

		if (getifaddrs(&addrs) != -1)
		{
			boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

			for (struct ifaddrs* ifa = paddrs.get(); ifa != NULL ; ifa = ifa->ifa_next)
			{
				const std::string ifname(ifa->ifa_name);

				if ((ifa->ifa_addr) && (ifname == name()))
				{
					if (ifa->ifa_addr->sa_family == AF_INET)
					{
						struct sockaddr_in* sai = (struct sockaddr_in*)ifa->ifa_addr;

						boost::asio::ip::address_v4::bytes_type bytes;
						memcpy(bytes.c_array(), &sai->sin_addr, bytes.size());

						boost::asio::ip::address_v4 address(bytes);

						unsigned int prefix_len = sizeof(in_addr) * 8;

						if (ifa->ifa_netmask)
						{
							struct sockaddr_in* sain = (struct sockaddr_in*)ifa->ifa_netmask;

							prefix_len = netmask_to_prefix_len(sain->sin_addr);
						}

						ip_address item = { address, prefix_len };

						result.push_back(item);
					}
					else if (ifa->ifa_addr->sa_family == AF_INET6)
					{
						struct sockaddr_in6* sai = (struct sockaddr_in6*)ifa->ifa_addr;

						boost::asio::ip::address_v6::bytes_type bytes;
						memcpy(bytes.c_array(), &sai->sin6_addr, bytes.size());

						boost::asio::ip::address_v6 address(bytes);

						unsigned int prefix_len = sizeof(in6_addr) * 8;

						if (ifa->ifa_netmask)
						{
							struct sockaddr_in6* sain = (struct sockaddr_in6*)ifa->ifa_netmask;

							prefix_len = netmask_to_prefix_len(sain->sin6_addr);
						}

						ip_address item = { address, prefix_len };

						result.push_back(item);
					}
				}
			}
		}

#endif

		return result;
	}

	bool tap_adapter_impl::add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len)
	{
		assert(prefix_len < 32);

		if (is_open())
		{
#ifdef WINDOWS
			// in TUN mode, we need to perform an ioctl with some IPv4 parameters
			if(m_type == AT_TUN_ADAPTER)
			{
				uint8_t param[12];
				DWORD len = 0;

				boost::asio::ip::address_v4::bytes_type addr = address.to_bytes();
				uint32_t netmask = htonl((0xFFFFFFFF >> (32 - prefix_len)) << (32 - prefix_len));
				uint32_t network = htonl(address.to_ulong()) & netmask;

				// address
				std::memcpy(param, addr.data(), addr.size());
				// network
				std::memcpy(param + 4, &network, sizeof(uint32_t));
				// netmask
				std::memcpy(param + 8, &netmask, sizeof(uint32_t));

				if (!DeviceIoControl(m_handle, TAP_IOCTL_CONFIG_TUN, param, sizeof(param), NULL, 0, &len, NULL))
				{
					throw_last_system_error();
				}
			}

			return (netsh_add_address("ipv4", m_interface_index, address.to_string(), prefix_len) == 0);
#else
			int ctl_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			bool result = true;

			try
			{
				ifreq ifr_a;

				std::memset(&ifr_a, 0x00, sizeof(ifr_a));
				std::strncpy(ifr_a.ifr_name, m_name.c_str(), IFNAMSIZ - 1);
				sockaddr_in* ifr_a_addr = reinterpret_cast<sockaddr_in*>(&ifr_a.ifr_addr);
				ifr_a_addr->sin_family = AF_INET;
#ifdef BSD
				ifr_a_addr->sin_len = sizeof(sockaddr_in);
#endif
				std::memcpy(&ifr_a_addr->sin_addr.s_addr, address.to_bytes().c_array(), address.to_bytes().size());

				if (::ioctl(ctl_fd, SIOCSIFADDR, &ifr_a) < 0)
				{
					if (errno == EEXIST)
					{
						result = false;
					}
					else
					{
						throw_last_system_error();
					}
				}

				if (prefix_len > 0)
				{
					ifreq ifr_n;

					std::memset(&ifr_n, 0x00, sizeof(ifr_n));
					std::strncpy(ifr_n.ifr_name, m_name.c_str(), IFNAMSIZ - 1);
					sockaddr_in* ifr_n_addr = reinterpret_cast<sockaddr_in*>(&ifr_n.ifr_addr);
					ifr_n_addr->sin_family = AF_INET;
#ifdef BSD
					ifr_n_addr->sin_len = sizeof(sockaddr_in);
#endif
					ifr_n_addr->sin_addr.s_addr = htonl((0xFFFFFFFF >> (32 - prefix_len)) << (32 - prefix_len));

					if (::ioctl(ctl_fd, SIOCSIFNETMASK, &ifr_n) < 0)
					{
						if (errno == EEXIST)
						{
							result = false;
						}
						else
						{
							throw_last_system_error();
						}
					}
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);

			return result;
#endif
		}
		else
		{
			return false;
		}
	}

	bool tap_adapter_impl::remove_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len)
	{
		(void)prefix_len;

		if (is_open())
		{
#ifdef WINDOWS

			return (netsh_remove_address("ipv4", m_interface_index, address.to_string()) == 0);
#elif defined(LINUX)
			(void)address;
			return add_ip_address_v4(boost::asio::ip::address_v4::any(), 0);
#elif defined(BSD) || defined(MACINTOSH)
			int ctl_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			bool result = true;

			try
			{
				unsigned int if_index = if_nametoindex(m_name.c_str());

				ifaliasreq ifr;
				std::memset(&ifr, 0x00, sizeof(ifr));

				if (if_indextoname(if_index, ifr.ifra_name) == NULL)
				{
					throw_last_system_error();
				}

				sockaddr_in* ifraddr = reinterpret_cast<sockaddr_in*>(&ifr.ifra_addr);
				std::memcpy(&ifraddr->sin_addr, address.to_bytes().c_array(), address.to_bytes().size());
				ifraddr->sin_family = AF_INET;

#ifdef BSD
				ifraddr->sin_len = sizeof(struct sockaddr_in);
#endif

				if (::ioctl(ctl_fd, SIOCDIFADDR, &ifr) < 0)
				{
					throw_last_system_error();
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);

			return result;
#endif
		}
		else
		{
			return false;
		}
	}

	bool tap_adapter_impl::add_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len)
	{
		if (is_open())
		{
#ifdef WINDOWS
			return (netsh_add_address("ipv6", m_interface_index, address.to_string(), prefix_len) == 0);

#else
			int ctl_fd = ::socket(AF_INET6, SOCK_DGRAM, 0);

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			bool result = true;

			try
			{
				unsigned int if_index = ::if_nametoindex(m_name.c_str());

				if (if_index == 0)
				{
					throw std::runtime_error("No interface found with the specified name");
				}

#ifdef LINUX
				in6_ifreq ifr;
				std::memset(&ifr, 0x00, sizeof(ifr));
				std::memcpy(&ifr.ifr6_addr, address.to_bytes().c_array(), address.to_bytes().size());
				ifr.ifr6_prefixlen = prefix_len;
				ifr.ifr6_ifindex = if_index;

				if (::ioctl(ctl_fd, SIOCSIFADDR, &ifr) < 0)
#elif defined(MACINTOSH) || defined(BSD)
				in6_aliasreq iar;
				std::memset(&iar, 0x00, sizeof(iar));
				std::memcpy(iar.ifra_name, m_name.c_str(), m_name.length());
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_family = AF_INET6;
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_family = AF_INET6;
				std::memcpy(&reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_addr, address.to_bytes().c_array(), address.to_bytes().size());
				std::memset(reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr, 0xFF, prefix_len / 8);
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr[prefix_len / 8] = (0xFF << (8 - (prefix_len % 8)));
				iar.ifra_lifetime.ia6t_pltime = 0xFFFFFFFF;
				iar.ifra_lifetime.ia6t_vltime = 0xFFFFFFFF;

#ifdef SIN6_LEN
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_len = sizeof(sockaddr_in6);
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_len = sizeof(sockaddr_in6);
#endif

				if (::ioctl(ctl_fd, SIOCAIFADDR_IN6, &iar) < 0)
#endif
				{
					if (errno == EEXIST)
					{
						result = false;
					}
					else
					{
						throw_last_system_error();
					}
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);

			return result;
#endif
		}
		else
		{
			return false;
		}
	}

	bool tap_adapter_impl::remove_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len)
	{
		if (is_open())
		{
#ifdef WINDOWS
			(void)prefix_len;

			return (netsh_remove_address("ipv6", m_interface_index, address.to_string()) == 0);
#else
			int ctl_fd = ::socket(AF_INET6, SOCK_DGRAM, 0);

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			bool result = true;

			try
			{
				unsigned int if_index = ::if_nametoindex(m_name.c_str());

				if (if_index == 0)
				{
					throw std::runtime_error("No interface found with the specified name");
				}

#ifdef LINUX
				in6_ifreq ifr;
				std::memset(&ifr, 0x00, sizeof(ifr));
				std::memcpy(&ifr.ifr6_addr, address.to_bytes().c_array(), address.to_bytes().size());
				ifr.ifr6_prefixlen = prefix_len;
				ifr.ifr6_ifindex = if_index;

				if (::ioctl(ctl_fd, SIOCDIFADDR, &ifr) < 0)
#elif defined(MACINTOSH) || defined(BSD)
				in6_aliasreq iar;
				std::memset(&iar, 0x00, sizeof(iar));
				std::memcpy(iar.ifra_name, m_name.c_str(), m_name.length());
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_family = AF_INET6;
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_family = AF_INET6;
				std::memcpy(&reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_addr, address.to_bytes().c_array(), address.to_bytes().size());
				std::memset(reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr, 0xFF, prefix_len / 8);
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr[prefix_len / 8] = (0xFF << (8 - (prefix_len % 8)));
				iar.ifra_lifetime.ia6t_pltime = 0xFFFFFFFF;
				iar.ifra_lifetime.ia6t_vltime = 0xFFFFFFFF;

#ifdef SIN6_LEN
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_len = sizeof(sockaddr_in6);
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_len = sizeof(sockaddr_in6);
#endif

				if (::ioctl(ctl_fd, SIOCDIFADDR_IN6, &iar) < 0)
#endif
				{
					if (errno == EEXIST)
					{
						result = false;
					}
					else
					{
						throw_last_system_error();
					}
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);

			return result;
#endif
		}
		else
		{
			return false;
		}
	}

	bool tap_adapter_impl::set_remote_ip_address_v4(const boost::asio::ip::address_v4& local, const boost::asio::ip::address_v4& remote)
	{
		if (m_type != AT_TUN_ADAPTER)
		{
			return false;
		}

		if (is_open())
		{
#ifdef WINDOWS
			ip_address_list addresses = get_ip_addresses();
			bool result = true;

			uint8_t param[8];
			DWORD len = 0;

			boost::asio::ip::address_v4::bytes_type v4addr = local.to_bytes();
			boost::asio::ip::address_v4::bytes_type v4network = remote.to_bytes();

			// address
			std::memcpy(param, v4addr.data(), v4addr.size());
			// network
			std::memcpy(param + 4, v4network.data(), v4network.size());

			if (!DeviceIoControl(m_handle, TAP_IOCTL_CONFIG_POINT_TO_POINT, param, sizeof(param), NULL, 0, &len, NULL))
			{
				throw_last_system_error();
			}

			return result;
#else
			int ctl_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

			(void)local;

			if (ctl_fd < 0)
			{
				throw_last_system_error();
			}

			bool result = true;

			try
			{
				ifreq ifr_d;

				std::memset(&ifr_d, 0x00, sizeof(ifr_d));
				std::strncpy(ifr_d.ifr_name, m_name.c_str(), IFNAMSIZ - 1);
				sockaddr_in* ifr_dst_addr = reinterpret_cast<sockaddr_in*>(&ifr_d.ifr_dstaddr);
				ifr_dst_addr->sin_family = AF_INET;
#ifdef BSD
				ifr_dst_addr->sin_len = sizeof(sockaddr_in);
#endif
				std::memcpy(&ifr_dst_addr->sin_addr.s_addr, remote.to_bytes().c_array(), remote.to_bytes().size());

				if (::ioctl(ctl_fd, SIOCSIFDSTADDR, &ifr_d) < 0)
				{
					if (errno == EEXIST)
					{
						result = false;
					}
					else
					{
						throw_last_system_error();
					}
				}
			}
			catch (...)
			{
				::close(ctl_fd);

				throw;
			}

			::close(ctl_fd);

			return result;
#endif
		}
		else
		{
			return false;
		}
	}
}

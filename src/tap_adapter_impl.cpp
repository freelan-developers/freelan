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

	bool tap_adapter_impl::add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len)
	{
		assert(prefix_len < 32);

		if (is_open())
		{
#ifdef WINDOWS
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
				std::memcpy(&ifr_a_addr->sin_addr.s_addr, address.to_bytes().data(), address.to_bytes().size());

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
				std::memcpy(&ifraddr->sin_addr, address.to_bytes().data(), address.to_bytes().size());
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
				std::memcpy(&ifr.ifr6_addr, address.to_bytes().data(), address.to_bytes().size());
				ifr.ifr6_prefixlen = prefix_len;
				ifr.ifr6_ifindex = if_index;

				if (::ioctl(ctl_fd, SIOCSIFADDR, &ifr) < 0)
#elif defined(MACINTOSH) || defined(BSD)
				in6_aliasreq iar;
				std::memset(&iar, 0x00, sizeof(iar));
				std::memcpy(iar.ifra_name, m_name.c_str(), m_name.length());
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_family = AF_INET6;
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_family = AF_INET6;
				std::memcpy(&reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_addr, address.to_bytes().data(), address.to_bytes().size());
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
				std::memcpy(&ifr.ifr6_addr, address.to_bytes().data(), address.to_bytes().size());
				ifr.ifr6_prefixlen = prefix_len;
				ifr.ifr6_ifindex = if_index;

				if (::ioctl(ctl_fd, SIOCDIFADDR, &ifr) < 0)
#elif defined(MACINTOSH) || defined(BSD)
				in6_aliasreq iar;
				std::memset(&iar, 0x00, sizeof(iar));
				std::memcpy(iar.ifra_name, m_name.c_str(), m_name.length());
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_family = AF_INET6;
				reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_family = AF_INET6;
				std::memcpy(&reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_addr, address.to_bytes().data(), address.to_bytes().size());
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
				std::memcpy(&ifr_dst_addr->sin_addr.s_addr, remote.to_bytes().data(), remote.to_bytes().size());

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

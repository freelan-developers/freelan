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
 * \file posix_tap_adapter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The posix tap adapter class.
 */

#include "posix/posix_tap_adapter.hpp"

#include <boost/lexical_cast.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <ifaddrs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef LINUX

#include <linux/if_tun.h>
#include <sys/sysmacros.h>

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

#elif defined(MACINTOSH) || defined(OS_BSD)

/*
 * Note for Mac OS X users : you have to download and install the tun/tap driver from (http://tuntaposx.sourceforge.net).
 */
#ifndef __NetBSD__
#include <net/if_var.h>
#endif

#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>

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

			return (sizeof(in_addr) * 8) - result;
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

			return (sizeof(in6_addr) * 8) - result;
		}

		class descriptor_handler
		{
			public:

				descriptor_handler() : m_fd(-1) {}
				explicit descriptor_handler(int fd) : m_fd(fd) {}
				descriptor_handler(const descriptor_handler&) = delete;
				descriptor_handler& operator=(const descriptor_handler&) = delete;
				descriptor_handler(descriptor_handler&& other) : m_fd(other.m_fd) { other.m_fd = -1; }
				descriptor_handler& operator=(descriptor_handler&& other) { using std::swap; swap(m_fd, other.m_fd); return *this; }
				~descriptor_handler() { if (m_fd >= 0) { ::close(m_fd); }}
				int native_handle() const { return m_fd; }
				bool valid() const { return (m_fd >= 0); }
				int release() { const int result = m_fd; m_fd = -1; return result; }

			private:

				int m_fd;
		};

		descriptor_handler open_device(const std::string& name, boost::system::error_code& ec)
		{
			const int device_fd = ::open(name.c_str(), O_RDWR);

			if (device_fd < 0)
			{
				// Unable to open the device.
				ec = boost::system::error_code(errno, boost::system::system_category());

				return descriptor_handler();
			}

			return descriptor_handler(device_fd);
		}

		descriptor_handler open_socket(int family, boost::system::error_code& ec)
		{
			const int socket_fd = ::socket(family, SOCK_DGRAM, 0);

			if (socket_fd < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return descriptor_handler();
			}

			return descriptor_handler(socket_fd);
		}

		descriptor_handler open_socket(int family)
		{
			boost::system::error_code ec;

			descriptor_handler result = open_socket(family, ec);

			if (!result.valid())
			{
				throw boost::system::system_error(ec);
			}

			return result;
		}
	}

	std::map<std::string, std::string> posix_tap_adapter::enumerate(tap_adapter_layer _layer)
	{
		std::map<std::string, std::string> result;

		struct ifaddrs* addrs = nullptr;

		if (getifaddrs(&addrs) != -1)
		{
			boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

			for (struct ifaddrs* ifa = paddrs.get(); ifa != NULL ; ifa = ifa->ifa_next)
			{
				const std::string name(ifa->ifa_name);

				switch (_layer)
				{
					case tap_adapter_layer::ethernet:
					{
						if (name.substr(0, 3) == "tap")
						{
							result[name] = name;
						}
						break;
					}
					case tap_adapter_layer::ip:
					{
						if (name.substr(0, 3) == "tun")
						{
							result[name] = name;
						}
						break;
					}
				}
			}
		}

		return result;
	}

	void posix_tap_adapter::open(boost::system::error_code& ec)
	{
		open("", ec);
	}

	void posix_tap_adapter::open(const std::string& _name, boost::system::error_code& ec)
	{
		ec = boost::system::error_code();

		m_existing_tap = !_name.empty();

#if defined(LINUX)
		const std::string dev_name = "/dev/net/tun";

		if (::access(dev_name.c_str(), F_OK) == -1)
		{
			if (errno != ENOENT)
			{
				// Unable to access the tap adapter yet it exists: this is an error.
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			// No tap found, create one.
			if (::mknod(dev_name.c_str(), S_IFCHR | S_IRUSR | S_IWUSR, ::makedev(10, 200)) == -1)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}
		}

		descriptor_handler device = open_device(dev_name, ec);

		if (!device.valid())
		{
			return;
		}

		struct ifreq ifr {};

		ifr.ifr_flags = IFF_NO_PI;

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
		ifr.ifr_flags |= IFF_ONE_QUEUE;
#endif

		if (layer() == tap_adapter_layer::ethernet)
		{
			ifr.ifr_flags |= IFF_TAP;
		}
		else
		{
			ifr.ifr_flags |= IFF_TUN;
		}

		if (!_name.empty())
		{
			strncpy(ifr.ifr_name, _name.c_str(), IFNAMSIZ);
			ifr.ifr_name[IFNAMSIZ - 1] = 0x00;
		}

		// Set the parameters on the tun device.
		if (::ioctl(device.native_handle(), TUNSETIFF, (void *)&ifr) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return;
		}

		descriptor_handler socket = open_socket(AF_INET, ec);

		if (!socket.valid())
		{
			return;
		}

		{
			struct ifreq netifr {};

			std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);
			netifr.ifr_name[IFNAMSIZ - 1] = 0x00;

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)

			netifr.ifr_qlen = 100; // 100 is the default value

			if (getuid() == 0 && ::ioctl(socket.native_handle(), SIOCSIFTXQLEN, (void *)&netifr) < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				/* Whithout return, because 100 is default and would work to. */
			}

			// Reset the structure for the next call.
			netifr = {};

			std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);
			netifr.ifr_name[IFNAMSIZ - 1] = 0x00;
#endif

			// Get the interface hwaddr
			if (::ioctl(socket.native_handle(), SIOCGIFHWADDR, (void*)&netifr) < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			osi::ethernet_address _ethernet_address;
			std::memcpy(_ethernet_address.data().data(), netifr.ifr_hwaddr.sa_data, _ethernet_address.data().size());
			set_ethernet_address(_ethernet_address);
		}

		set_name(ifr.ifr_name);

#else /* *BSD and Mac OS X */

		const std::string dev_type = (layer() == tap_adapter_layer::ethernet) ? "tap" : "tun";
		std::string interface_name = _name;

		descriptor_handler device;

		if (!_name.empty())
		{
			device = open_device("/dev/" + _name, ec);
		}
		else
		{
			for (unsigned int i = 0 ; !device.valid(); ++i)
			{
				interface_name = dev_type + boost::lexical_cast<std::string>(i);
				device = open_device("/dev/" + interface_name, ec);

				if (!device.valid() && (errno == ENOENT))
				{
					// We reached the end of the available tap adapters.
					break;
				}
			}
		}

		if (!device.valid())
		{
			ec = make_error_code(asiotap_error::no_such_tap_adapter);

			return;
		}

		struct stat st {};

		if (::fstat(device.native_handle(), &st) != 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return;
		}

		char namebuf[256];
		memset(namebuf, 0x00, sizeof(namebuf));

#ifdef __NetBSD__
		if (::devname_r(st.st_dev, S_IFCHR, namebuf, 255) != 0)
		{
#elif defined(__OpenBSD__)
		char* n = ::devname(st.st_dev, S_IFCHR);
		if(n)
		{
			strncpy(namebuf, n, 255);
#else
		if (::devname_r(st.st_dev, S_IFCHR, namebuf, 255) != NULL)
		{
#endif
			set_name(namebuf);
		}
		else
		{
			set_name(interface_name);
		}

		if (if_nametoindex(name().c_str()) == 0)
		{
			ec = make_error_code(asiotap_error::no_such_tap_adapter);

			return;
		}

		// Do not pass the descriptor to child
		::fcntl(device.native_handle(), F_SETFD, FD_CLOEXEC);

		descriptor_handler socket = open_socket(AF_INET, ec);

		if (!socket.valid())
		{
			return;
		}

		/* Get the hardware address of tap inteface. */
		struct ifaddrs* addrs = nullptr;

		if (getifaddrs(&addrs) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return;
		}

		boost::shared_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

		for (struct ifaddrs* ifa = addrs; ifa != NULL; ifa = ifa->ifa_next)
		{
			const std::string if_name = name();

			if ((ifa->ifa_addr->sa_family == AF_LINK) && !std::memcmp(ifa->ifa_name, if_name.c_str(), if_name.length()))
			{
				struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;

				if (sdl->sdl_type == IFT_ETHER)
				{
					osi::ethernet_address _ethernet_address;
					std::memcpy(_ethernet_address.data().data(), LLADDR(sdl), ethernet_address().data().size());
					set_ethernet_address(_ethernet_address);

					break;
				}
			}
		}
#endif

		update_mtu_from_device();

		if (descriptor().assign(device.release(), ec))
		{
			return;
		}
	}

	void posix_tap_adapter::open(const std::string& _name)
	{
		boost::system::error_code ec;

		open(_name, ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}
	}

	void posix_tap_adapter::destroy_device()
	{
		boost::system::error_code ec;

		destroy_device(ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}
	}

	void posix_tap_adapter::destroy_device(boost::system::error_code& ec)
	{
		// do not attempt to destroy interface if non-root
		if(getuid() != 0)
		{
			return;
		}

#if defined(MACINTOSH) || defined(OS_BSD)
		descriptor_handler socket = open_socket(AF_INET, ec);

		if (!socket.valid())
		{
			return;
		}

		struct ifreq ifr {};

		strncpy(ifr.ifr_name, name().c_str(), IFNAMSIZ);
		ifr.ifr_name[IFNAMSIZ - 1] = 0x00;

		// Destroy the virtual tap device
		if (ioctl(socket.native_handle(), SIOCIFDESTROY, &ifr) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());
		}
#else
		static_cast<void>(ec);
#endif
	}

	void posix_tap_adapter::set_connected_state(bool connected)
	{
		descriptor_handler socket = open_socket(AF_INET);

		struct ifreq netifr {};

		strncpy(netifr.ifr_name, name().c_str(), IFNAMSIZ);
		netifr.ifr_name[IFNAMSIZ - 1] = 0x00;

		// Get the interface flags
		if (::ioctl(socket.native_handle(), SIOCGIFFLAGS, static_cast<void*>(&netifr)) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		// as non-root, assume that existing TAP is correctly configured
		if (getuid() != 0 && m_existing_tap)
		{
			return;
		}

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
			// so do nothing for the moment.
			return;
#else
			netifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
#endif
		}

		// Set the interface UP
		if (::ioctl(socket.native_handle(), SIOCSIFFLAGS, static_cast<void*>(&netifr)) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}
	}

	ip_network_address_list posix_tap_adapter::get_ip_addresses()
	{
		ip_network_address_list result;

		struct ifaddrs* addrs = nullptr;

		if (::getifaddrs(&addrs) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		boost::shared_ptr<struct ifaddrs> paddrs(addrs, ::freeifaddrs);

		for (struct ifaddrs* ifa = paddrs.get(); ifa != nullptr; ifa = ifa->ifa_next)
		{
			const std::string ifname(ifa->ifa_name);

			if ((ifa->ifa_addr) && (ifname == name()))
			{
				if (ifa->ifa_addr->sa_family == AF_INET)
				{
					struct sockaddr_in* sai = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);

					boost::asio::ip::address_v4::bytes_type bytes;
					memcpy(bytes.data(), &sai->sin_addr, bytes.size());

					boost::asio::ip::address_v4 address(bytes);

					unsigned int prefix_len = sizeof(in_addr) * 8;

					if (ifa->ifa_netmask)
					{
						struct sockaddr_in* sain = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);

						prefix_len = netmask_to_prefix_len(sain->sin_addr);
					}

					result.push_back(ipv4_network_address{ address, prefix_len });
				}
				else if (ifa->ifa_addr->sa_family == AF_INET6)
				{
					struct sockaddr_in6* sai = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);

					boost::asio::ip::address_v6::bytes_type bytes;
					memcpy(bytes.data(), &sai->sin6_addr.s6_addr, bytes.size());

					boost::asio::ip::address_v6 address(bytes);

					unsigned int prefix_len = sizeof(in6_addr) * 8;

					if (ifa->ifa_netmask)
					{
						struct sockaddr_in6* sain = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_netmask);

						prefix_len = netmask_to_prefix_len(sain->sin6_addr);
					}

					result.push_back(ipv6_network_address{ address, prefix_len });
				}
			}
		}

		return result;
	}

	void posix_tap_adapter::configure(const configuration_type& configuration)
	{
		// as non-root, assume that existing TAP is correctly configured
		if(getuid() != 0 && m_existing_tap)
		{
			return;
		}

		if (configuration.ipv4.network_address)
		{
			if (layer() == tap_adapter_layer::ethernet)
			{
				set_ip_address_v4(*configuration.ipv4.network_address);
			}
			else
			{
				if (configuration.ipv4.remote_address)
				{
					set_remote_ip_address_v4(*configuration.ipv4.network_address, *configuration.ipv4.remote_address);
				}
			}
		}

		if (configuration.ipv6.network_address)
		{
			set_ip_address_v6(*configuration.ipv6.network_address);
		}

		if (configuration.mtu > 0)
		{
			set_device_mtu(configuration.mtu);
		}
	}

	void posix_tap_adapter::update_mtu_from_device()
	{
		descriptor_handler socket = open_socket(AF_INET);

		struct ifreq netifr {};

		strncpy(netifr.ifr_name, name().c_str(), IFNAMSIZ);
		netifr.ifr_name[IFNAMSIZ - 1] = 0x00;

		if (::ioctl(socket.native_handle(), SIOCGIFMTU, (void*)&netifr) >= 0)
		{
			set_mtu(netifr.ifr_mtu);
		}
		else
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}
	}

	void posix_tap_adapter::set_device_mtu(size_t _mtu)
	{
		descriptor_handler socket = open_socket(AF_INET);

		struct ifreq netifr {};

		strncpy(netifr.ifr_name, name().c_str(), IFNAMSIZ);
		netifr.ifr_name[IFNAMSIZ - 1] = 0x00;

		netifr.ifr_mtu = _mtu;

		if (::ioctl(socket.native_handle(), SIOCSIFMTU, (void*)&netifr) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}
	}

	void posix_tap_adapter::set_ip_address_v4(const ipv4_network_address& network_address)
	{
		const boost::asio::ip::address_v4& address = network_address.address();
		const unsigned int prefix_len = network_address.prefix_length();

		assert(prefix_len < 32);

		descriptor_handler socket = open_socket(AF_INET);

		ifreq ifr_a {};

		std::strncpy(ifr_a.ifr_name, name().c_str(), IFNAMSIZ);
		ifr_a.ifr_name[IFNAMSIZ - 1] = 0x00;

		sockaddr_in* ifr_a_addr = reinterpret_cast<sockaddr_in*>(&ifr_a.ifr_addr);
		ifr_a_addr->sin_family = AF_INET;
#ifdef OS_BSD
		ifr_a_addr->sin_len = sizeof(sockaddr_in);
#endif
		std::memcpy(&ifr_a_addr->sin_addr.s_addr, address.to_bytes().data(), address.to_bytes().size());

		if (::ioctl(socket.native_handle(), SIOCSIFADDR, &ifr_a) < 0)
		{
			if (errno == EEXIST)
			{
				// The address is already set. We ignore this.
			}
			else
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}
		}

		if (prefix_len > 0)
		{
			ifreq ifr_n {};

			std::strncpy(ifr_n.ifr_name, name().c_str(), IFNAMSIZ);
			ifr_n.ifr_name[IFNAMSIZ - 1] = 0x00;
			sockaddr_in* ifr_n_addr = reinterpret_cast<sockaddr_in*>(&ifr_n.ifr_addr);
			ifr_n_addr->sin_family = AF_INET;
#ifdef OS_BSD
			ifr_n_addr->sin_len = sizeof(sockaddr_in);
#endif
			ifr_n_addr->sin_addr.s_addr = htonl((0xFFFFFFFF >> (32 - prefix_len)) << (32 - prefix_len));

			if (::ioctl(socket.native_handle(), SIOCSIFNETMASK, &ifr_n) < 0)
			{
				if (errno == EEXIST)
				{
					// The netmask is already set. We ignore this.
				}
				else
				{
					throw boost::system::system_error(errno, boost::system::system_category());
				}
			}
		}
	}

	void posix_tap_adapter::set_ip_address_v6(const ipv6_network_address& network_address)
	{
		const boost::asio::ip::address_v6& address = network_address.address();
		const unsigned int prefix_len = network_address.prefix_length();

		assert(prefix_len < 128);

		descriptor_handler socket = open_socket(AF_INET6);

#ifdef LINUX
		const unsigned int if_index = ::if_nametoindex(name().c_str());

		if (if_index == 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		in6_ifreq ifr {};
		std::memcpy(&ifr.ifr6_addr.s6_addr, address.to_bytes().data(), address.to_bytes().size());
		ifr.ifr6_prefixlen = prefix_len;
		ifr.ifr6_ifindex = if_index;

		if (::ioctl(socket.native_handle(), SIOCSIFADDR, &ifr) < 0)
#elif defined(MACINTOSH) || defined(OS_BSD)
		in6_aliasreq iar {};
		std::memcpy(iar.ifra_name, name().c_str(), name().length());
		reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_family = AF_INET6;
		reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_family = AF_INET6;
		std::memcpy(&reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_addr.s6_addr, address.to_bytes().data(), address.to_bytes().size());
		std::memset(reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr, 0xFF, prefix_len / 8);
		reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_addr.s6_addr[prefix_len / 8] = (0xFF << (8 - (prefix_len % 8)));
		iar.ifra_lifetime.ia6t_pltime = 0xFFFFFFFF;
		iar.ifra_lifetime.ia6t_vltime = 0xFFFFFFFF;

#ifdef SIN6_LEN
		reinterpret_cast<sockaddr_in6*>(&iar.ifra_addr)->sin6_len = sizeof(sockaddr_in6);
		reinterpret_cast<sockaddr_in6*>(&iar.ifra_prefixmask)->sin6_len = sizeof(sockaddr_in6);
#endif

		if (::ioctl(socket.native_handle(), SIOCAIFADDR_IN6, &iar) < 0)
#endif
		{
			if (errno == EEXIST)
			{
				// The address is already set. We ignore this.
			}
			else
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}
		}
	}

	void posix_tap_adapter::set_remote_ip_address_v4(const ipv4_network_address& network_address, const boost::asio::ip::address_v4& remote_address)
	{
		if (layer() != tap_adapter_layer::ip)
		{
			throw boost::system::system_error(make_error_code(asiotap_error::invalid_tap_adapter_layer));
		}

#ifdef MACINTOSH
		// The TUN adapter for Mac OSX has a weird behavior regarding routes and ioctl.

		// For some reason, on Mac, setting up the IP address using ioctl() doesn't work for TUN devices.
		m_route_manager.ifconfig(name(), network_address, remote_address);

		// OSX apparently does not create a route even though ifconfig indicates that the netmask is understood.
		// We must create it ourselves.
		m_route_manager.set_route(posix_route_manager::route_action::add, name(), network_address);
#else
		set_ip_address_v4(network_address);

		descriptor_handler socket = open_socket(AF_INET);

		ifreq ifr_d {};

		std::strncpy(ifr_d.ifr_name, name().c_str(), IFNAMSIZ);
		ifr_d.ifr_name[IFNAMSIZ - 1] = 0x00;
		sockaddr_in* ifr_dst_addr = reinterpret_cast<sockaddr_in*>(&ifr_d.ifr_dstaddr);
		ifr_dst_addr->sin_family = AF_INET;
#ifdef OS_BSD
		ifr_dst_addr->sin_len = sizeof(sockaddr_in);
#endif
		std::memcpy(&ifr_dst_addr->sin_addr.s_addr, remote_address.to_bytes().data(), remote_address.to_bytes().size());

		if (::ioctl(socket.native_handle(), SIOCSIFDSTADDR, &ifr_d) < 0)
		{
			if (errno == EEXIST)
			{
				// The address is already set. We ignore this.
			}
			else
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}
		}
#endif
	}
}

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

 namespace asiotap
 {
 	namespace
 	{
 		class descriptor_handler
 		{
 			public:

 				descriptor_handler() : m_fd(-1) {}
 				explicit descriptor_handler(int fd) : m_fd(-1) {}
 				descriptor_handler(const descriptor_handler&) = delete;
 				descriptor_handler& operator=(const descriptor_handler&) = delete;
 				descriptor_handler(descriptor_handler&& other) : m_fd(other.m_fd) { other.m_fd = -1; }
 				descriptor_handler& operator(descriptor_handler&& other) { using std::swap; swap(m_fd, other.m_fd); }
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

				return;
			}

			return descriptor_handler(device_fd);
 		}

 		descriptor_handler open_socket(boost::system::error_code& ec)
 		{
			const int socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

			if (socket_fd < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			return descriptor_handler(socket_fd);
 		}
 	}

 	std::map<std::string, std::string> posix_tap_adapter::enumerate(tap_adapter_layer _layer)
 	{
 		std::map<std::string, std::string> result;

 		struct ifaddrs* addrs = nullptr;

 		if (getifaddrs(&addrs) != -1)
 		{
 			boost::unique_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

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
 					}
 					case tap_adapter_layer::ip:
 					{
 						if (name.substr(0, 3) == "tun")
 						{
 							result[name] = name;
 						}
 					}
 				}
 			}
 		}

 		return result;
 	}

 	void posix_tap_adapter::open(size_t _mtu, boost::system::error_code& ec)
 	{
 		open("", _mtu, ec);
 	}

 	void posix_tap_adapter::open(const std::string& _name, size_t _mtu, boost::system::error_code& ec)
 	{
 		ec = boost::system::error_code();

#if defined(LINUX)
 		const std::string dev_name = (layer() == tap_adapter_layer::ethernet) ? "/dev/net/tap" : "/dev/net/tun";

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

 		struct ifreq ifr = {};

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
		}

		// Set the parameters on the tun device.
		if (::ioctl(device.native_handle(), TUNSETIFF, (void *)&ifr) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return;
		}

		descriptor_handler socket = open_socket(ec);

		if (!socket.valid())
		{
			return;
		}

 		{
			struct ifreq netifr = {};

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
			std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);
			netifr.ifr_qlen = 100; // 100 is the default value

			if (::ioctl(socket.native_handle(), SIOCSIFTXQLEN, (void *)&netifr) < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			// Reset the structure for the next call.
			netifr = {};
#endif
			// Set the MTU
			std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);

			if (_mtu > 0)
			{
				netifr.ifr_mtu = _mtu;

				::ioctl(socket.native_handle(), SIOCSIFMTU, (void*)&netifr);
			}

			if (::ioctl(socket.native_handle(), SIOCGIFMTU, (void*)&netifr) >= 0)
			{
				mtu() = netifr.ifr_mtu;
			}
			else
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			netifr = {};

			std::strncpy(netifr.ifr_name, ifr.ifr_name, IFNAMSIZ);

			// Get the interface hwaddr
			if (::ioctl(socket.native_handle(), SIOCGIFHWADDR, (void*)&netifr) < 0)
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}

			std::memcpy(ethernet_address().data().data(), netifr.ifr_hwaddr.sa_data, ethernet_address().data().size());
		}

		name() = ifr.ifr_name;

#else /* *BSD and Mac OS X */

 		const std::string dev_name = (layer() == tap_adapter_layer::ethernet) ? "/dev/tap" : "/dev/tun";

		descriptor_handler device;

		if (!_name.empty())
		{
			device = open_device("/dev/" + _name, ec);
		}
		else
		{
			device = open_device(dev_name, ec);

			if (!device.valid() && (errno == ENOENT))
			{
				for (unsigned int i = 0 ; !device.valid(); ++i)
				{
					device = open_device(dev_name + boost::lexical_cast<std::string>(i), ec);

					if (!device.valid() && (errno == ENOENT))
					{
						// We reached the end of the available tap adapters.
						break;
					}
				}
			}
		}

		if (!device.valid())
		{
			return;
		}

		struct stat st;

		::fstat(device.native_handle(), &st);

		name() = ::devname(st.st_rdev, S_IFCHR);

		if (if_nametoindex(name().c_str()) == 0)
		{
			ec = make_error_code(asiotap_error::no_such_tap_adapter);

			return;
		}

		// Do not pass the descriptor to child
		::fcntl(device.native_handle(), F_SETFD, FD_CLOEXEC);

		descriptor_handler socket = open_socket(ec);

		if (!socket.valid())
		{
			return;
		}

		{
			struct ifreq netifr = {};

			// Set the MTU
			strncpy(netifr.ifr_name, m_name.c_str(), IFNAMSIZ);

			if (_mtu > 0)
			{
				netifr.ifr_mtu = _mtu;

				::ioctl(socket.native_handle(), SIOCSIFMTU, (void*)&netifr);
			}

			if (::ioctl(socket.native_handle(), SIOCGIFMTU, (void*)&netifr) >= 0)
			{
				mtu() = netifr.ifr_mtu;
			}
			else
			{
				ec = boost::system::error_code(errno, boost::system::system_category());

				return;
			}
		}

		/* Get the hardware address of tap inteface. */
		struct ifaddrs* addrs = NULL;

		if (getifaddrs(&addrs) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return;
		}

		boost::unique_ptr<struct ifaddrs> paddrs(addrs, freeifaddrs);

		for (struct ifaddrs* ifa = addrs; ifa != NULL; ifa = ifa->ifa_next)
		{
			const std::string if_name = name();

			if ((ifa->ifa_addr->sa_family == AF_LINK) && !std::memcmp(ifa->ifa_name, if_name.c_str(), if_name.length()))
			{
				struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;

				if (sdl->sdl_type == IFT_ETHER)
				{
					std::memcpy(ethernet_address().data().data(), LLADDR(sdl), ethernet_address().data().siwe());

					break;
				}
			}
		}
#endif

		if (descriptor.assign(device.release(), ec))
		{
			return;
		}
	}

	void posix_tap_adapter::open(const std::string& _name, size_t _mtu)
	{
		boost::system::error_code ec;

		open(_name, _mtu, ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}
	}
}

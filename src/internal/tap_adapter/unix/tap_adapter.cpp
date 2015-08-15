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

#ifdef LINUX

#include <linux/if_tun.h>

// Replacement structure since the include of linux/ipv6.h introduces
// conflicts.
// If someone comes up with a better solution, feel free to contribute.
struct in6_ifreq
{
	struct in6_addr ifr6_addr; /**< IPv6 address */
	uint32_t ifr6_prefixlen; /**< Length of the prefix */
	int ifr6_ifindex; /**< Interface index */
};

#elif defined(MACINTOSH) || defined(BSD)

// Note for Mac OS X users : you have to download and install the tun/tap
// driver from (http://tuntaposx.sourceforge.net).

#include <net/if_var.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>

#endif

namespace freelan {

using namespace boost::asio;

namespace {
    template <int Name>
    class BasicInterfaceCommand {
        public:
            explicit BasicInterfaceCommand(const std::string& _interface_name) :
                m_ifr() {
                strncpy(m_ifr.ifr_name, _interface_name.c_str(), IFNAMSIZ);
            }

            int name() const { return Name; }
            void* data() { return &m_ifr; }
            std::string interface_name() const { return m_ifr.ifr_name; }

        protected:
            struct ifreq m_ifr;
    };

#if defined(MACINTOSH) || defined(BSD)
    class InterfaceDestroy : public BasicInterfaceCommand<SIOCIFDESTROY> {
        public:
            explicit InterfaceDestroy(const std::string& _interface_name) :
                BasicInterfaceCommand(_interface_name)
            {}
    };
#endif

    class TunSetInterfaceFunction : public BasicInterfaceCommand<TUNSETIFF> {
        public:
            explicit TunSetInterfaceFunction(const std::string& _interface_name, int flags) :
                BasicInterfaceCommand(_interface_name) {
                m_ifr.ifr_flags = flags;
            }
    };

    class InterfaceSetTransferQueueLength : public BasicInterfaceCommand<SIOCSIFTXQLEN> {
        public:
            explicit InterfaceSetTransferQueueLength(const std::string& _interface_name, int queue_length) :
                BasicInterfaceCommand(_interface_name) {
                m_ifr.ifr_qlen = queue_length;
            }
    };

    class InterfaceGetHardwareAddress : public BasicInterfaceCommand<SIOCGIFHWADDR> {
        public:
            explicit InterfaceGetHardwareAddress(const std::string& _interface_name) :
                BasicInterfaceCommand(_interface_name) {
            }

            EthernetAddress value() const {
                EthernetAddress::value_type result;
                std::memcpy(result.data(), m_ifr.ifr_hwaddr.sa_data, result.size());

                return result;
            }
    };

    class InterfaceGetMTU : public BasicInterfaceCommand<SIOCGIFMTU> {
        public:
            explicit InterfaceGetMTU(const std::string& _interface_name) :
                BasicInterfaceCommand(_interface_name) {
            }

            int value() const { return m_ifr.ifr_mtu; }
    };

    posix::stream_descriptor open_device(io_service& _io_service, const std::string& name, boost::system::error_code& ec) {
        const int device_fd = ::open(name.c_str(), O_RDWR);

        if (device_fd < 0)
        {
            ec = boost::system::error_code(errno, boost::system::system_category());

            return posix::stream_descriptor(_io_service);
        }

        return posix::stream_descriptor(_io_service, device_fd);
    }
}

boost::system::error_code TapAdapter::open(const std::string& _name, boost::system::error_code& ec) {
    ec = boost::system::error_code();

    LOG(LogLevel::DEBUG, "tap_adapter::open", "start") \
        .attach("name", _name);

#ifdef LINUX
    const std::string dev_name = (layer() == TapAdapterLayer::ethernet) ? "/dev/net/tap" : "/dev/net/tun";

    if (::access(dev_name.c_str(), F_OK) == -1)
    {
        if (errno != ENOENT)
        {
            // Unable to access the tap adapter yet it exists: this is an error.
            LOG(LogLevel::ERROR, "tap_adapter::open", "access_denied") \
                .attach("name", _name);

            return (ec = boost::system::error_code(errno, boost::system::system_category()));
        }

        // No tap found, create one.
        if (::mknod(dev_name.c_str(), S_IFCHR | S_IRUSR | S_IWUSR, ::makedev(10, 200)) == -1)
        {
            LOG(LogLevel::ERROR, "tap_adapter::open", "device_creation_failed") \
                .attach("name", _name);

            return (ec = boost::system::error_code(errno, boost::system::system_category()));
        }
    }

    auto device = open_device(get_io_service(), dev_name, ec);

    if (ec) {
        LOG(LogLevel::ERROR, "tap_adapter::open", "device_opening_failed") \
            .attach("name", _name) \
            .attach("dev_name", dev_name);

        return ec;
    }

    int flags = IFF_NO_PI;

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
    flags |= IFF_ONE_QUEUE;
#endif

    if (layer() == TapAdapterLayer::ethernet) {
        flags |= IFF_TAP;
    } else {
        flags |= IFF_TUN;
    }

    std::string interface_name;

    {
        TunSetInterfaceFunction command(_name, flags);

        if (device.io_control(command, ec)) {
            LOG(LogLevel::ERROR, "tap_adapter::open", "set_interface_function_failed") \
                .attach("name", _name) \
                .attach("flags", flags);

            return ec;
        }

        interface_name = command.interface_name();
    }

    auto socket = ip::udp::socket(get_io_service());

    if (socket.open(ip::udp::v4(), ec)) {
        LOG(LogLevel::ERROR, "tap_adapter::open", "socket_opening_failed") \
            .attach("name", _name);

        return ec;
    }

#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
    {
        const auto queue_length = 100;
        InterfaceSetTransferQueueLength command(interface_name, queue_length);

        if (socket.io_control(command, ec)) {
            LOG(LogLevel::ERROR, "tap_adapter::open", "set_interface_queue_length_failed") \
                .attach("name", _name) \
                .attach("interface_name", interface_name) \
                .attach("queue_length", queue_length);

            return ec;
        }
    }
#endif

    {
        InterfaceGetHardwareAddress command(interface_name);

        if (socket.io_control(command, ec)) {
            LOG(LogLevel::ERROR, "tap_adapter::open", "get_interface_hardware_address_failed") \
                .attach("name", _name) \
                .attach("interface_name", interface_name);

            return ec;
        }

        set_ethernet_address(command.value());
    }

    set_name(interface_name);

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

    if (::devname_r(st.st_dev, S_IFCHR, namebuf, 255) != NULL)
    {
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

    {
        InterfaceGetMTU command(interface_name);

        if (socket.io_control(command, ec)) {
            LOG(LogLevel::ERROR, "tap_adapter::open", "get_interface_mtu_failed") \
                .attach("name", _name) \
                .attach("interface_name", interface_name);

            return ec;
        }

        set_mtu(command.value());
    }

    descriptor().assign(device.release());

    return ec;
}

boost::system::error_code TapAdapter::destroy_device(boost::system::error_code& ec) {
#if defined(MACINTOSH) || defined(BSD)
    auto socket = ip::udp::socket(get_io_service());

    if (socket.open(ip::udp::v4(), ec)) {
        return ec;
    }

    InterfaceDestroy command(name());
    return socket.io_control(command, ec);
#else
    return ec;
#endif
}

}

/**
 * \file tap.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple TAP test program.
 */
//Use the behavior of Boost from bevor 1.63
#define BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX

#include <asiotap/asiotap.hpp>
#include <asiotap/osi/ethernet_filter.hpp>
#include <asiotap/osi/arp_filter.hpp>
#include <asiotap/osi/ipv4_filter.hpp>
#include <asiotap/osi/ipv6_filter.hpp>
#include <asiotap/osi/icmp_filter.hpp>
#include <asiotap/osi/udp_filter.hpp>
#include <asiotap/osi/bootp_filter.hpp>
#include <asiotap/osi/dhcp_filter.hpp>
#include <asiotap/osi/complex_filter.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

static volatile bool signaled = false;
static boost::function<void ()> stop_function = 0;

static void signal_handler(int code)
{
	switch (code)
	{
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
			if (!signaled && stop_function)
			{
				signaled = true;
				std::cerr << "Signal caught: stopping..." << std::endl;

				stop_function();
				stop_function = 0;
			}
			break;
		default:
			break;
	}
}

static bool register_signal_handlers()
{
	if (signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGTERM signals." << std::endl;
		return false;
	}

	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGINT signals." << std::endl;
		return false;
	}

	if (signal(SIGABRT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGABRT signals." << std::endl;
		return false;
	}

	return true;
}

static char my_buf[2048];

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);
void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);
void ethernet_frame_read(asiotap::osi::const_helper<asiotap::osi::ethernet_frame> frame);
void arp_frame_read(asiotap::osi::const_helper<asiotap::osi::arp_frame> frame);
void ipv4_frame_read(asiotap::osi::const_helper<asiotap::osi::ipv4_frame> frame);
void ipv6_frame_read(asiotap::osi::const_helper<asiotap::osi::ipv6_frame> frame);
void icmp_frame_read(asiotap::osi::const_helper<asiotap::osi::icmp_frame> frame);
void udp_frame_read(asiotap::osi::const_helper<asiotap::osi::udp_frame> frame);
void bootp_frame_read(asiotap::osi::const_helper<asiotap::osi::bootp_frame> frame);
void dhcp_frame_read(asiotap::osi::const_helper<asiotap::osi::dhcp_frame> frame);

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	if (!ec)
	{
		std::cout << "Write: " << cnt << " bytes." << std::endl;

		tap_adapter.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));
	}
	else
	{
		std::cout << "Write: " << cnt << " bytes. Error: " << ec.message() << std::endl;
	}
}

void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	if (!ec)
	{
		std::cout << "Read: " << cnt << " bytes." << std::endl;

		namespace ao = asiotap::osi;

		boost::asio::const_buffer buffer(my_buf, cnt);

		ao::filter<ao::ethernet_frame> ethernet_filter;
		ethernet_filter.add_handler(&ethernet_frame_read);

		ao::complex_filter<ao::arp_frame, ao::ethernet_frame>::type arp_filter(ethernet_filter);
		arp_filter.add_handler(&arp_frame_read);

		ao::complex_filter<ao::ipv4_frame, ao::ethernet_frame>::type ipv4_filter(ethernet_filter);
		ipv4_filter.add_handler(&ipv4_frame_read);
		ipv4_filter.add_checksum_filter();

		ao::complex_filter<ao::ipv6_frame, ao::ethernet_frame>::type ipv6_filter(ethernet_filter);
		ipv6_filter.add_handler(&ipv6_frame_read);

		ao::complex_filter<ao::icmp_frame, ao::ipv4_frame, ao::ethernet_frame>::type icmp_ipv4_filter(ipv4_filter);
		icmp_ipv4_filter.add_handler(&icmp_frame_read);
		icmp_ipv4_filter.add_checksum_filter();

		ao::complex_filter<ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type udp_ipv4_filter(ipv4_filter);
		udp_ipv4_filter.add_handler(&udp_frame_read);
		udp_ipv4_filter.add_checksum_bridge_filter();

		ao::complex_filter<ao::udp_frame, ao::ipv6_frame, ao::ethernet_frame>::type udp_ipv6_filter(ipv6_filter);
		udp_ipv6_filter.add_handler(&udp_frame_read);
		udp_ipv6_filter.add_checksum_bridge_filter();

		ao::complex_filter<ao::bootp_frame, ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type bootp_filter(udp_ipv4_filter);
		bootp_filter.add_handler(&bootp_frame_read);

		ao::complex_filter<ao::dhcp_frame, ao::bootp_frame, ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type dhcp_filter(bootp_filter);
		dhcp_filter.add_handler(&dhcp_frame_read);

		ethernet_filter.parse(buffer);

		tap_adapter.async_write(boost::asio::buffer(buffer), boost::bind(&write_done, boost::ref(tap_adapter), _1, _2));
	}
	else
	{
		std::cout << "Read: " << cnt << " bytes. Error: " << ec.message() << std::endl;
	}
}

void ethernet_frame_read(asiotap::osi::const_helper<asiotap::osi::ethernet_frame> frame)
{
	(void)frame;

	std::cout << "Ethernet frame" << std::endl;
}

void arp_frame_read(asiotap::osi::const_helper<asiotap::osi::arp_frame> frame)
{
	std::cout << "ARP frame: " << frame.sender_logical_address() << std::endl;
}

void ipv4_frame_read(asiotap::osi::const_helper<asiotap::osi::ipv4_frame> frame)
{
	std::cout << "IPv4 frame: " << frame.source() << " -> " << frame.destination() << std::endl;
}

void ipv6_frame_read(asiotap::osi::const_helper<asiotap::osi::ipv6_frame> frame)
{
	std::cout << "IPv6 frame: " << frame.source() << " -> " << frame.destination() << std::endl;
}

void icmp_frame_read(asiotap::osi::const_helper<asiotap::osi::icmp_frame> frame)
{
	std::cout << "ICMP frame: " << static_cast<int>(frame.type()) << ": " << static_cast<int>(frame.code()) << std::endl;
}

void udp_frame_read(asiotap::osi::const_helper<asiotap::osi::udp_frame> frame)
{
	std::cout << "UDP frame: " << frame.source() << " -> " << frame.destination() << std::endl;
}

void bootp_frame_read(asiotap::osi::const_helper<asiotap::osi::bootp_frame> frame)
{
	std::cout << "BOOTP frame. Options size: " << boost::asio::buffer_size(frame.options()) << std::endl;
}

void dhcp_frame_read(asiotap::osi::const_helper<asiotap::osi::dhcp_frame> frame)
{
	std::cout << "DHCP frame. Options: ";

	typedef asiotap::osi::const_helper<asiotap::osi::dhcp_frame>::const_iterator const_iterator;

	for (const_iterator it = frame.begin(); (it != frame.end()) && (it->tag() != asiotap::osi::dhcp_option::end); ++it)
	{
		std::cout << it->tag() << ", ";
	}

	std::cout << "end." << std::endl;
}

void close_tap_adapter(asiotap::tap_adapter& tap_adapter)
{
	tap_adapter.cancel();
	tap_adapter.set_connected_state(false);
	tap_adapter.close();
}

int main()
{
	if (!register_signal_handlers())
	{
		return EXIT_FAILURE;
	}

	try
	{
		boost::asio::io_service _io_service;

		asiotap::tap_adapter tap_adapter(_io_service, asiotap::tap_adapter_layer::ethernet);

		stop_function = boost::bind(&close_tap_adapter, boost::ref(tap_adapter));

		tap_adapter.open();

		asiotap::tap_adapter_configuration configuration {};
		asiotap::ipv4_network_address ipv4 = { boost::asio::ip::address_v4::from_string("9.0.0.1"), 24 };
		asiotap::ipv6_network_address ipv6 = { boost::asio::ip::address_v6::from_string("fe80::c887:eb51:aaaa:bbbb"), 64 };
		configuration.ipv4.network_address = ipv4;
		configuration.ipv6.network_address = ipv6;

		tap_adapter.configure(configuration);

		tap_adapter.set_connected_state(true);

		tap_adapter.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));

		const auto addresses = tap_adapter.get_ip_addresses();

		std::cout << "Current IP addresses for the interface:" << std::endl;

		for(auto&& address : addresses)
		{
			std::cout << address << std::endl;
		}

		_io_service.run();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

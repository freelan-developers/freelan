/**
 * \file proxy.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A program to test TAP proxies abilities.
 */

#include <asiotap/asiotap.hpp>
#include <asiotap/osi/arp_proxy.hpp>
#include <asiotap/osi/dhcp_proxy.hpp>
#include <asiotap/osi/complex_filter.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

//TODO: Remove this fix for MingW64 as soon as boost supports it
#if defined(WINDOWS) && !defined(MSV)
namespace boost
{
	void tss_cleanup_implemented()
	{
	}
}
#endif

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

static boost::array<char, 2048> read_buffer;
static boost::array<char, 2048> write_buffer;
asiotap::osi::filter<asiotap::osi::ethernet_frame> ethernet_filter;

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);
void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);
void do_write(asiotap::tap_adapter& tap_adapter, boost::asio::const_buffer buffer);

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	(void) tap_adapter;

	std::cout << "Write: " << cnt << " bytes. Error: " << ec << std::endl;
}

void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	std::cout << "Read: " << cnt << " bytes. Error: " << ec << std::endl;

	if (!ec)
	{
		ethernet_filter.parse(boost::asio::buffer(read_buffer, cnt));

		tap_adapter.async_read(boost::asio::buffer(read_buffer, sizeof(read_buffer)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));
	}
}

void do_write(asiotap::tap_adapter& tap_adapter, boost::asio::const_buffer buffer)
{
	tap_adapter.async_write(buffer, boost::bind(&write_done, boost::ref(tap_adapter), _1, _2));
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

	namespace ba = boost::asio;
	namespace ao = asiotap::osi;

	try
	{
		ba::io_service _io_service;

		asiotap::tap_adapter tap_adapter(_io_service);

		stop_function = boost::bind(&close_tap_adapter, boost::ref(tap_adapter));

		tap_adapter.open();
		tap_adapter.set_connected_state(true);

		tap_adapter.async_read(ba::buffer(read_buffer, sizeof(read_buffer)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));

		// The requested address
		ba::ip::address_v4 dhcp_server_ipv4_address = ba::ip::address_v4::from_string("9.0.0.0");
		ba::ip::address_v4 dhcp_server_ipv4_netmask = ba::ip::address_v4::from_string("255.255.255.0");
		ba::ip::address_v4 my_ipv4_address = ba::ip::address_v4::from_string("9.0.0.1");
		ba::ip::address_v4 other_ipv4_address = ba::ip::address_v4::from_string("9.0.0.2");

		// We need some filters
		ao::complex_filter<ao::arp_frame, ao::ethernet_frame>::type arp_filter(ethernet_filter);
		ao::complex_filter<ao::ipv4_frame, ao::ethernet_frame>::type ipv4_filter(ethernet_filter);
		ao::complex_filter<ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type udp_filter(ipv4_filter);
		ao::complex_filter<ao::bootp_frame, ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type bootp_filter(udp_filter);
		ao::complex_filter<ao::dhcp_frame, ao::bootp_frame, ao::udp_frame, ao::ipv4_frame, ao::ethernet_frame>::type dhcp_filter(bootp_filter);

		// We add the ARP proxy
		ao::proxy<ao::arp_frame> arp_proxy(ba::buffer(write_buffer), boost::bind(&do_write, boost::ref(tap_adapter), _1), arp_filter);
		arp_proxy.add_entry(other_ipv4_address, tap_adapter.ethernet_address());

		// We add the DHCP proxy
		ao::proxy<ao::dhcp_frame> dhcp_proxy(ba::buffer(write_buffer), boost::bind(&do_write, boost::ref(tap_adapter), _1), dhcp_filter);
		dhcp_proxy.set_hardware_address(tap_adapter.ethernet_address());
		dhcp_proxy.set_software_address(dhcp_server_ipv4_address);
		dhcp_proxy.add_entry(tap_adapter.ethernet_address(), my_ipv4_address, my_ipv4_netmask);

		// Let's run !
		_io_service.run();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

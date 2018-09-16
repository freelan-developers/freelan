/**
 * \file routes.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple routes test program.
 */
//Use the behavior of Boost from bevor 1.63
#define BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX

#include <asiotap/asiotap.hpp>
#include <asiotap/route_manager.hpp>

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

void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	if (!ec)
	{
		std::cout << "Read: " << cnt << " bytes." << std::endl;

		tap_adapter.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));
	}
	else
	{
		std::cout << "Read: " << cnt << " bytes. Error: " << ec << std::endl;
	}
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

		asiotap::tap_adapter tap_adapter(_io_service, asiotap::tap_adapter_layer::ip);

		stop_function = boost::bind(&close_tap_adapter, boost::ref(tap_adapter));

		tap_adapter.open();

		asiotap::tap_adapter_configuration configuration {};
		asiotap::ipv4_network_address ipv4 = { boost::asio::ip::address_v4::from_string("9.0.0.1"), 24 };
		boost::asio::ip::address_v4 ipv4_remote = boost::asio::ip::address_v4::from_string("9.0.0.0");
		asiotap::ipv6_network_address ipv6 = { boost::asio::ip::address_v6::from_string("fe80::c887:eb51:aaaa:bbbb"), 64 };
		configuration.ipv4.network_address = ipv4;
		configuration.ipv4.remote_address = ipv4_remote;
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

		std::cout << "Adding routes" << std::endl;

		asiotap::route_manager rmgr(_io_service);
		rmgr.set_route_registration_success_handler([](const asiotap::route_manager::route_type& route){
			std::cout << "Added route: " << route << std::endl;
		});
		rmgr.set_route_registration_failure_handler([](const asiotap::route_manager::route_type& route, const boost::system::system_error& ex){
			std::cout << "Failure adding route (" << route << "): " << ex.what() << std::endl;
		});
		rmgr.set_route_unregistration_success_handler([](const asiotap::route_manager::route_type& route){
			std::cout << "Removed route: " << route << std::endl;
		});
		rmgr.set_route_unregistration_failure_handler([](const asiotap::route_manager::route_type& route, const boost::system::system_error& ex){
			std::cout << "Failure removing route (" << route << "): " << ex.what() << std::endl;
		});

		{
			std::cout << "Testing route addition..." << std::endl;

			const auto r1 = rmgr.get_route_entry(tap_adapter.get_route(asiotap::to_ip_route(boost::asio::ip::address_v4::from_string("9.0.1.0"), 24)));
			const auto r2 = rmgr.get_route_entry(tap_adapter.get_route(asiotap::to_ip_route(boost::asio::ip::address_v4::from_string("9.0.2.0"), 24, boost::asio::ip::address_v4::from_string("9.0.0.2"))));

			std::cout << "Press any key now to delete the routes." << std::endl;
			std::cin.get();
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

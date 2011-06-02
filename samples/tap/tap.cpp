/**
 * \file tap.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple TAP test program.
 */

#include <asiotap/asiotap.hpp>

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

char my_buf[2048];

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);
void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt);

void write_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	std::cout << "Write: " << cnt << " bytes. Error: " << ec << std::endl;

	if (!ec)
	{
		tap_adapter.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));
	}
}

void read_done(asiotap::tap_adapter& tap_adapter, const boost::system::error_code& ec, size_t cnt)
{
	std::cout << "Read: " << cnt << " bytes. Error: " << ec << std::endl;

	if (!ec)
	{
		(void)tap_adapter;
		//tap_adapter.async_write(boost::asio::buffer(my_buf, cnt), boost::bind(&write_done, boost::ref(tap_adapter), _1, _2));
	}
}

void close_tap_adapter(asiotap::tap_adapter& tap_adapter)
{
	tap_adapter.remove_ip_address_v6(boost::asio::ip::address_v6::from_string("fe80::c887:eb51:aaaa:bbbb"), 64);
	tap_adapter.remove_ip_address_v4(boost::asio::ip::address_v4::from_string("9.0.0.1"), 24);
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

		asiotap::tap_adapter tap_adapter(_io_service);

		stop_function = boost::bind(&close_tap_adapter, boost::ref(tap_adapter));

		tap_adapter.open();
		tap_adapter.add_ip_address_v4(boost::asio::ip::address_v4::from_string("9.0.0.1"), 24);
		tap_adapter.add_ip_address_v6(boost::asio::ip::address_v6::from_string("fe80::c887:eb51:aaaa:bbbb"), 64);
		tap_adapter.set_connected_state(true);

		tap_adapter.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)), boost::bind(&read_done, boost::ref(tap_adapter), _1, _2));

		_io_service.run();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

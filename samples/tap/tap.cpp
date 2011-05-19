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

static boost::function<void ()> stop_function = 0;

static void signal_handler(int code)
{
	switch (code)
	{
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
			if (stop_function)
			{
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

int main()
{
	if (!register_signal_handlers())
	{
		return EXIT_FAILURE;
	}

	boost::asio::io_service _io_service;

	asiotap::tap_adapter tap_adapter(_io_service);

	stop_function = boost::bind(&boost::asio::io_service::stop, &_io_service);

	_io_service.run();

	return EXIT_SUCCESS;
}

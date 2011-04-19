/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>

#include <boost/asio.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

static boost::asio::io_service io_service;

static void signal_handler(int code)
{
	switch (code)
	{
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
			io_service.stop();
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

	fscp::server server(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));

	io_service.run();

	return EXIT_SUCCESS;
}

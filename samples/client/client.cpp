/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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

static bool on_hello_request(const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received HELLO request from " << sender << std::endl;

	return default_accept;
}

static void on_hello_response(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, const boost::posix_time::time_duration& time_duration)
{
	if (time_duration.is_special())
	{
		std::cout << "Received no HELLO response from " << sender << std::endl;
	} else
	{
		std::cout << "Received HELLO response from " << sender << " (" << time_duration.total_milliseconds() << " ms)" << std::endl;

		server.greet(sender, boost::bind(&on_hello_response, boost::ref(server), _1, _2));
	}
}

static void _stop_function(fscp::server& s1, fscp::server& s2)
{
	s1.close();
	s2.close();
}

int main()
{
	if (!register_signal_handlers())
	{
		return EXIT_FAILURE;
	}

	boost::asio::io_service _io_service;

	fscp::server alice_server(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
	fscp::server bob_server(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));

	boost::asio::ip::udp::resolver resolver(_io_service);
	boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
	boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

	alice_server.greet(bob_endpoint, boost::bind(&on_hello_response, boost::ref(alice_server), _1, _2));
	bob_server.set_hello_message_callback(&on_hello_request);

	stop_function = boost::bind(&_stop_function, boost::ref(alice_server), boost::ref(bob_server));

	_io_service.run();

	return EXIT_SUCCESS;
}

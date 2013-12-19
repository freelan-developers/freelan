/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>
#include <fscp/server2.hpp>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

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

static fscp::identity_store load_identity_store(const std::string& name)
{
	using cryptoplus::file;

	cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open(name + ".crt", "r"));
	cryptoplus::pkey::pkey key = cryptoplus::pkey::pkey::from_private_key(file::open(name + ".key", "r"));

	return fscp::identity_store(cert, key);
}

static void on_hello_response(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, const boost::system::error_code& ec)
{
	static_cast<void>(server);

	if (ec)
	{
		std::cout << "[" << name << "] Received no HELLO response from " << sender << ". Error is: " << ec << std::endl;
	} else
	{
		std::cout << "[" << name << "] Received HELLO response from " << sender << ". Result is: " << ec << std::endl;
	}
}

static void _stop_function(fscp::server2& s1, fscp::server2& s2, fscp::server2& s3)
{
	s1.close();
	s2.close();
	s3.close();
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	if (!register_signal_handlers())
	{
		return EXIT_FAILURE;
	}

	try
	{
		boost::asio::io_service _io_service;

		fscp::server2 alice_server(_io_service, load_identity_store("alice"));
		fscp::server2 bob_server(_io_service, load_identity_store("bob"));
		fscp::server2 chris_server(_io_service, load_identity_store("chris"));

		alice_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
		bob_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));
		chris_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12002));

		boost::asio::ip::udp::resolver resolver(_io_service);
		boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
		boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

		alice_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "alice", boost::ref(alice_server), bob_endpoint, _1));
		chris_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "chris", boost::ref(chris_server), bob_endpoint, _1));

		stop_function = boost::bind(&_stop_function, boost::ref(alice_server), boost::ref(bob_server), boost::ref(chris_server));

		_io_service.run();

		stop_function = 0;
	} catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

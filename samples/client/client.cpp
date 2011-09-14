/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>

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

static bool on_hello_request(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received HELLO request from " << sender << std::endl;

  server.async_introduce_to(sender);

	return default_accept;
}

static void on_hello_response(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, const boost::posix_time::time_duration& time_duration, bool success)
{
	if (!success)
	{
		std::cout << "Received no HELLO response from " << sender << " after " << time_duration.total_milliseconds() << " ms" << std::endl;
	} else
	{
		std::cout << "Received HELLO response from " << sender << " (" << time_duration.total_milliseconds() << " ms)" << std::endl;

		server.async_introduce_to(sender);
	}
}

static bool on_presentation(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, fscp::server::cert_type sig_cert, fscp::server::cert_type /*enc_cert*/, bool default_accept)
{
	std::cout << "Received PRESENTATION from " << sender << " (" << sig_cert.subject().oneline() << ")" << std::endl;

	server.async_request_session(sender);

  return default_accept;
}

static bool on_session_request(fscp::server& /*server*/, const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received SESSION_REQUEST from " << sender << std::endl;

	return default_accept;
}

static bool on_session(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received SESSION from " << sender << std::endl;

	server.async_send_data(sender, boost::asio::buffer(std::string("Hello you !")));

	return default_accept;
}

static void on_data(fscp::server& /*server*/, const boost::asio::ip::udp::endpoint& sender, boost::asio::const_buffer data)
{
	std::cout << "Received DATA from " << sender << ": " << std::string(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data)) << std::endl;
}

static void _stop_function(fscp::server& s1, fscp::server& s2)
{
	s1.close();
	s2.close();
}

static fscp::identity_store load_identity_store(const std::string& name)
{
	using cryptoplus::file;

	cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open(name + ".crt", "r"));
	cryptoplus::pkey::pkey key = cryptoplus::pkey::pkey::from_private_key(file::open(name + ".key", "r"));

	return fscp::identity_store(cert, key);
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

	boost::asio::io_service _io_service;

	fscp::server alice_server(_io_service, load_identity_store("alice"));
	fscp::server bob_server(_io_service, load_identity_store("bob"));

	alice_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
	bob_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));

	boost::asio::ip::udp::resolver resolver(_io_service);
	boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
	boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

	alice_server.async_greet(bob_endpoint, &on_hello_response);
	bob_server.set_hello_message_callback(&on_hello_request);
	alice_server.set_presentation_message_callback(&on_presentation);
	bob_server.set_presentation_message_callback(&on_presentation);
	alice_server.set_session_request_message_callback(&on_session_request);
	bob_server.set_session_request_message_callback(&on_session_request);
	alice_server.set_session_message_callback(&on_session);
	bob_server.set_session_message_callback(&on_session);
	alice_server.set_data_message_callback(&on_data);
	bob_server.set_data_message_callback(&on_data);

	stop_function = boost::bind(&_stop_function, boost::ref(alice_server), boost::ref(bob_server));

	_io_service.run();

	return EXIT_SUCCESS;
}

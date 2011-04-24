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

static bool on_hello_request(const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received HELLO request from " << sender << std::endl;

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

		server.introduce_to(sender);
	}
}

static void on_presentation(const boost::asio::ip::udp::endpoint& sender, fscp::server::cert_type sig_cert, fscp::server::cert_type /*enc_cert*/)
{
	std::cout << "Received PRESENTATION from " << sender << " (" << sig_cert.subject().oneline() << ")" << std::endl;
}

static void _stop_function(fscp::server& s1, fscp::server& s2)
{
	s1.close();
	s2.close();
}

static fscp::identity_store load_identity_store(const std::string& name)
{
	FILE* cert_file = fopen((name + ".crt").c_str(), "r");

	if (cert_file)
	{
		boost::shared_ptr<FILE> pcert_file(cert_file, fclose);

		FILE* key_file = fopen((name + ".key").c_str(), "r");

		if (key_file)
		{
			boost::shared_ptr<FILE> pkey_file(key_file, fclose);

			cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(pcert_file.get());
			cryptoplus::pkey::pkey key = cryptoplus::pkey::pkey::from_private_key(pkey_file.get());

			return fscp::identity_store(cert, key);
		}
	}

	throw std::runtime_error("Unable to create identity store for: " + name);
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

	fscp::server alice_server(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000), load_identity_store("alice"));
	fscp::server bob_server(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001), load_identity_store("bob"));

	boost::asio::ip::udp::resolver resolver(_io_service);
	boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
	boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

	alice_server.greet(bob_endpoint, boost::bind(&on_hello_response, boost::ref(alice_server), _1, _2, _3));
	bob_server.set_hello_message_callback(&on_hello_request);
	bob_server.set_presentation_message_callback(&on_presentation);

	stop_function = boost::bind(&_stop_function, boost::ref(alice_server), boost::ref(bob_server));

	_io_service.run();

	return EXIT_SUCCESS;
}

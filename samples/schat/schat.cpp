/**
 * \file schat.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple chat client.
 */

#include <fscp/fscp.hpp>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <csignal>

static void signal_handler(int code)
{
	switch (code)
	{
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
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

static bool on_session(fscp::server& /*server*/, const boost::asio::ip::udp::endpoint& sender, bool default_accept)
{
	std::cout << "Received SESSION from " << sender << std::endl;

	return default_accept;
}

static void on_session_established(fscp::server& /*server*/, const boost::asio::ip::udp::endpoint& host)
{
	std::cout << "Session established with " << host << std::endl;
}

static void on_session_lost(fscp::server& /*server*/, const boost::asio::ip::udp::endpoint& host)
{
	std::cout << "Session lost with " << host << std::endl;
}

static void on_data(fscp::server& server, const boost::asio::ip::udp::endpoint& sender, boost::asio::const_buffer data)
{
	try
	{
		const std::string sender_name = server.get_presentation(sender).signature_certificate().subject().find(NID_commonName)->data().str();
		std::cout << sender_name << ": " << std::string(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data)) << std::endl;
	}
	catch (std::exception&)
	{
	}
}

int main(int argc, char** argv)
{
	register_signal_handlers();

	try
	{
		if (argc != 5)
		{
			std::cerr << "Usage: schat <certificate> <private_key> <listen_host> <listen_port>" << std::endl;

			return EXIT_FAILURE;
		}

		const std::string certificate_filename(argv[1]);
		const std::string private_key_filename(argv[2]);
		const std::string listen_host(argv[3]);
		const std::string listen_port(argv[4]);

		cryptoplus::crypto_initializer crypto_initializer;
		cryptoplus::algorithms_initializer algorithms_initializer;
		cryptoplus::error::error_strings_initializer error_strings_initializer;

		boost::asio::io_service io_service;

		boost::asio::ip::udp::resolver listen_resolver(io_service);
		boost::asio::ip::udp::resolver::query listen_query(listen_host, listen_port);
		boost::asio::ip::udp::endpoint listen_ep = *listen_resolver.resolve(listen_query);

		cryptoplus::x509::certificate certificate = cryptoplus::x509::certificate::from_certificate(cryptoplus::file::open(certificate_filename, "r"));
		cryptoplus::pkey::pkey private_key = cryptoplus::pkey::pkey::from_private_key(cryptoplus::file::open(private_key_filename, "r"));

		const std::string local_name = certificate.subject().find(NID_commonName)->data().str();

		fscp::server server(io_service, listen_ep, fscp::identity_store(certificate, private_key));

		server.set_hello_message_callback(&on_hello_request);
		server.set_presentation_message_callback(&on_presentation);
		server.set_session_request_message_callback(&on_session_request);
		server.set_session_message_callback(&on_session);
		server.set_session_established_callback(&on_session_established);
		server.set_session_lost_callback(&on_session_lost);
		server.set_data_message_callback(&on_data);

		std::cout << "Chat started. Type !quit to exit." << std::endl;

		boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));

		char line[512];

		while (std::cin.getline(line, sizeof(line)))
		{
			if (line[0] == '!')
			{
				std::istringstream iss(line + 1);

				std::string command;

				if (iss >> command)
				{
					if (command == "connect")
					{
						std::string host, port;

						iss >> host >> port;

						boost::asio::ip::udp::resolver resolver(io_service);
						boost::asio::ip::udp::resolver::query query(host, port);

						try
						{
							boost::asio::ip::udp::endpoint ep = *resolver.resolve(query);

							server.async_greet(ep, &on_hello_response);
							std::cout << "Contacting " << ep << "..." << std::endl;
						}
						catch (std::exception& ex)
						{
							std::cerr << "Unable to resolve the specified host/port: " << ex.what() << std::endl;
						}
					} else if (command == "quit" || command == "exit")
					{
						break;
					}
				}
			} else
			{
				server.async_send_data_to_all(boost::asio::buffer(line, strlen(line)));
			}
		}

		std::cout << "Chat closing..." << std::endl;

		server.close();
		thread.join();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

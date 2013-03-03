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

static fscp::identity_store load_identity_store(const std::string& name)
{
	using cryptoplus::file;

	cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open(name + ".crt", "r"));
	cryptoplus::pkey::pkey key = cryptoplus::pkey::pkey::from_private_key(file::open(name + ".key", "r"));

	return fscp::identity_store(cert, key);
}

static bool on_hello_request(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, bool default_accept)
{
	std::cout << "[" << name << "] Received HELLO request from " << sender << std::endl;

  server.async_introduce_to(sender);

	return default_accept;
}

static void on_hello_response(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, const boost::posix_time::time_duration& time_duration, bool success)
{
	if (!success)
	{
		std::cout << "[" << name << "] Received no HELLO response from " << sender << " after " << time_duration.total_milliseconds() << " ms" << std::endl;
	} else
	{
		std::cout << "[" << name << "] Received HELLO response from " << sender << " (" << time_duration.total_milliseconds() << " ms)" << std::endl;

		server.async_introduce_to(sender);
	}
}

static bool on_presentation(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::server::cert_type sig_cert, fscp::server::cert_type /*enc_cert*/, bool /*is_new*/)
{
	std::cout << "[" << name << "] Received PRESENTATION from " << sender << " (" << sig_cert.subject().oneline() << ")" << std::endl;

	server.async_request_session(sender);

  return true;
}

static bool on_session_request(const std::string& name, fscp::server& /*server*/, const fscp::server::ep_type& sender, const fscp::cipher_algorithm_list_type&, const fscp::message_digest_algorithm_list_type&, bool default_accept)
{
	std::cout << "[" << name << "] Received SESSION_REQUEST from " << sender << std::endl;

	return default_accept;
}

static bool on_session(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, bool default_accept)
{
	std::cout << "[" << name << "] Received SESSION from " << sender << std::endl;

	server.async_send_data(sender, fscp::CHANNEL_NUMBER_3, boost::asio::buffer(std::string("Hello ! I'm " + name)));

	return default_accept;
}

static void on_data(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::channel_number_type channel_number, boost::asio::const_buffer data)
{
	const std::string str_data(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data));

	std::cout << "[" << name << "] Received DATA on channel " << static_cast<unsigned int>(channel_number) << " from " << sender << ": " << str_data << std::endl;

	if (name == "alice")
	{
		if (str_data == "Hello ! I'm chris")
		{
			const std::string common_name = server.identity().signature_certificate().subject().find(NID_commonName)->data().str();
			const std::string new_common_name = "denis";

			if (common_name != new_common_name)
			{
				std::cout << "[" << name << "] My current name is " << common_name << ". Switching to " << new_common_name << "." << std::endl;

				server.set_identity(load_identity_store(new_common_name));
			}
		}
		else
		{
			using cryptoplus::file;

			cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open("chris.crt", "r"));

			server.async_send_contact_request(sender, cert);
		}
	}
}

static bool on_contact_request_message(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::server::cert_type cert, const fscp::server::ep_type& target)
{
	(void)server;

	std::cout << "[" << name << "] Received CONTACT_REQUEST from " << sender << ": Where is " << cert.subject().oneline() << " ? (Answer: " << target << ")" << std::endl;

	return true;
}

static void on_contact_message(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::server::cert_type cert, const fscp::server::ep_type& target)
{
	std::cout << "[" << name << "] Received CONTACT from " << sender << ": " << cert.subject().oneline() << " is at " << target << std::endl;

	server.async_greet(target, boost::bind(&on_hello_response, name, boost::ref(server), _1, _2, _3));
}

static void _stop_function(fscp::server& s1, fscp::server& s2, fscp::server& s3)
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

		fscp::server alice_server(_io_service, load_identity_store("alice"));
		fscp::server bob_server(_io_service, load_identity_store("bob"));
		fscp::server chris_server(_io_service, load_identity_store("chris"));

		alice_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
		bob_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));
		chris_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12002));

		boost::asio::ip::udp::resolver resolver(_io_service);
		boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
		boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

		alice_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "alice", boost::ref(alice_server), _1, _2, _3));
		chris_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "chris", boost::ref(chris_server), _1, _2, _3));

		bob_server.set_hello_message_callback(boost::bind(&on_hello_request, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_hello_message_callback(boost::bind(&on_hello_request, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_presentation_message_callback(boost::bind(&on_presentation, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_presentation_message_callback(boost::bind(&on_presentation, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_presentation_message_callback(boost::bind(&on_presentation, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_request_message_callback(boost::bind(&on_session_request, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_request_message_callback(boost::bind(&on_session_request, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_request_message_callback(boost::bind(&on_session_request, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_message_callback(boost::bind(&on_session, "alice", boost::ref(alice_server), _1, _2));
		bob_server.set_session_message_callback(boost::bind(&on_session, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_session_message_callback(boost::bind(&on_session, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_data_message_callback(boost::bind(&on_data, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_data_message_callback(boost::bind(&on_data, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_data_message_callback(boost::bind(&on_data, "chris", boost::ref(chris_server), _1, _2, _3));

		bob_server.set_contact_request_message_callback(boost::bind(&on_contact_request_message, "bob", boost::ref(bob_server), _1, _2, _3));

		alice_server.set_contact_message_callback(boost::bind(&on_contact_message, "alice", boost::ref(alice_server), _1, _2, _3));

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

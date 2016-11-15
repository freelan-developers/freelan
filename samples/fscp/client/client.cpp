/**
 * \file client.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A simple client.
 */

#include <fscp/fscp.hpp>
#include <fscp/server.hpp>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

static boost::mutex output_mutex;

using boost::mutex;

void signal_handler(const boost::system::error_code& error, int signal_number, boost::function<void ()> stop_function)
{
	if (!error)
	{
		{
			mutex::scoped_lock lock(output_mutex);
			std::cerr << "Signal caught (" << signal_number << "): exiting..." << std::endl;
		}

		stop_function();
	}
}

static void simple_handler(const std::string& name, const std::string& msg, const boost::system::error_code& ec)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] " << msg << ": ";

	if (ec)
	{
		std::cout << ec.message();
	}
	else
	{
		std::cout << "OK";
	}

	std::cout << std::endl;
}

static bool on_hello(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, bool default_accept)
{
	static_cast<void>(server);

	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received HELLO request from " << sender << " (default accept is: " << default_accept << ")" << std::endl;

	server.async_introduce_to(sender, boost::bind(&simple_handler, name, "async_introduce_to()", _1));

	return default_accept;
}

static void on_hello_response(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, const boost::system::error_code& ec, const boost::posix_time::time_duration& duration)
{
	mutex::scoped_lock lock(output_mutex);

	if (ec)
	{
		std::cout << "[" << name << "] Received no HELLO response from " << sender << " after " << duration << ": " << ec.message() << std::endl;
	}
	else
	{
		std::cout << "[" << name << "] Received HELLO response from " << sender << " after " << duration << ": " << ec.message() << std::endl;

		server.async_introduce_to(sender, boost::bind(&simple_handler, name, "async_introduce_to()", _1));

		std::cout << "[" << name << "] Sending a presentation message to " << sender << std::endl;
	}
}

static bool on_presentation(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::server::cert_type sig_cert, fscp::server::presentation_status_type status)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received PRESENTATION from " << sender << " (" << sig_cert.subject() << ") - " << status << std::endl;

	server.async_request_session(sender, boost::bind(&simple_handler, name, "async_request_session()", _1));

	return true;
}

static bool on_session_request(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, const fscp::cipher_suite_list_type&, const fscp::elliptic_curve_list_type&, bool default_accept)
{
	mutex::scoped_lock lock(output_mutex);

	static_cast<void>(server);

	std::cout << "[" << name << "] Received SESSION_REQUEST from " << sender << ". Default accept is: " << default_accept << std::endl;

	return default_accept;
}

static bool on_session(const std::string& name, fscp::server&, const fscp::server::ep_type& sender, fscp::cipher_suite_type cs, fscp::elliptic_curve_type ec, bool default_accept)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received SESSION from " << sender << " (cipher suite: " << cs << ", elliptic curve: " << ec << ")" << std::endl;

	return default_accept;
}

static void on_session_failed(const std::string& name, fscp::server&, const fscp::server::ep_type& host, bool is_new)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session failed with " << host << std::endl;
	std::cout << "[" << name << "] New session: " << is_new << std::endl;
}

static void on_session_error(const std::string& name, fscp::server&, const fscp::server::ep_type& host, bool is_new, const std::exception& error)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session error with " << host << ": " << error.what() << std::endl;
	std::cout << "[" << name << "] New session: " << is_new << std::endl;
}

static void on_session_established(const std::string& name, fscp::server& server, const fscp::server::ep_type& host, bool is_new, const fscp::cipher_suite_type& cs, const fscp::elliptic_curve_type& ec)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session established with " << host << std::endl;
	std::cout << "[" << name << "] New session: " << is_new << std::endl;
	std::cout << "[" << name << "] Cipher suite: " << cs << std::endl;
	std::cout << "[" << name << "] Elliptic curve: " << ec << std::endl;

	static const std::string HELLO = "Hello you !";

	server.async_send_data(host, fscp::CHANNEL_NUMBER_3, boost::asio::buffer(HELLO), boost::bind(&simple_handler, name, "async_send_data()", _1));

	if (name == "alice")
	{
		using cryptoplus::file;

		cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open("chris.crt", "r"));

		fscp::hash_list_type hash_list;
		hash_list.insert(fscp::get_certificate_hash(cert));

		server.async_send_contact_request(host, hash_list, boost::bind(&simple_handler, name, "async_send_contact_request()", _1));
	}
}

static void on_session_lost(const std::string& name, fscp::server&, const fscp::server::ep_type& host, fscp::server::session_loss_reason reason)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session lost with " << host << " (" << reason << ")" << std::endl;
}

static void on_data(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::channel_number_type channel_number, fscp::SharedBuffer, boost::asio::const_buffer data)
{
	static_cast<void>(server);

	static std::atomic<int> send_counter;

	if (channel_number == fscp::CHANNEL_NUMBER_3) {
		const std::string str_data(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data));

		mutex::scoped_lock lock(output_mutex);

		std::cout << "[" << name << "] Received DATA on channel " << static_cast<unsigned int>(channel_number) << " from " << sender << ": " << str_data << std::endl;

	} else if (channel_number == fscp::CHANNEL_NUMBER_4) {

		const int receive_counter = *boost::asio::buffer_cast<const int*>(data);

		mutex::scoped_lock lock(output_mutex);

		std::cout << "[" << name << "] Received DATA on channel " << static_cast<unsigned int>(channel_number) << " from " << sender << ": #" << receive_counter << std::endl;
	}

	if ((name == "alice") || (name == "chris")) {
		const boost::shared_ptr<int> local_counter(new int(send_counter++));

		server.async_send_data(
			sender,
			fscp::CHANNEL_NUMBER_4,
			boost::asio::buffer(local_counter.get(), sizeof(int)),
			[name, local_counter](const boost::system::error_code& ec) {
				simple_handler(name, "async_send_data()", ec);
			}
		);
	}
}

static bool on_contact_request_message(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::server::cert_type cert, fscp::hash_type hash, const fscp::server::ep_type& target)
{
	static_cast<void>(server);

	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received CONTACT_REQUEST from " << sender << ": Where is " << cert.subject() << " ? (Answer: " << hash << " is at " << target << ")" << std::endl;

	return true;
}

static void on_contact_message(const std::string& name, fscp::server& server, const fscp::server::ep_type& sender, fscp::hash_type hash, const fscp::server::ep_type& target)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received CONTACT from " << sender << ": " << hash << " is at " << target << std::endl;

	server.async_greet(target, boost::bind(&on_hello_response, name, boost::ref(server), target, _1, _2));
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	try
	{
		boost::asio::io_service _io_service;
		boost::asio::signal_set signals(_io_service, SIGINT, SIGTERM);
		fscp::logger _logger;

		using cryptoplus::file;

		cryptoplus::x509::certificate alice_cert = cryptoplus::x509::certificate::from_certificate(file::open("alice.crt", "r"));
		cryptoplus::pkey::pkey alice_key = cryptoplus::pkey::pkey::from_private_key(file::open("alice.key", "r"));
		cryptoplus::x509::certificate bob_cert = cryptoplus::x509::certificate::from_certificate(file::open("bob.crt", "r"));
		cryptoplus::pkey::pkey bob_key = cryptoplus::pkey::pkey::from_private_key(file::open("bob.key", "r"));
		cryptoplus::x509::certificate chris_cert = cryptoplus::x509::certificate::from_certificate(file::open("chris.crt", "r"));
		cryptoplus::pkey::pkey chris_key = cryptoplus::pkey::pkey::from_private_key(file::open("chris.key", "r"));

		fscp::server alice_server(_io_service, _logger, fscp::identity_store(alice_cert, alice_key));
		fscp::server bob_server(_io_service, _logger, fscp::identity_store(bob_cert, bob_key));
		fscp::server chris_server(_io_service, _logger, fscp::identity_store(chris_cert, chris_key));

		alice_server.set_hello_message_received_callback(boost::bind(&on_hello, "alice", boost::ref(alice_server), _1, _2));
		bob_server.set_hello_message_received_callback(boost::bind(&on_hello, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_hello_message_received_callback(boost::bind(&on_hello, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "chris", boost::ref(chris_server), _1, _2, _3));

		alice_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_message_received_callback(boost::bind(&on_session, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_message_received_callback(boost::bind(&on_session, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_message_received_callback(boost::bind(&on_session, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_failed_callback(boost::bind(&on_session_failed, "alice", boost::ref(alice_server), _1, _2));
		bob_server.set_session_failed_callback(boost::bind(&on_session_failed, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_session_failed_callback(boost::bind(&on_session_failed, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_session_error_callback(boost::bind(&on_session_error, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_session_error_callback(boost::bind(&on_session_error, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_session_error_callback(boost::bind(&on_session_error, "chris", boost::ref(chris_server), _1, _2, _3));

		alice_server.set_session_established_callback(boost::bind(&on_session_established, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_established_callback(boost::bind(&on_session_established, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_established_callback(boost::bind(&on_session_established, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_lost_callback(boost::bind(&on_session_lost, "alice", boost::ref(alice_server), _1, _2));
		bob_server.set_session_lost_callback(boost::bind(&on_session_lost, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_session_lost_callback(boost::bind(&on_session_lost, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_data_received_callback(boost::bind(&on_data, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_data_received_callback(boost::bind(&on_data, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_data_received_callback(boost::bind(&on_data, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		bob_server.set_contact_request_received_callback(boost::bind(&on_contact_request_message, "bob", boost::ref(bob_server), _1, _2, _3, _4));

		alice_server.set_contact_received_callback(boost::bind(&on_contact_message, "alice", boost::ref(alice_server), _1, _2, _3));

		alice_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
		bob_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));
		chris_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12002));

		boost::asio::ip::udp::resolver resolver(_io_service);
		const boost::asio::ip::udp::resolver::query alice_query("127.0.0.1", "12000");
		const boost::asio::ip::udp::resolver::query bob_query("127.0.0.1", "12001");
		const boost::asio::ip::udp::resolver::query chris_query("127.0.0.1", "12002");

		const boost::asio::ip::udp::endpoint alice_endpoint = *resolver.resolve(alice_query);
		const boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(bob_query);
		const boost::asio::ip::udp::endpoint chris_endpoint = *resolver.resolve(chris_query);

		alice_server.set_presentation(bob_endpoint, bob_cert, cryptoplus::buffer());
		alice_server.set_presentation(chris_endpoint, chris_cert, cryptoplus::buffer());
		bob_server.set_presentation(alice_endpoint, alice_cert, cryptoplus::buffer());
		bob_server.set_presentation(chris_endpoint, chris_cert, cryptoplus::buffer());
		chris_server.set_presentation(bob_endpoint, bob_cert, cryptoplus::buffer());
		chris_server.set_presentation(chris_endpoint, chris_cert, cryptoplus::buffer());

		alice_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "alice", boost::ref(alice_server), bob_endpoint, _1, _2));
		chris_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "chris", boost::ref(chris_server), bob_endpoint, _1, _2));

		auto stop_function = [&](){
			alice_server.close();
			bob_server.close();
			chris_server.close();
		};

		signals.async_wait(boost::bind(signal_handler, _1, _2, stop_function));
		boost::thread_group threads;

		const unsigned int THREAD_COUNT = boost::thread::hardware_concurrency();

		std::cout << "Starting client with " << THREAD_COUNT << " thread(s)." << std::endl;

		for (std::size_t i = 0; i < THREAD_COUNT; ++i)
		{
			threads.create_thread([&_io_service, i, &stop_function, &signals] () {
				{
					mutex::scoped_lock lock(output_mutex);
					std::cout << "Thread #" << i << " started." << std::endl;
				}

				try
				{
					_io_service.run();
				}
				catch (std::exception& ex)
				{
					mutex::scoped_lock lock(output_mutex);
					std::cout << "Fatal exception occurred in thread #" << i << ": " << ex.what() << std::endl;

					stop_function();
					signals.cancel();
				}

				{
					mutex::scoped_lock lock(output_mutex);
					std::cout << "Thread #" << i << " stopped." << std::endl;
				}
			});
		}

		threads.join_all();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

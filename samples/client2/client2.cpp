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
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

static boost::function<void ()> stop_function = 0;
static boost::mutex output_mutex;

using boost::mutex;

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

static void simple_handler(const std::string& name, const std::string& msg, const boost::system::error_code& ec)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] " << msg << ": " << ec.message() << std::endl;
}

static bool on_hello(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, bool default_accept)
{
	static_cast<void>(server);

	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received HELLO request from " << sender << " (default accept is: " << default_accept << ")" << std::endl;

	server.async_introduce_to(sender, boost::bind(&simple_handler, name, "async_introduce_to()", _1));

	return default_accept;
}

static void on_hello_response(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, const boost::system::error_code& ec, const boost::posix_time::time_duration& duration)
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

static bool on_presentation(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, fscp::server2::cert_type sig_cert, fscp::server2::cert_type /*enc_cert*/, bool is_new)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received PRESENTATION from " << sender << " (" << sig_cert.subject().oneline() << ") - " << (is_new ? "new" : "existing") << std::endl;

	server.async_request_session(sender, boost::bind(&simple_handler, name, "async_request_session()", _1));

	return true;
}

static bool on_session_request(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, const fscp::cipher_algorithm_list_type&, bool default_accept)
{
	mutex::scoped_lock lock(output_mutex);

	static_cast<void>(server);

	std::cout << "[" << name << "] Received SESSION_REQUEST from " << sender << std::endl;

	return default_accept;
}

static bool on_session(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, fscp::cipher_algorithm_type calg, bool default_accept)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received SESSION from " << sender << " (cipher: " << calg << ")" << std::endl;

	static const std::string HELLO = "Hello you !";

	server.async_send_data(sender, fscp::CHANNEL_NUMBER_3, boost::asio::buffer(HELLO), boost::bind(&simple_handler, name, "async_send_data()", _1));

	return default_accept;
}

static void on_session_failed(const std::string& name, fscp::server2&, const fscp::server2::ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session failed with " << host << std::endl;
	std::cout << "[" << name << "] New session: " << is_new << std::endl;
	std::cout << "[" << name << "] Local algorithms: " << local << std::endl;
	std::cout << "[" << name << "] Remote algorithms: " << remote << std::endl;
}

static void on_session_established(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session established with " << host << std::endl;
	std::cout << "[" << name << "] New session: " << is_new << std::endl;
	std::cout << "[" << name << "] Local algorithms: " << local << std::endl;
	std::cout << "[" << name << "] Remote algorithms: " << remote << std::endl;

	if (name == "alice")
	{
		using cryptoplus::file;

		cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_certificate(file::open("chris.crt", "r"));

		fscp::hash_list_type hash_list;
		hash_list.insert(fscp::get_certificate_hash(cert));

		server.async_send_contact_request(host, hash_list, boost::bind(&simple_handler, name, "async_send_contact_request()", _1));
	}
}

static void on_session_lost(const std::string& name, fscp::server2&, const fscp::server2::ep_type& host)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Session lost with " << host << std::endl;
}

static void on_data(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, fscp::channel_number_type channel_number, boost::asio::const_buffer data)
{
	static_cast<void>(server);

	const std::string str_data(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data));

	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received DATA on channel " << static_cast<unsigned int>(channel_number) << " from " << sender << ": " << str_data << std::endl;
}

static bool on_contact_request_message(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, fscp::server2::cert_type cert, fscp::hash_type hash, const fscp::server2::ep_type& target)
{
	static_cast<void>(server);

	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received CONTACT_REQUEST from " << sender << ": Where is " << cert.subject().oneline() << " ? (Answer: " << fscp::write(std::cout, hash) << " is at " << target << ")" << std::endl;

	return true;
}

static void on_contact_message(const std::string& name, fscp::server2& server, const fscp::server2::ep_type& sender, fscp::hash_type hash, const fscp::server2::ep_type& target)
{
	mutex::scoped_lock lock(output_mutex);

	std::cout << "[" << name << "] Received CONTACT from " << sender << ": " << fscp::write(std::cout, hash) << " is at " << target << std::endl;

	server.async_greet(target, boost::bind(&on_hello_response, name, boost::ref(server), target, _1, _2));
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

		alice_server.set_hello_message_received_callback(boost::bind(&on_hello, "alice", boost::ref(alice_server), _1, _2));
		bob_server.set_hello_message_received_callback(boost::bind(&on_hello, "bob", boost::ref(bob_server), _1, _2));
		chris_server.set_hello_message_received_callback(boost::bind(&on_hello, "chris", boost::ref(chris_server), _1, _2));

		alice_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_presentation_message_received_callback(boost::bind(&on_presentation, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_session_request_message_received_callback(boost::bind(&on_session_request, "chris", boost::ref(chris_server), _1, _2, _3));

		alice_server.set_session_message_received_callback(boost::bind(&on_session, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_session_message_received_callback(boost::bind(&on_session, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_session_message_received_callback(boost::bind(&on_session, "chris", boost::ref(chris_server), _1, _2, _3));

		alice_server.set_session_failed_callback(boost::bind(&on_session_failed, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_failed_callback(boost::bind(&on_session_failed, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_failed_callback(boost::bind(&on_session_failed, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_established_callback(boost::bind(&on_session_established, "alice", boost::ref(alice_server), _1, _2, _3, _4));
		bob_server.set_session_established_callback(boost::bind(&on_session_established, "bob", boost::ref(bob_server), _1, _2, _3, _4));
		chris_server.set_session_established_callback(boost::bind(&on_session_established, "chris", boost::ref(chris_server), _1, _2, _3, _4));

		alice_server.set_session_lost_callback(boost::bind(&on_session_lost, "alice", boost::ref(alice_server), _1));
		bob_server.set_session_lost_callback(boost::bind(&on_session_lost, "bob", boost::ref(bob_server), _1));
		chris_server.set_session_lost_callback(boost::bind(&on_session_lost, "chris", boost::ref(chris_server), _1));

		alice_server.set_data_received_callback(boost::bind(&on_data, "alice", boost::ref(alice_server), _1, _2, _3));
		bob_server.set_data_received_callback(boost::bind(&on_data, "bob", boost::ref(bob_server), _1, _2, _3));
		chris_server.set_data_received_callback(boost::bind(&on_data, "chris", boost::ref(chris_server), _1, _2, _3));

		bob_server.set_contact_request_received_callback(boost::bind(&on_contact_request_message, "bob", boost::ref(bob_server), _1, _2, _3, _4));

		alice_server.set_contact_received_callback(boost::bind(&on_contact_message, "alice", boost::ref(alice_server), _1, _2, _3));

		alice_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12000));
		bob_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12001));
		chris_server.open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 12002));

		boost::asio::ip::udp::resolver resolver(_io_service);
		boost::asio::ip::udp::resolver::query query("127.0.0.1", "12001");
		boost::asio::ip::udp::endpoint bob_endpoint = *resolver.resolve(query);

		alice_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "alice", boost::ref(alice_server), bob_endpoint, _1, _2));
		chris_server.async_greet(bob_endpoint, boost::bind(&on_hello_response, "chris", boost::ref(chris_server), bob_endpoint, _1, _2));

		stop_function = boost::bind(&_stop_function, boost::ref(alice_server), boost::ref(bob_server), boost::ref(chris_server));

		boost::thread_group threads;

		const unsigned int THREAD_COUNT = boost::thread::hardware_concurrency();

		std::cout << "Starting client with " << THREAD_COUNT << " thread(s)." << std::endl;

		for (std::size_t i = 0; i < THREAD_COUNT; ++i)
		{
			threads.create_thread(boost::bind(&boost::asio::io_service::run, &_io_service));
		}

		threads.join_all();

		stop_function = 0;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

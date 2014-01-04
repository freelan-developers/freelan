/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file core.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan core class.
 */

#include "core2.hpp"

#include "os.hpp"
#include "client.hpp"
#include "tap_adapter_switch_port.hpp"
#include "endpoint_switch_port.hpp"
#include "tap_adapter_router_port.hpp"
#include "endpoint_router_port.hpp"

#include <fscp/server_error.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/future.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <cassert>

namespace freelan
{
	using boost::asio::buffer;
	using boost::asio::buffer_cast;
	using boost::asio::buffer_size;

	namespace
	{
		typedef boost::function<void (const core::ep_type&)> resolve_success_handler_type;
		typedef core::simple_handler_type resolve_error_handler_type;

		void resolve_handler(const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator it, resolve_success_handler_type success_handler, resolve_error_handler_type error_handler)
		{
			if (ec)
			{
				success_handler(*it);
			}
			else
			{
				error_handler(ec);
			}
		}

		endpoint to_endpoint(const core::ep_type& host)
		{
			if (host.address().is_v4())
			{
				return ipv4_endpoint(host.address().to_v4(), host.port());
			}
			else
			{
				return ipv6_endpoint(host.address().to_v6(), host.port());
			}
		}

		template <typename SharedBufferType, typename Handler>
		class shared_buffer_handler
		{
			public:

				shared_buffer_handler(SharedBufferType _buffer, Handler _handler) :
					m_buffer(_buffer),
					m_handler(_handler)
				{}

				void operator()()
				{
					m_handler();
				}

				template <typename Arg1>
				void operator()(Arg1 arg1)
				{
					m_handler(arg1);
				}

				template <typename Arg1, typename Arg2>
				void operator()(Arg1 arg1, Arg2 arg2)
				{
					m_handler(arg1, arg2);
				}

			private:

				SharedBufferType m_buffer;
				Handler m_handler;
		};

		template <typename SharedBufferType, typename Handler>
		inline shared_buffer_handler<SharedBufferType, Handler> make_shared_buffer_handler(SharedBufferType _buffer, Handler _handler)
		{
			return shared_buffer_handler<SharedBufferType, Handler>(_buffer, _handler);
		}

		template <typename KeyType, typename ValueType, typename Handler>
		class results_gatherer
		{
			public:

				typedef std::set<KeyType> set_type;
				typedef std::map<KeyType, ValueType> map_type;

				results_gatherer(Handler handler, const set_type& keys) :
					m_handler(handler),
					m_keys(keys)
				{}

				void gather(const KeyType& key, const ValueType& value)
				{
					boost::mutex::scoped_lock lock(m_mutex);

					const size_t erased_count = m_keys.erase(key);

					// Ensure that gather was called only once for a given key.
					assert(erased_count == 1);

					m_results[key] = value;

					if (m_keys.empty())
					{
						m_handler(m_results);
					}
				}

			private:

				boost::mutex m_mutex;
				Handler m_handler;
				set_type m_keys;
				map_type m_results;
		};

		unsigned int get_auto_mtu_value()
		{
			const unsigned int default_mtu_value = 1500;
			const size_t static_payload_size = 20 + 8 + 4 + 22; // IP + UDP + FSCP HEADER + FSCP DATA HEADER

			return default_mtu_value - static_payload_size;
		}

		static const switch_::group_type TAP_ADAPTERS_GROUP = 0;
		static const switch_::group_type ENDPOINTS_GROUP = 1;
	}

	typedef boost::asio::ip::udp::resolver::query resolver_query;

	// Has to be put first, as static variables definition order matters.
	const int core::ex_data_index = cryptoplus::x509::store_context::register_index();

	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);
	const boost::posix_time::time_duration core::DYNAMIC_CONTACT_PERIOD = boost::posix_time::seconds(45);

	const std::string core::DEFAULT_SERVICE = "12000";

	core::core(boost::asio::io_service& io_service, const freelan::configuration& _configuration, const freelan::logger& _logger) :
		m_io_service(io_service),
		m_configuration(_configuration),
		m_logger(_logger),
		m_server(),
		m_contact_timer(m_io_service, CONTACT_PERIOD),
		m_dynamic_contact_timer(m_io_service, DYNAMIC_CONTACT_PERIOD),
		m_tap_adapter_strand(m_io_service),
		m_proxies_strand(m_io_service),
		m_arp_filter(m_ethernet_filter),
		m_ipv4_filter(m_ethernet_filter),
		m_udp_filter(m_ipv4_filter),
		m_bootp_filter(m_udp_filter),
		m_dhcp_filter(m_bootp_filter),
		m_switch(m_configuration.switch_),
		m_router(m_configuration.router)
	{
		if (!m_configuration.security.identity)
		{
			throw std::runtime_error("No user certificate or private key set. Unable to continue.");
		}

		m_arp_filter.add_handler(boost::bind(&core::do_handle_arp_frame, this, _1));
		m_dhcp_filter.add_handler(boost::bind(&core::do_handle_dhcp_frame, this, _1));
	}

	void core::open()
	{
		m_logger(LL_DEBUG) << "Opening core...";

		open_server();
		open_tap_adapter();

		m_logger(LL_DEBUG) << "Core opened.";
	}

	void core::close()
	{
		m_logger(LL_DEBUG) << "Closing core...";

		close_tap_adapter();
		close_server();

		m_logger(LL_DEBUG) << "Core closed.";
	}

	// Private methods

	bool core::is_banned(const boost::asio::ip::address& address) const
	{
		return has_address(m_configuration.fscp.never_contact_list.begin(), m_configuration.fscp.never_contact_list.end(), address);
	}

	void core::open_server()
	{
		m_server.reset(new fscp::server(m_io_service, *m_configuration.security.identity));

		m_server->set_cipher_capabilities(m_configuration.fscp.cipher_capabilities);

		m_server->set_hello_message_received_callback(boost::bind(&core::do_handle_hello_received, this, _1, _2));
		m_server->set_contact_request_received_callback(boost::bind(&core::do_handle_contact_request_received, this, _1, _2, _3, _4));
		m_server->set_contact_received_callback(boost::bind(&core::do_handle_contact_received, this, _1, _2, _3));
		m_server->set_presentation_message_received_callback(boost::bind(&core::do_handle_presentation_received, this, _1, _2, _3, _4));
		m_server->set_session_request_message_received_callback(boost::bind(&core::do_handle_session_request_received, this, _1, _2, _3));
		m_server->set_session_message_received_callback(boost::bind(&core::do_handle_session_received, this, _1, _2, _3));
		m_server->set_session_failed_callback(boost::bind(&core::do_handle_session_failed, this, _1, _2, _3, _4));
		m_server->set_session_established_callback(boost::bind(&core::do_handle_session_established, this, _1, _2, _3, _4));
		m_server->set_session_lost_callback(boost::bind(&core::do_handle_session_lost, this, _1));
		m_server->set_data_received_callback(boost::bind(&core::do_handle_data_received, this, _1, _2, _3));

		resolver_type resolver(m_io_service);

		const ep_type listen_endpoint = boost::apply_visitor(
			endpoint_resolve_visitor(
				resolver,
				to_protocol(m_configuration.fscp.hostname_resolution_protocol),
				resolver_query::address_configured | resolver_query::passive, DEFAULT_SERVICE
			),
			m_configuration.fscp.listen_on
		);

		m_logger(LL_INFORMATION) << "Core set to listen on: " << listen_endpoint;

		if (m_configuration.security.certificate_validation_method == security_configuration::CVM_DEFAULT)
		{
			m_ca_store = cryptoplus::x509::store::create();

			BOOST_FOREACH(const cert_type& cert, m_configuration.security.certificate_authority_list)
			{
				m_ca_store.add_certificate(cert);
			}

			BOOST_FOREACH(const crl_type& crl, m_configuration.security.certificate_revocation_list_list)
			{
				m_ca_store.add_certificate_revocation_list(crl);
			}

			switch (m_configuration.security.certificate_revocation_validation_method)
			{
				case security_configuration::CRVM_LAST:
					{
						m_ca_store.set_verification_flags(X509_V_FLAG_CRL_CHECK);
						break;
					}
				case security_configuration::CRVM_ALL:
					{
						m_ca_store.set_verification_flags(X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
						break;
					}
				case security_configuration::CRVM_NONE:
					{
						break;
					}
			}
		}

		BOOST_FOREACH(const ip_network_address& network_address, m_configuration.fscp.never_contact_list)
		{
			m_logger(LL_INFORMATION) << "Configured not to accept requests from: " << network_address;
		}

		// Let's open the server.
		m_server->open(listen_endpoint);

		// We start the contact loop.
		async_contact_all();

		m_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_contact, this, boost::asio::placeholders::error));
		m_dynamic_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_dynamic_contact, this, boost::asio::placeholders::error));
	}

	void core::close_server()
	{
		// Stop the contact loop timers.
		m_dynamic_contact_timer.cancel();
		m_contact_timer.cancel();

		m_server->close();
	}

	void core::async_contact(const endpoint& target, duration_handler_type handler)
	{
		resolve_success_handler_type success_handler = boost::bind(&core::do_contact, this, _1, handler);
		resolve_error_handler_type error_handler = boost::bind(handler, ep_type(), _1, boost::posix_time::time_duration());

		boost::apply_visitor(
			endpoint_async_resolve_visitor(
				boost::make_shared<resolver_type>(boost::ref(m_io_service)),
				to_protocol(m_configuration.fscp.hostname_resolution_protocol),
				resolver_query::address_configured,
				DEFAULT_SERVICE,
				boost::bind(
					&resolve_handler,
					_1,
					_2,
					success_handler,
					error_handler
				)
			),
			target
		);
	}

	void core::async_contact(const endpoint& target)
	{
		async_contact(target, boost::bind(&core::do_handle_contact, this, target, _1, _2, _3));
	}

	void core::async_contact_all()
	{
		BOOST_FOREACH(const endpoint& contact, m_configuration.fscp.contact_list)
		{
			async_contact(contact);
		}
	}

	void core::async_dynamic_contact_all()
	{
		using boost::make_transform_iterator;

		hash_type (*func)(cert_type) = fscp::get_certificate_hash;

		const hash_list_type hash_list(make_transform_iterator(m_configuration.fscp.dynamic_contact_list.begin(), func), make_transform_iterator(m_configuration.fscp.dynamic_contact_list.end(), func));

		async_send_contact_request_to_all(hash_list);
	}

	void core::async_send_contact_request_to_all(const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
	{
		m_server->async_send_contact_request_to_all(hash_list, handler);
	}

	void core::async_send_contact_request_to_all(const hash_list_type& hash_list)
	{
		async_send_contact_request_to_all(hash_list, boost::bind(&core::do_handle_send_contact_request_to_all, this, _1));
	}

	void core::async_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		assert(m_server);

		m_server->async_introduce_to(target, handler);
	}

	void core::async_introduce_to(const ep_type& target)
	{
		async_introduce_to(target, boost::bind(&core::do_handle_introduce_to, this, target, _1));
	}

	void core::async_request_session(const ep_type& target, simple_handler_type handler)
	{
		assert(m_server);

		m_server->async_request_session(target, handler);
	}

	void core::async_request_session(const ep_type& target)
	{
		async_request_session(target, boost::bind(&core::do_handle_request_session, this, target, _1));
	}

	void core::do_contact(const ep_type& address, duration_handler_type handler)
	{
		assert(m_server);

		m_server->async_greet(address, boost::bind(handler, address, _1, _2));
	}

	void core::do_handle_contact(const endpoint& host, const ep_type& address, const boost::system::error_code& ec, const boost::posix_time::time_duration& duration)
	{
		if (!ec)
		{
			m_logger(LL_DEBUG) << "Received HELLO_RESPONSE from " << host << " at " << address << ". Latency: " << duration << "";

			async_introduce_to(address);
		}
		else
		{
			if (ec == fscp::server_error::hello_request_timed_out)
			{
				m_logger(LL_DEBUG) << "Received no HELLO_RESPONSE from " << host << " at " << address << ": " << ec.message() << " (timeout: " << duration << ")";
			}
			else
			{
				m_logger(LL_DEBUG) << "Unable to send HELLO to " << host << ": " << ec.message();
			}
		}
	}

	void core::do_handle_periodic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			async_contact_all();

			m_contact_timer.expires_from_now(CONTACT_PERIOD);
			m_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_handle_periodic_dynamic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			async_dynamic_contact_all();

			m_dynamic_contact_timer.expires_from_now(DYNAMIC_CONTACT_PERIOD);
			m_dynamic_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_dynamic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_handle_send_contact_request(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(LL_WARNING) << "Error sending contact request to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_send_contact_request_to_all(const std::map<ep_type, boost::system::error_code>& results)
	{
		for (std::map<ep_type, boost::system::error_code>::const_iterator result = results.begin(); result != results.end(); ++result)
		{
			do_handle_send_contact_request(result->first, result->second);
		}
	}

	void core::do_handle_introduce_to(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(LL_WARNING) << "Error sending introduction message to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_request_session(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(LL_WARNING) << "Error requesting session to " << target << ": " << ec.message();
		}
	}

	bool core::do_handle_hello_received(const ep_type& sender, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received HELLO_REQUEST from " << sender << ".";

		if (is_banned(sender.address()))
		{
			m_logger(LL_WARNING) << "Ignoring HELLO_REQUEST from " << sender << " as it is a banned host.";

			default_accept = false;
		}

		if (default_accept)
		{
			async_introduce_to(sender);
		}

		return default_accept;
	}

	bool core::do_handle_contact_request_received(const ep_type& sender, cert_type cert, hash_type hash, const ep_type& answer)
	{
		if (m_configuration.fscp.accept_contact_requests)
		{
			m_logger(LL_INFORMATION) << "Received contact request from " << sender << " for " << cert.subject().oneline() << " (" << hash << "). Host is at: " << answer;

			return true;
		}
		else
		{
			return false;
		}
	}

	void core::do_handle_contact_received(const ep_type& sender, hash_type hash, const ep_type& answer)
	{
		if (m_configuration.fscp.accept_contacts)
		{
			// We check if the contact belongs to the forbidden network list.
			if (is_banned(answer.address()))
			{
				m_logger(LL_WARNING) << "Received forbidden contact from " << sender << ": " << hash << " is at " << answer << " but won't be contacted.";
			}
			else
			{
				m_logger(LL_INFORMATION) << "Received contact from " << sender << ": " << hash << " is at: " << answer;

				async_contact(to_endpoint(answer));
			}
		}
	}

	bool core::do_handle_presentation_received(const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool is_new)
	{
		if (m_logger.level() <= LL_DEBUG)
		{
			m_logger(LL_DEBUG) << "Received PRESENTATION from " << sender << ". Signature: " << sig_cert.subject().oneline() << ". Cipherment: " << enc_cert.subject().oneline() << ". New presentation: " << is_new << ".";
		}

		if (is_banned(sender.address()))
		{
			m_logger(LL_WARNING) << "Ignoring PRESENTATION from " << sender << " as it is a banned host.";

			return false;
		}

		if (certificate_is_valid(sig_cert) && certificate_is_valid(enc_cert))
		{
			async_request_session(sender);

			return true;
		}

		return false;
	}

	bool core::do_handle_session_request_received(const ep_type& sender, const fscp::cipher_algorithm_list_type& calg_capabilities, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received SESSION_REQUEST from " << sender << " (default: " << (default_accept ? std::string("accept") : std::string("deny")) << ").";

		if (m_logger.level() <= LL_DEBUG)
		{
			std::ostringstream oss;

			BOOST_FOREACH(const fscp::cipher_algorithm_type& calg, calg_capabilities)
			{
				oss << " " << calg;
			}

			m_logger(LL_DEBUG) << "Cipher algorithm capabilities:" << oss.str();
		}

		return default_accept;
	}

	bool core::do_handle_session_received(const ep_type& sender, fscp::cipher_algorithm_type calg, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received SESSION from " << sender << " (default: " << (default_accept ? std::string("accept") : std::string("deny")) << ").";
		m_logger(LL_DEBUG) << "Cipher algorithm: " << calg;

		return default_accept;
	}

	void core::do_handle_session_failed(const ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
	{
		if (is_new)
		{
			m_logger(LL_WARNING) << "Session establishment with " << host << " failed.";
		}
		else
		{
			m_logger(LL_WARNING) << "Session renewal with " << host << " failed.";
		}

		m_logger(LL_WARNING) << "Local algorithms: " << local;
		m_logger(LL_WARNING) << "Remote algorithms: " << remote;

		//TODO: Handle callback
		//m_session_failed_callback(host, is_new, local, remote);
	}

	void core::do_handle_session_established(const ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
	{
		if (is_new)
		{
			m_logger(LL_INFORMATION) << "Session established with " << host << ".";
		}
		else
		{
			m_logger(LL_INFORMATION) << "Session renewed with " << host << ".";
		}

		m_logger(LL_INFORMATION) << "Local algorithms: " << local;
		m_logger(LL_INFORMATION) << "Remote algorithms: " << remote;

		if (is_new)
		{
			if (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TAP)
			{
				//TODO: Make sure the buffer remains available and unmodified until the callback (which must be changed) gets called.
				//const switch_::port_type port = boost::make_shared<endpoint_switch_port>(host, boost::bind(&fscp::server::async_send_data, &*m_server, _1, fscp::CHANNEL_NUMBER_0, _2, fscp::server::write_callback()));

				//m_endpoint_switch_port_map[host] = port;
				//m_switch.register_port(port, ENDPOINTS_GROUP);
			}
			else
			{
				//TODO: Get the routes somewhere...
				//routes_type local_routes;

				//TODO: Make sure the buffer remains available and unmodified until the callback (which must be changed) gets called.
				//const router::port_type port = boost::make_shared<endpoint_router_port>(host, local_routes, boost::bind(&fscp::server::async_send_data, &*m_server, _1, fscp::CHANNEL_NUMBER_0, _2, fscp::server::write_callback()));

				//m_endpoint_router_port_map[host] = port;
				//m_router.register_port(port, ENDPOINTS_GROUP);
			}
		}

		//TODO: Handle callbacks
		//m_session_established_callback(host, is_new, local, remote);
	}

	void core::do_handle_session_lost(const ep_type& host)
	{
		m_logger(LL_INFORMATION) << "Session with " << host << " lost.";

		//TODO: Handle session lost callback
		//m_session_lost_callback(host);

		if (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TAP)
		{
			//TODO: Implement
			//const switch_::port_type port = m_endpoint_switch_port_map[sender];

			//if (port)
			//{
			//	m_switch.unregister_port(port);
			//	m_endpoint_switch_port_map.erase(sender);
			//}
		}
		else
		{
			//TODO: Implement
			//const router::port_type port = m_endpoint_router_port_map[sender];

			//if (port)
			//{
			//	m_router.unregister_port(port);
			//	m_endpoint_router_port_map.erase(sender);
			//}
		}
	}

	void core::do_handle_data_received(const ep_type& sender, fscp::channel_number_type channel_number, boost::asio::const_buffer data)
	{
		switch (channel_number)
		{
			// Channel 0 contains ethernet/ip frames
			case fscp::CHANNEL_NUMBER_0:
				if (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TAP)
				{
					//TODO: Uncomment
					//on_ethernet_data(sender, data);
				}
				else
				{
					//TODO: Uncomment
					//on_ip_data(sender, data);
				}

				break;
			// Channel 1 contains messages
			case fscp::CHANNEL_NUMBER_1:
				try
				{
					//TODO: Uncomment
					//const message msg(boost::asio::buffer_cast<const uint8_t*>(data), boost::asio::buffer_size(data));

					//on_message(sender, msg);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Received incorrectly formatted message from " << sender << ". Error was: " << ex.what();
				}

				break;
			default:
				m_logger(LL_WARNING) << "Received unhandled " << buffer_size(data) << " byte(s) of data on FSCP channel #" << static_cast<int>(channel_number);
				break;
		}
	}

	int core::certificate_validation_callback(int ok, X509_STORE_CTX* ctx)
	{
		cryptoplus::x509::store_context store_context(ctx);

		core* _this = static_cast<core*>(store_context.get_external_data(core::ex_data_index));

		return (_this->certificate_validation_method(ok != 0, store_context)) ? 1 : 0;
	}

	bool core::certificate_validation_method(bool ok, cryptoplus::x509::store_context store_context)
	{
		cert_type cert = store_context.get_current_certificate();

		if (!ok)
		{
			m_logger(LL_WARNING) << "Error when validating " << cert.subject().oneline() << ": " << store_context.get_error_string() << " (depth: " << store_context.get_error_depth() << ")";
		}
		else
		{
			m_logger(LL_INFORMATION) << cert.subject().oneline() << " is valid.";
		}

		return ok;
	}

	bool core::certificate_is_valid(cert_type cert)
	{
		switch (m_configuration.security.certificate_validation_method)
		{
			case security_configuration::CVM_DEFAULT:
				{
					using namespace cryptoplus;

					// We can't easily ensure m_ca_store is used only in one strand, so we protect it with a mutex instead.
					boost::mutex::scoped_lock lock(m_ca_store_mutex);

					// Create a store context to proceed to verification
					x509::store_context store_context = x509::store_context::create();

					store_context.initialize(m_ca_store, cert, NULL);

					// Ensure to set the verification callback *AFTER* you called initialize or it will be ignored.
					store_context.set_verification_callback(&core::certificate_validation_callback);

					// Add a reference to the current instance into the store context.
					store_context.set_external_data(core::ex_data_index, this);

					if (!store_context.verify())
					{
						return false;
					}

					break;
				}
			case security_configuration::CVM_NONE:
				{
					break;
				}
		}

		if (m_configuration.security.certificate_validation_callback)
		{
			return m_configuration.security.certificate_validation_callback(*this, cert);
		}

		return true;
	}

	void core::open_tap_adapter()
	{
		if (m_configuration.tap_adapter.enabled)
		{
			const asiotap::tap_adapter::adapter_type tap_adapter_type = (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TAP) ? asiotap::tap_adapter::AT_TAP_ADAPTER : asiotap::tap_adapter::AT_TUN_ADAPTER;

			m_tap_adapter = boost::make_shared<asiotap::tap_adapter>(boost::ref(m_io_service));

			if (tap_adapter_type == asiotap::tap_adapter::AT_TAP_ADAPTER)
			{
				// Registers the switch port.
				m_tap_adapter_switch_port = boost::make_shared<tap_adapter_switch_port>(boost::ref(*m_tap_adapter));
				m_switch.register_port(m_tap_adapter_switch_port, TAP_ADAPTERS_GROUP);
			}
			else
			{
				// Registers the router port.
				const routes_type& local_routes = m_configuration.router.local_ip_routes;

				m_tap_adapter_router_port = boost::make_shared<tap_adapter_router_port>(boost::ref(*m_tap_adapter), local_routes);
				m_router.register_port(m_tap_adapter_router_port, TAP_ADAPTERS_GROUP);
			}

			m_tap_adapter->open(m_configuration.tap_adapter.name, compute_mtu(m_configuration.tap_adapter.mtu, get_auto_mtu_value()), tap_adapter_type);

			m_logger(LL_INFORMATION) << "Tap adapter \"" << m_tap_adapter->name() << "\" opened in mode " << m_configuration.tap_adapter.type << " with a MTU set to: " << m_tap_adapter->mtu();

			// IPv4 address
			if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
			{
				try
				{
#ifdef WINDOWS
					// Quick fix for Windows:
					// Directly setting the IPv4 address/prefix length doesn't work like it should on Windows.
					// We disable direct setting if DHCP is enabled.

					if ((m_configuration.tap_adapter.type != tap_adapter_configuration::TAT_TAP) || !m_configuration.tap_adapter.dhcp_proxy_enabled)
					{
						m_tap_adapter->add_ip_address_v4(
						    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
						    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
						);
					}
#else
					m_tap_adapter->add_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
					);
#endif
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot set IPv4 address: " << ex.what();
				}
			}

			// IPv6 address
			if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->add_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv6_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot set IPv6 address: " << ex.what();
				}
			}

			if (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TUN)
			{
				if (m_configuration.tap_adapter.remote_ipv4_address)
				{
					m_tap_adapter->set_remote_ip_address_v4(m_configuration.tap_adapter.ipv4_address_prefix_length.address(), *m_configuration.tap_adapter.remote_ipv4_address);
				}
			}

			m_tap_adapter->set_connected_state(true);

			if (m_configuration.tap_adapter.type == tap_adapter_configuration::TAT_TAP)
			{
				// The ARP proxy
				if (m_configuration.tap_adapter.arp_proxy_enabled)
				{
					m_arp_proxy.reset(new arp_proxy_type());
					m_arp_proxy->set_arp_request_callback(boost::bind(&core::do_handle_arp_request, this, _1, _2));
				}
				else
				{
					m_arp_proxy.reset();
				}

				// The DHCP proxy
				if (m_configuration.tap_adapter.dhcp_proxy_enabled)
				{
					m_dhcp_proxy.reset(new dhcp_proxy_type());
					m_dhcp_proxy->set_hardware_address(m_tap_adapter->ethernet_address());

					if (!m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.is_null())
					{
						m_dhcp_proxy->set_software_address(m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.address());
					}

					if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
					{
						m_dhcp_proxy->add_entry(
								m_tap_adapter->ethernet_address(),
								m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
								m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
						);
					}
				}
				else
				{
					m_dhcp_proxy.reset();
				}
			}
			else
			{
				// We don't need any proxies in TUN mode.
				m_arp_proxy.reset();
				m_dhcp_proxy.reset();
			}

			m_configuration.tap_adapter.up_callback(*this, *m_tap_adapter);

			async_read_tap();
		}
		else
		{
			m_tap_adapter.reset();
		}
	}

	void core::close_tap_adapter()
	{
		m_dhcp_proxy.reset();
		m_arp_proxy.reset();

		if (m_tap_adapter)
		{
			//TODO: Handle tap_adapter_down callback
			//m_configuration.tap_adapter.down_callback(*this, *m_tap_adapter);

			m_switch.unregister_port(m_tap_adapter_switch_port);
			m_router.unregister_port(m_tap_adapter_router_port);

			m_tap_adapter->cancel();
			m_tap_adapter->set_connected_state(false);

			// IPv6 address
			if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->remove_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv6_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv6 address: " << ex.what();
				}
			}

			// IPv4 address
			if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->remove_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv4 address: " << ex.what();
				}
			}

			m_tap_adapter->close();
		}
	}

	void core::async_read_tap()
	{
		m_tap_adapter_strand.post(boost::bind(&core::do_read_tap, this));
	}

	void core::do_read_tap()
	{
		// All calls to do_read_tap() are done within the m_tap_adapter_strand, so the following is safe.
		assert(m_tap_adapter);

		const tap_adapter_memory_pool::shared_buffer_type receive_buffer = m_tap_adapter_memory_pool.allocate_shared_buffer();

		m_tap_adapter->async_read(
			buffer(receive_buffer),
			m_proxies_strand.wrap(
				boost::bind(
					&core::do_handle_tap_adapter_read,
					this,
					receive_buffer,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			)
		);
	}

	void core::do_handle_tap_adapter_read(tap_adapter_memory_pool::shared_buffer_type receive_buffer, const boost::system::error_code& ec, size_t count)
	{
		// All calls to do_read_tap() are done within the m_proxies_strand, so the following is safe.
		if (ec != boost::asio::error::operation_aborted)
		{
			// We try to read again, as soon as possible.
			async_read_tap();
		}

		if (!ec)
		{
			const boost::asio::const_buffer data = buffer(receive_buffer, count);

			if (m_tap_adapter->type() == asiotap::tap_adapter::AT_TAP_ADAPTER)
			{
				bool handled = false;

				if (m_arp_proxy || m_dhcp_proxy)
				{
					// This line will eventually call the filters callbacks.
					m_ethernet_filter.parse(data);

					if (m_arp_proxy && m_arp_filter.get_last_helper())
					{
						handled = true;
						m_arp_filter.clear_last_helper();
					}

					if (m_dhcp_proxy && m_dhcp_filter.get_last_helper())
					{
						handled = true;
						m_dhcp_filter.clear_last_helper();
					}
				}

				if (!handled)
				{
					m_switch.receive_data(m_tap_adapter_switch_port, data);
				}
			}
			else
			{
				// This is a TUN interface. We receive either IPv4 or IPv6 frames.
				m_router.receive_data(m_tap_adapter_router_port, data);
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			m_logger(LL_ERROR) << "Read failed on " << m_tap_adapter->name() << ". Error: " << ec.message();
		}
	}

	void core::do_handle_tap_adapter_write(const boost::system::error_code& ec, size_t count)
	{
		static_cast<void>(count);

		if (ec)
		{
			if (ec != boost::asio::error::operation_aborted)
			{
				m_logger(LL_WARNING) << "Write failed on " << m_tap_adapter->name() << ". Error: " << ec.message();
			}
		}
	}

	void core::do_handle_arp_frame(const arp_helper_type& helper)
	{
		if (m_arp_proxy)
		{
			const proxy_memory_pool::shared_buffer_type response_buffer = m_proxy_memory_pool.allocate_shared_buffer();

			const boost::optional<boost::asio::const_buffer> data = m_arp_proxy->process_frame(
				*m_arp_filter.parent().get_last_helper(),
				helper,
				buffer(response_buffer)
			);

			if (data)
			{
				async_write_tap(
					*data,
					make_shared_buffer_handler(
						response_buffer,
						boost::bind(
							&core::do_handle_tap_adapter_write,
							this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred
						)
					)
				);
			}
		}
	}

	void core::do_handle_dhcp_frame(const dhcp_helper_type& helper)
	{
		if (m_dhcp_proxy)
		{
			const proxy_memory_pool::shared_buffer_type response_buffer = m_proxy_memory_pool.allocate_shared_buffer();

			const boost::optional<boost::asio::const_buffer> data = m_dhcp_proxy->process_frame(
				*m_dhcp_filter.parent().parent().parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().get_last_helper(),
				helper,
				buffer(response_buffer)
			);

			if (data)
			{
				async_write_tap(
					*data,
					make_shared_buffer_handler(
						response_buffer,
						boost::bind(
							&core::do_handle_tap_adapter_write,
							this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred
						)
					)
				);
			}
		}
	}

	bool core::do_handle_arp_request(const boost::asio::ip::address_v4& logical_address, ethernet_address_type& ethernet_address)
	{
		if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
		{
			if (logical_address != m_configuration.tap_adapter.ipv4_address_prefix_length.address())
			{
				ethernet_address = m_configuration.tap_adapter.arp_proxy_fake_ethernet_address;

				return true;
			}
		}

		return false;
	}
}

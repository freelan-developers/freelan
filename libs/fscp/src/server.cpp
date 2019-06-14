/*
 * libfscp - A C++ library to establish peer-to-peer virtual private networks.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file server.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#include "server.hpp"

#include "server_error.hpp"

#include "message.hpp"
#include "hello_message.hpp"
#include "presentation_message.hpp"
#include "session_request_message.hpp"
#include "session_message.hpp"
#include "data_message.hpp"

// Mac OSX version of Boost has shadowing warnings that fail the build.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/random.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/thread/future.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <cassert>

namespace fscp
{
	using boost::asio::buffer;
	using boost::asio::buffer_cast;
	using boost::asio::buffer_size;

	namespace
	{
		void null_simple_handler(const boost::system::error_code&) {}
		void null_multiple_endpoints_handler(const std::map<server::ep_type, boost::system::error_code>&) {}

		server::ep_type normalize(const server::ep_type& ep)
		{
			server::ep_type result = ep;

			// If the endpoint is an IPv4 mapped address, return a real IPv4 address
			if (result.address().is_v6())
			{
				boost::asio::ip::address_v6 address = result.address().to_v6();

				if (address.is_v4_mapped())
				{
					result = server::ep_type(address.to_v4(), result.port());
				}
			}

			return result;
		}

		template <typename Handler, typename CausalHandler>
		class causal_handler
		{
			private:

				class automatic_caller : public boost::noncopyable
				{
					public:

						automatic_caller(CausalHandler& _handler) :
							m_auto_handler(_handler)
						{
						}

						~automatic_caller()
						{
							m_auto_handler();
						}

					private:

						CausalHandler& m_auto_handler;
				};

			public:

				typedef void result_type;

				causal_handler(Handler _handler, CausalHandler _causal_handler) :
					m_handler(_handler),
					m_causal_handler(_causal_handler)
				{}

				result_type operator()()
				{
					automatic_caller ac(m_causal_handler);

					m_handler();
				}

				template <typename Arg1>
				result_type operator()(Arg1 arg1)
				{
					automatic_caller ac(m_causal_handler);

					m_handler(arg1);
				}

				template <typename Arg1, typename Arg2>
				result_type operator()(Arg1 arg1, Arg2 arg2)
				{
					automatic_caller ac(m_causal_handler);

					m_handler(arg1, arg2);
				}

			private:

				Handler m_handler;
				CausalHandler m_causal_handler;
		};

		template <typename Handler, typename CausalHandler>
		inline causal_handler<Handler, CausalHandler> make_causal_handler(Handler _handler, CausalHandler _causal_handler)
		{
			return causal_handler<Handler, CausalHandler>(_handler, _causal_handler);
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
				{
					if (m_keys.empty())
					{
						m_handler(m_results);
					}
				}

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

		bool compare_certificates(const server::cert_type& lhs, const server::cert_type& rhs)
		{
			if (!!lhs && !!rhs)
			{
				return (lhs.write_der() == rhs.write_der());
			}
			else
			{
				return (!lhs && !rhs);
			}
		}
	}

	// Public methods

	server::server(boost::asio::io_service& io_service, fscp::logger& _logger, const identity_store& identity) :
		m_logger(_logger),
		m_identity_store(identity),
		m_socket(io_service),
		m_socket_strand(io_service),
		m_write_queue_strand(io_service),
		m_greet_strand(io_service),
		m_accept_hello_messages_default(true),
		m_hello_message_received_handler(),
		m_hello_limit_timer(io_service, boost::posix_time::seconds(10)),
		m_hello_max_per_second(1),
		m_presentation_strand(io_service),
		m_presentation_message_received_handler(),
		m_presentation_limit_timer(io_service, boost::posix_time::seconds(10)),
		m_presentation_max_per_second(1),
		m_session_strand(io_service),
		m_accept_session_request_messages_default(true),
		m_cipher_suites(get_default_cipher_suites()),
		m_elliptic_curves(get_supported_elliptic_curves(get_default_elliptic_curves())),
		m_session_request_message_received_handler(),
		m_accept_session_messages_default(true),
		m_session_message_received_handler(),
		m_session_failed_handler(),
		m_session_error_handler(),
		m_session_established_handler(),
		m_session_lost_handler(),
		m_contact_strand(io_service),
		m_data_received_handler(),
		m_contact_request_message_received_handler(),
		m_contact_message_received_handler(),
		m_keep_alive_timer(io_service, SESSION_KEEP_ALIVE_PERIOD)
	{
		// These calls are needed in C++03 to ensure that static initializations are done in a single thread.
		server_category();
	}

	elliptic_curve_list_type server::get_supported_elliptic_curves(
			const elliptic_curve_list_type& curves)
	{
		elliptic_curve_list_type ret;

		for (auto&& ec : curves)
		{
			try
			{
				cryptoplus::pkey::ecdhe_context(ec.to_elliptic_curve_nid())
					.get_public_key();
				ret.push_back(ec);
			}
			catch(boost::system::system_error&)
			{
				m_logger(log_level::warning) << "Elliptic curve not supported: "
					<< ec.to_string();
			}
		}
		return ret;
	}

	identity_store server::sync_get_identity()
	{
		typedef boost::promise<identity_store> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const identity_store&) = &promise_type::set_value;

		async_get_identity(boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::sync_set_identity(const identity_store& identity)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_identity(identity, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::open(const ep_type& listen_endpoint)
	{
		m_socket.open(listen_endpoint.protocol());

		if (listen_endpoint.address().is_v6())
		{
			// We accept both IPv4 and IPv6 addresses
			m_socket.set_option(boost::asio::ip::v6_only(false));
		}

		m_socket.bind(listen_endpoint);

		async_receive_from();

		m_keep_alive_timer.async_wait(m_session_strand.wrap(boost::bind(&server::do_check_keep_alive, this, boost::asio::placeholders::error)));
		m_hello_limit_timer.async_wait(m_greet_strand.wrap(
					boost::bind(&server::do_hello_reset_limit, this,
						boost::asio::placeholders::error)));
		m_presentation_limit_timer.async_wait(m_presentation_strand.wrap(
					boost::bind(&server::do_presentation_reset_limit, this,
						boost::asio::placeholders::error)));
	}

	void server::close()
	{
		cancel_all_greetings();

		m_keep_alive_timer.cancel();

		m_hello_limit_timer.cancel();
		m_presentation_limit_timer.cancel();

		m_socket.close();
	}

#ifdef USE_UPNP
	void server::upnp_punch_hole(uint16_t port)
	{
		try
		{
			std::ostringstream oss;
			oss << port;

			if(m_upnp.get() == nullptr)
			{
				m_logger(log_level::information) << "Discovering UPnP IGD gateways.";
				m_upnp.reset(new miniupnpcplus::upnp_device(2000));

				// same external port as local port
				m_logger(log_level::trace) << "Try to register UPnP port mapping: " <<
					m_upnp->get_external_ip() << ":" << port << " -> " << m_upnp->get_lan_ip() <<
					":" << port;
				m_upnp->register_port_mapping(miniupnpcplus::UDP, oss.str(), oss.str(),
						"FreeLAN peer");
				m_logger(log_level::information) << "UPnP port mapping registered: " <<
					m_upnp->get_external_ip() << ":" << port << " -> " << m_upnp->get_lan_ip() <<
					":" << port;

				// TODO retry if port is already taken
			}
		}
		catch (const boost::system::system_error& ex)
		{
			m_logger(log_level::error) << "UPnP discovery/port mapping failed: " << ex.what();
		}
	}
#endif

	void server::async_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout)
	{
		m_greet_strand.post(boost::bind(&server::do_greet, this, normalize(target), handler, timeout));
	}

	void server::sync_set_accept_hello_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_hello_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_hello_message_received_callback(hello_message_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_hello_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::async_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		m_socket_strand.post(boost::bind(&server::do_introduce_to, this, normalize(target), handler));
	}

	boost::system::error_code server::sync_introduce_to(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_introduce_to(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_reintroduce_to_all(multiple_endpoints_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server::do_reintroduce_to_all, this, handler));
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_reintroduce_to_all()
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_reintroduce_to_all(boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	boost::optional<presentation_store> server::get_presentation(const ep_type& target)
	{
		const presentation_store_map::const_iterator item = m_presentation_store_map.find(target);

		if (item != m_presentation_store_map.end())
		{
			return boost::optional<presentation_store>(item->second);
		}
		else
		{
			return boost::optional<presentation_store>();
		}
	}

	void server::async_get_presentation(const ep_type& target, optional_presentation_store_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server::do_get_presentation, this, normalize(target), handler));
	}

	boost::optional<presentation_store> server::sync_get_presentation(const ep_type& target)
	{
		typedef boost::promise<boost::optional<presentation_store> > promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::optional<presentation_store>&) = &promise_type::set_value;

		async_get_presentation(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key)
	{
		m_presentation_store_map[target] = presentation_store(signature_certificate, pre_shared_key);
	}

	void server::async_set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key, void_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server::do_set_presentation, this, normalize(target), signature_certificate, pre_shared_key, handler));
	}

	void server::sync_set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_presentation(target, signature_certificate, pre_shared_key, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::clear_presentation(const ep_type& target)
	{
		m_presentation_store_map.erase(target);
	}

	void server::async_clear_presentation(const ep_type& target, void_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server::do_clear_presentation, this, normalize(target), handler));
	}

	void server::sync_clear_presentation(const ep_type& target)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_clear_presentation(target, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_presentation_message_received_callback(presentation_message_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_presentation_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::async_request_session(const ep_type& target, simple_handler_type handler)
	{
		async_get_identity(m_session_strand.wrap(boost::bind(&server::do_request_session, this, _1, normalize(target), handler)));
	}

	void server::async_close_session(const ep_type& target, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server::do_close_session, this, normalize(target), handler));
	}

	boost::system::error_code server::sync_close_session(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_close_session(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	std::set<server::ep_type> server::sync_get_session_endpoints()
	{
		typedef std::set<ep_type> result_type;
		typedef boost::promise<result_type> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_get_session_endpoints(boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	bool server::sync_has_session_with_endpoint(const ep_type& host)
	{
		typedef bool result_type;
		typedef boost::promise<result_type> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_has_session_with_endpoint(host, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	boost::system::error_code server::sync_request_session(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_request_session(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::sync_set_accept_session_request_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_session_request_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_cipher_suites(const cipher_suite_list_type& cipher_suites)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_cipher_suites(cipher_suites, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_elliptic_curves(const elliptic_curve_list_type& elliptic_curves)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_elliptic_curves(elliptic_curves, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_request_message_received_callback(session_request_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_request_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_accept_session_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_session_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_message_received_callback(session_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_failed_callback(session_failed_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_failed_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_error_callback(session_error_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_error_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_established_callback(session_established_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_established_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_session_lost_callback(session_lost_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_lost_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::async_send_data(const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server::do_send_data, this, normalize(target), channel_number, data, handler));
	}

	boost::system::error_code server::sync_send_data(const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_send_data(target, channel_number, data, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_send_data_to_list(const std::set<ep_type>& targets, channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler)
	{
		const std::set<ep_type> normalized_targets(boost::make_transform_iterator(targets.begin(), normalize), boost::make_transform_iterator(targets.end(), normalize));

		m_session_strand.post(boost::bind(&server::do_send_data_to_list, this, normalized_targets, channel_number, data, handler));
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_data_to_list(const std::set<ep_type>& targets, channel_number_type channel_number, boost::asio::const_buffer data)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_data_to_list(targets, channel_number, data, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_data_to_all(channel_number_type channel_number, boost::asio::const_buffer data)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_data_to_all(channel_number, data, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_send_contact_request(const ep_type& target, const hash_list_type& hash_list, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server::do_send_contact_request, this, normalize(target), hash_list, handler));
	}

	boost::system::error_code server::sync_send_contact_request(const ep_type& target, const hash_list_type& hash_list)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_send_contact_request(target, hash_list, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_send_contact_request_to_list(const std::set<ep_type>& targets, const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
	{
		const std::set<ep_type> normalized_targets(boost::make_transform_iterator(targets.begin(), normalize), boost::make_transform_iterator(targets.end(), normalize));

		m_session_strand.post(boost::bind(&server::do_send_contact_request_to_list, this, normalized_targets, hash_list, handler));
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_contact_request_to_list(const std::set<ep_type>& targets, const hash_list_type& hash_list)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_contact_request_to_list(targets, hash_list, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_contact_request_to_all(const hash_list_type& hash_list)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_contact_request_to_all(hash_list, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_send_contact(const ep_type& target, const contact_map_type& contact_map, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server::do_send_contact, this, normalize(target), contact_map, handler));
	}

	boost::system::error_code server::sync_send_contact(const ep_type& target, const contact_map_type& contact_map)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_send_contact(target, contact_map, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::async_send_contact_to_list(const std::set<ep_type>& targets, const contact_map_type& contact_map, multiple_endpoints_handler_type handler)
	{
		const std::set<ep_type> normalized_targets(boost::make_transform_iterator(targets.begin(), normalize), boost::make_transform_iterator(targets.end(), normalize));

		m_session_strand.post(boost::bind(&server::do_send_contact_to_list, this, normalized_targets, contact_map, handler));
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_contact_to_list(const std::set<ep_type>& targets, const contact_map_type& contact_map)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_contact_to_list(targets, contact_map, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	std::map<server::ep_type, boost::system::error_code> server::sync_send_contact_to_all(const contact_map_type& contact_map)
	{
		typedef std::map<server::ep_type, boost::system::error_code> result_type;
		typedef boost::promise<result_type> promise_type;

		promise_type promise;

		void (promise_type::*setter)(const result_type&) = &promise_type::set_value;

		async_send_contact_to_all(contact_map, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server::sync_set_data_received_callback(data_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_data_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_contact_request_received_callback(contact_request_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_contact_request_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server::sync_set_contact_received_callback(contact_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_contact_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	// Private methods

	void server::do_get_identity(identity_handler_type handler)
	{
		// do_set_identity() is executed within the socket strand so this is safe.

		handler(get_identity());
	}

	void server::do_set_identity(const identity_store& identity, void_handler_type handler)
	{
		// do_set_identity() is executed within the socket strand so this is safe.
		set_identity(identity);

		async_reintroduce_to_all(&null_multiple_endpoints_handler);

		if (handler)
		{
			handler();
		}
	}

	void server::do_async_receive_from()
	{
		// do_async_receive_from() is executed within the socket strand so this is safe.
		boost::shared_ptr<ep_type> sender = boost::make_shared<ep_type>();

		// Get either a new buffer or an old, recycled one if possible.
		const SharedBuffer receive_buffer = m_socket_buffers.empty() ? SharedBuffer(65536) : [this]() {
			const auto result = m_socket_buffers.front();
			m_socket_buffers.pop_front();

			return result;
		}();

		m_socket.async_receive_from(
			buffer(receive_buffer),
			*sender,
			boost::bind(
				&server::handle_receive_from,
				this,
				get_identity(),
				sender,
				/*
				SharedBuffer(receive_buffer, [this](const SharedBuffer& buffer) {
					m_socket_strand.post([this, buffer]() {
						m_socket_buffers.push_back(buffer);
					});
				}),
				*/
				receive_buffer,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	void server::handle_receive_from(const identity_store& identity, boost::shared_ptr<ep_type> sender, SharedBuffer data, const boost::system::error_code& ec, size_t bytes_received)
	{
		assert(sender);

		if (ec != boost::asio::error::operation_aborted)
		{
			// Let's read again !
			async_receive_from();

			*sender = normalize(*sender);

			if (!ec)
			{
				try
				{
					message message(buffer_cast<const uint8_t*>(data), bytes_received);

					switch (message.type())
					{
						case MESSAGE_TYPE_DATA_0:
						case MESSAGE_TYPE_DATA_1:
						case MESSAGE_TYPE_DATA_2:
						case MESSAGE_TYPE_DATA_3:
						case MESSAGE_TYPE_DATA_4:
						case MESSAGE_TYPE_DATA_5:
						case MESSAGE_TYPE_DATA_6:
						case MESSAGE_TYPE_DATA_7:
						case MESSAGE_TYPE_DATA_8:
						case MESSAGE_TYPE_DATA_9:
						case MESSAGE_TYPE_DATA_10:
						case MESSAGE_TYPE_DATA_11:
						case MESSAGE_TYPE_DATA_12:
						case MESSAGE_TYPE_DATA_13:
						case MESSAGE_TYPE_DATA_14:
						case MESSAGE_TYPE_DATA_15:
						case MESSAGE_TYPE_CONTACT_REQUEST:
						case MESSAGE_TYPE_CONTACT:
						case MESSAGE_TYPE_KEEP_ALIVE:
						{
							data_message data_message(message);

							m_session_strand.post(
								make_shared_buffer_handler(
									data,
									boost::bind(
										&server::do_handle_data,
										this,
										identity,
										*sender,
										data_message
									)
								)
							);

							break;
						}
						case MESSAGE_TYPE_HELLO_REQUEST:
						case MESSAGE_TYPE_HELLO_RESPONSE:
						{
							hello_message hello_message(message);

							handle_hello_message_from(hello_message, *sender);

							break;
						}
						case MESSAGE_TYPE_PRESENTATION:
						{
							presentation_message presentation_message(message);

							handle_presentation_message_from(identity, presentation_message, *sender);

							break;
						}
						case MESSAGE_TYPE_SESSION_REQUEST:
						{
							session_request_message session_request_message(message);

							m_presentation_strand.post(
								boost::bind(
									&server::do_handle_session_request,
									this,
									data,
									identity,
									*sender,
									session_request_message
								)
							);

							break;
						}
						case MESSAGE_TYPE_SESSION:
						{
							session_message session_message(message);

							m_presentation_strand.post(
								boost::bind(
									&server::do_handle_session,
									this,
									data,
									identity,
									*sender,
									session_message
								)
							);

							break;
						}
						default:
						{
							break;
						}
					}
				}
				catch (std::runtime_error&)
				{
					// These errors can happen in normal situations (for instance when a crypto operation fails due to invalid input).
				}
			}
			else if (ec == boost::asio::error::connection_refused)
			{
				// The host refused the connection, meaning it closed its socket so we can force-terminate the session.
				async_close_session(*sender, &null_simple_handler);
			}
		}
	}

	void server::push_write(void_handler_type handler)
	{
		// All push_write() calls are done in the same strand so the following is thread-safe.
		if (m_write_queue.empty())
		{
			// Nothing is being written, lets start the write immediately.
			m_socket_strand.post(make_causal_handler(handler, m_write_queue_strand.wrap(boost::bind(&server::pop_write, this))));
		}

		m_write_queue.push(handler);
	}

	void server::pop_write()
	{
		// All pop_write() calls are done in the same strand so the following is thread-safe.
		m_write_queue.pop();

		if (!m_write_queue.empty())
		{
			m_socket_strand.post(make_causal_handler(m_write_queue.front(), m_write_queue_strand.wrap(boost::bind(&server::pop_write, this))));
		}
	}

	server::ep_type server::to_socket_format(const server::ep_type& ep)
	{
#if defined(WINDOWS) || defined(MACINTOSH)
		if (m_socket.local_endpoint().address().is_v6() && ep.address().is_v4())
		{
			return server::ep_type(boost::asio::ip::address_v6::v4_mapped(ep.address().to_v4()), ep.port());
		}
		else
		{
			return ep;
		}
#else
		return ep;
#endif
	}

	uint32_t server::ep_hello_context_type::generate_unique_number()
	{
		// The first call to this function is *NOT* thread-safe in C++03 !
		static boost::mt19937 rng(static_cast<uint32_t>(time(0)));

		return rng();
	}

	server::ep_hello_context_type::ep_hello_context_type() :
		m_current_hello_unique_number(generate_unique_number())
	{
	}

	uint32_t server::ep_hello_context_type::next_hello_unique_number()
	{
		return m_current_hello_unique_number++;
	}

	template <typename WaitHandler>
	void server::ep_hello_context_type::async_wait_reply(boost::asio::io_service& io_service, uint32_t hello_unique_number, const boost::posix_time::time_duration& timeout, WaitHandler handler)
	{
		const boost::shared_ptr<boost::asio::deadline_timer> timer = boost::make_shared<boost::asio::deadline_timer>(io_service, timeout);

		m_pending_requests[hello_unique_number] = pending_request_status(timer);

		timer->async_wait(handler);
	}

	bool server::ep_hello_context_type::cancel_reply_wait(uint32_t hello_unique_number, bool success)
	{
		pending_requests_map::iterator request = m_pending_requests.find(hello_unique_number);

		if (request != m_pending_requests.end())
		{
			if (request->second.timer->cancel() > 0)
			{
				// At least one handler was cancelled which means we can set the success flag.
				request->second.success = success;

				return true;
			}
		}

		return false;
	}

	void server::ep_hello_context_type::cancel_all_reply_wait()
	{
		for (pending_requests_map::iterator request = m_pending_requests.begin(); request != m_pending_requests.end(); ++request)
		{
			if (request->second.timer->cancel() > 0)
			{
				// At least one handler was cancelled which means we can set the success flag.
				request->second.success = false;
			}
		}
	}

	bool server::ep_hello_context_type::remove_reply_wait(uint32_t hello_unique_number, boost::posix_time::time_duration& duration)
	{
		pending_requests_map::iterator request = m_pending_requests.find(hello_unique_number);

		assert(request != m_pending_requests.end());

		const bool result = request->second.success;

		duration = boost::posix_time::microsec_clock::universal_time() - request->second.start_date;

		m_pending_requests.erase(request);

		return result;
	}

	void server::do_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout)
	{
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline, boost::posix_time::time_duration());

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		const uint32_t hello_unique_number = ep_hello_context.next_hello_unique_number();
		const auto send_buffer = SharedBuffer(16);
		const size_t size = hello_message::write_request(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), hello_unique_number);

		async_send_to(
			send_buffer,
			size,
			target,
			m_greet_strand.wrap([this, target, hello_unique_number, handler, timeout](const boost::system::error_code& ec) {
				do_greet_handler(target, hello_unique_number, handler, timeout, ec);
			})
		);
	}

	void server::do_greet_handler(const ep_type& target, uint32_t hello_unique_number, duration_handler_type handler, const boost::posix_time::time_duration& timeout, const boost::system::error_code& ec)
	{
		if (ec)
		{
			handler(ec, boost::posix_time::time_duration());

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		ep_hello_context.async_wait_reply(get_io_service(), hello_unique_number, timeout, m_greet_strand.wrap(boost::bind(&server::do_greet_timeout, this, target, hello_unique_number, handler, _1)));
	}

	void server::do_greet_timeout(const ep_type& target, uint32_t hello_unique_number, duration_handler_type handler, const boost::system::error_code& ec)
	{
		// All do_greet_timeout() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		boost::posix_time::time_duration duration;

		const bool success = ep_hello_context.remove_reply_wait(hello_unique_number, duration);

		if (ec == boost::asio::error::operation_aborted)
		{
			// The timer was aborted, which means we received a reply or the server was shut down.
			if (success)
			{
				// The success flag is set: the timer was cancelled due to a reply.
				handler(server_error::success, duration);

				return;
			}
		}
		else if (!ec)
		{
			// The timer timed out: replacing the error code.
			handler(server_error::hello_request_timed_out, duration);

			return;
		}

		handler(ec, duration);
	}

	void server::do_cancel_all_greetings()
	{
		// All do_cancel_all_greetings() calls are done in the same strand so the following is thread-safe.
		for (ep_hello_context_map::iterator hello_context = m_ep_hello_contexts.begin(); hello_context != m_ep_hello_contexts.end(); ++hello_context)
		{
			hello_context->second.cancel_all_reply_wait();
		}
	}

	void server::handle_hello_message_from(const hello_message& _hello_message, const ep_type& sender)
	{
		switch (_hello_message.type())
		{
			case MESSAGE_TYPE_HELLO_REQUEST:
			{
				// We need to handle the response in the proper strand to avoid race conditions.
				m_greet_strand.post(boost::bind(&server::do_handle_hello_request, this, sender, _hello_message.unique_number()));

				break;
			}
			case MESSAGE_TYPE_HELLO_RESPONSE:
			{
				// We need to handle the response in the proper strand to avoid race conditions.
				m_greet_strand.post(boost::bind(&server::do_handle_hello_response, this, sender, _hello_message.unique_number()));

				break;
			}
			default:
			{
				// This should never happen.
				assert(false);

				break;
			}
		}
	}

	void server::do_handle_hello_request(const ep_type& sender, uint32_t hello_unique_number)
	{
		// All do_handle_hello_request() calls are done in the same strand so the following is thread-safe.
		std::map<ep_type, size_t>::iterator sender_hello = m_hello_requests_map.find(sender);

		if(sender_hello == m_hello_requests_map.end())
		{
			// new hello
			m_hello_requests_map[sender] = 1;
		}
		else if(m_hello_requests_map[sender] >= (m_hello_max_per_second * 10))
		{
			// in 10s we have reach limits! Presentation flood?
			m_logger(log_level::warning) <<
				"Received too many HELLO messages from " << sender <<
				", limit is " << (m_hello_max_per_second * 10) <<
				" messages per 10 seconds";
			return;
		}
		else
		{
			m_hello_requests_map[sender]++;
		}

		bool can_reply = m_accept_hello_messages_default;

		if (m_hello_message_received_handler)
		{
			can_reply = m_hello_message_received_handler(sender, can_reply);
		}

		if (can_reply)
		{
			const auto send_buffer = SharedBuffer(16);
			const size_t size = hello_message::write_response(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), hello_unique_number);

			async_send_to(
				send_buffer,
				size,
				sender,
				[](const boost::system::error_code&) {}
			);
		}
	}

	void server::do_handle_hello_response(const ep_type& sender, uint32_t hello_unique_number)
	{
		// All do_handle_hello_response() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[sender];

		ep_hello_context.cancel_reply_wait(hello_unique_number, true);
	}

	void server::do_set_accept_hello_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_accept_hello_messages_default() calls are done in the same strand so the following is thread-safe.
		set_accept_hello_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server::do_hello_reset_limit(const boost::system::error_code& ec)
	{
		// All do_hello_reset_limit calls are done in the same strand so the following is thread-safe.
		if (ec != boost::asio::error::operation_aborted)
		{
			m_hello_requests_map.clear();

			// rearm timer again
			m_hello_limit_timer.expires_from_now(boost::posix_time::seconds(10));
			m_hello_limit_timer.async_wait(m_greet_strand.wrap(
						boost::bind(&server::do_hello_reset_limit, this,
							boost::asio::placeholders::error)));
		}
	}

	void server::do_set_hello_message_received_callback(hello_message_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_hello_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	bool server::has_presentation_store_for(const ep_type& ep) const
	{
		// This method should only be called from within the presentation strand.
		presentation_store_map::const_iterator presentation_store = m_presentation_store_map.find(ep);

		if (presentation_store != m_presentation_store_map.end())
		{
			return !(presentation_store->second.empty());
		}

		return false;
	}

	void server::do_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		// All do_introduce_to() calls are done in the same strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		const identity_store& identity = get_identity();
		const auto send_buffer = SharedBuffer(4096);

		try
		{
			const size_t size = presentation_message::write(
				buffer_cast<uint8_t*>(send_buffer),
				buffer_size(send_buffer),
				identity.signature_certificate()
			);

			async_send_to(
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	void server::do_reintroduce_to_all(multiple_endpoints_handler_type handler)
	{
		// All do_reintroduce_to_all() calls are done in the same strand so the following is thread-safe.
		typedef results_gatherer<ep_type, boost::system::error_code, multiple_endpoints_handler_type> results_gatherer_type;

		results_gatherer_type::set_type targets;

		for (presentation_store_map::const_iterator presentation_store = m_presentation_store_map.begin(); presentation_store != m_presentation_store_map.end(); ++presentation_store)
		{
			targets.insert(presentation_store->first);
		}

		boost::shared_ptr<results_gatherer_type> rg = boost::make_shared<results_gatherer_type>(handler, targets);

		for (presentation_store_map::const_iterator presentation_store = m_presentation_store_map.begin(); presentation_store != m_presentation_store_map.end(); ++presentation_store)
		{
			async_introduce_to(presentation_store->first, boost::bind(&results_gatherer_type::gather, rg, presentation_store->first, _1));
		}
	}

	void server::do_get_presentation(const ep_type& target, optional_presentation_store_handler_type handler)
	{
		// All do_get_presentation() calls are done in the same strand so the following is thread-safe.
		handler(get_presentation(target));
	}

	void server::do_set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key, void_handler_type handler)
	{
		// All do_set_presentation() calls are done in the same strand so the following is thread-safe.
		set_presentation(target, signature_certificate, pre_shared_key);

		if (handler)
		{
			handler();
		}
	}

	void server::do_clear_presentation(const ep_type& target, void_handler_type handler)
	{
		// All do_set_presentation() calls are done in the same strand so the following is thread-safe.
		clear_presentation(target);

		if (handler)
		{
			handler();
		}
	}

	void server::handle_presentation_message_from(const identity_store& identity, const presentation_message& _presentation_message, const ep_type& sender)
	{
		const auto signature_certificate = _presentation_message.signature_certificate();

		async_has_session_with_endpoint(sender, [this, identity, sender, signature_certificate](bool has_session) {
			m_presentation_strand.post(
				boost::bind(
					&server::do_handle_presentation,
					this,
					identity,
					sender,
					has_session,
					signature_certificate
				)
			);
		});
	}

	void server::do_handle_presentation(const identity_store& identity, const ep_type& sender, bool has_session, cert_type signature_certificate)
	{
		// All do_handle_presentation() calls are done in the same strand so the following is thread-safe.
		std::map<ep_type, size_t>::iterator sender_presentation = m_presentation_requests_map.find(sender);

		if(sender_presentation == m_presentation_requests_map.end())
		{
			// new presentation
			m_presentation_requests_map[sender] = 1;
		}
		else if(m_presentation_requests_map[sender] >= (m_presentation_max_per_second * 10))
		{
			// in 10s we have reach limits! Presentation flood?
			m_logger(log_level::warning) <<
				"Received too many PRESENTATION messages from " << sender <<
				", limit is " << (m_presentation_max_per_second * 10) <<
				" messages per 10 seconds";
			return;
		}
		else
		{
			m_presentation_requests_map[sender]++;
		}

		presentation_status_type presentation_status = PS_FIRST;

		const presentation_store_map::iterator entry = m_presentation_store_map.find(sender);

		if (entry != m_presentation_store_map.end())
		{
			if (compare_certificates(entry->second.signature_certificate(), signature_certificate))
			{
				presentation_status = PS_SAME;
			}
			else
			{
				presentation_status = PS_NEW;
			}
		}

		if (m_presentation_message_received_handler)
		{
			if (!m_presentation_message_received_handler(sender, signature_certificate, presentation_status, has_session))
			{
				return;
			}
		}

		m_presentation_store_map[sender] = presentation_store(signature_certificate, identity.pre_shared_key());
	}

	void server::do_presentation_reset_limit(const boost::system::error_code& ec)
	{
		// All do_presentation_reset_limit calls are done in the same strand so the following is thread-safe.
		if (ec != boost::asio::error::operation_aborted)
		{
			m_presentation_requests_map.clear();

			// rearm timer again
			m_presentation_limit_timer.expires_from_now(boost::posix_time::seconds(10));
			m_presentation_limit_timer.async_wait(m_presentation_strand.wrap(
						boost::bind(&server::do_presentation_reset_limit, this,
							boost::asio::placeholders::error)));
		}
	}

	void server::do_set_presentation_message_received_callback(presentation_message_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_presentation_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_presentation_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	cipher_suite_type server::get_first_common_supported_cipher_suite(const cipher_suite_list_type& reference, const cipher_suite_list_type& capabilities, cipher_suite_type default_value = cipher_suite_type::unsupported)
	{
		for (auto&& cs : reference)
		{
			if (std::find(capabilities.begin(), capabilities.end(), cs) != capabilities.end())
			{
				return cs;
			}
		}

		return default_value;
	}

	elliptic_curve_type server::get_first_common_supported_elliptic_curve(const elliptic_curve_list_type& reference, const elliptic_curve_list_type& capabilities, elliptic_curve_type default_value = elliptic_curve_type::unsupported)

	{
		for (auto&& cs : reference)
		{
			if (std::find(capabilities.begin(), capabilities.end(), cs) != capabilities.end())
			{
				return cs;
			}
		}

		return default_value;
	}

	void server::do_request_session(const identity_store& identity, const ep_type& target, simple_handler_type handler)
	{
		// All do_request_session() calls are done in the session strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		peer_session& p_session = m_peer_sessions[target];

		if (p_session.has_current_session())
		{
			handler(server_error::session_already_exist);

			return;
		}

		const SharedBuffer send_buffer = m_session_buffers.empty() ? SharedBuffer(65536) : [this]() {
			const auto result = m_session_buffers.front();
			m_session_buffers.pop_front();

			return result;
		}();

		try
		{
			const auto next_session_number = p_session.next_session_number();
			const auto local_host_identifier = p_session.local_host_identifier();

			size_t size = 0;

			if (!!identity.signature_key())
			{
				m_logger(log_level::trace) << "Sending session request message to " << target << " (next_session_number: " << next_session_number << ", local_host_identifier: " << local_host_identifier << ", asymmetric signature)";

				size = session_request_message::write(
					buffer_cast<uint8_t*>(send_buffer),
					buffer_size(send_buffer),
					next_session_number,
					local_host_identifier,
					m_cipher_suites,
					m_elliptic_curves,
					identity.signature_key()
				);
			}
			else
			{
				m_logger(log_level::trace) << "Sending session request message to " << target << " (next_session_number: " << next_session_number << ", local_host_identifier: " << local_host_identifier << ", HMAC signature)";

				size = session_request_message::write(
					buffer_cast<uint8_t*>(send_buffer),
					buffer_size(send_buffer),
					next_session_number,
					local_host_identifier,
					m_cipher_suites,
					m_elliptic_curves,
					buffer_cast<const uint8_t*>(identity.pre_shared_key()),
					buffer_size(identity.pre_shared_key())
					);
			}

			// disable buffer recycling from now (possible issue with SharedBuffer
			// which references itself due to lambda capture)
			async_send_to(
				/*
				SharedBuffer(send_buffer, [this](const SharedBuffer& buffer) {
					m_session_strand.post([this, buffer]() {
						m_session_buffers.push_back(buffer);
					});
				}),
				*/
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	void server::do_close_session(const ep_type& target, simple_handler_type handler)
	{
		// All do_close_session() calls are done in the same strand so the following is thread-safe.

		if (m_peer_sessions[target].clear())
		{
			handler(server_error::success);

			if (m_session_lost_handler)
			{
				m_session_lost_handler(target, session_loss_reason::manual_termination);
			}
		}
		else
		{
			handler(server_error::no_session_for_host);
		}
	}

	void server::do_handle_session_request(SharedBuffer data, const identity_store& identity, const ep_type& sender, const session_request_message& _session_request_message)
	{
		// All do_handle_session_request() calls are done in the presentation strand so the following is thread-safe.
		if (!has_presentation_store_for(sender))
		{
			// No presentation_store for the given host.
			// We do nothing.
			m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " but no presentation is available. Ignoring.";

			return;
		}

		// We make sure the signatures matches.
		if (!!m_presentation_store_map[sender].signature_certificate())
		{
			 if (!_session_request_message.check_signature(m_presentation_store_map[sender].signature_certificate().public_key()))
			 {
				 m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " with an invalid asymmetric signature. Ignoring.";

				 return;
			 }
		}
		else
		{
			const auto psk = m_presentation_store_map[sender].pre_shared_key();

			if (!_session_request_message.check_signature(buffer_cast<const uint8_t*>(psk), buffer_size(psk)))
			{
				 m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " with an invalid HMAC signature. Ignoring.";

				 return;
			}
		}

		// The make_shared_buffer_handler() call below is necessary so that the reference to session_request_message remains valid.
		m_session_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server::do_handle_verified_session_request,
					this,
					identity,
					sender,
					_session_request_message
				)
			)
		);
	}

	void server::do_handle_verified_session_request(const identity_store& identity, const ep_type& sender, const session_request_message& _session_request_message)
	{
		// All do_handle_verified_session_request() calls are done in the session strand so the following is thread-safe.

		// Get the associated session, creating one if none exists.
		peer_session& p_session = m_peer_sessions[sender];

		if (!p_session.set_first_remote_host_identifier(_session_request_message.host_identifier()))
		{
			m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " but the host identifier does not match (Received: " << _session_request_message.host_identifier() << ". Expected: " << *p_session.remote_host_identifier() << "). Ignoring.";

			// The host identifier does not match.
			return;
		}

		const cipher_suite_list_type cipher_suites = _session_request_message.cipher_suite_capabilities();
		const elliptic_curve_list_type elliptic_curves = _session_request_message.elliptic_curve_capabilities();
		const cipher_suite_type calg = get_first_common_supported_cipher_suite(m_cipher_suites, cipher_suites);
		const elliptic_curve_type ec = get_first_common_supported_elliptic_curve(m_elliptic_curves, elliptic_curves);

		if ((calg == cipher_suite_type::unsupported) || (ec == elliptic_curve_type::unsupported))
		{
			// No suitable cipher and/or elliptic curve is available.
			m_logger(log_level::warning) << "Received a SESSION_REQUEST from " << sender << " but can't agree on the cipher suite to use. Ignoring.";

			return;
		}

		bool can_reply = m_accept_session_request_messages_default;

		if (m_session_request_message_received_handler)
		{
			can_reply = m_session_request_message_received_handler(sender, cipher_suites, elliptic_curves, m_accept_session_request_messages_default);
		}

		if (!can_reply)
		{
			m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " but not allowed to reply (`m_accept_session_request_messages_default` is " << m_accept_session_request_messages_default << ").";
		}
		else
		{
			if (!p_session.has_current_session())
			{
				m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " with session number " << _session_request_message.session_number() << " and cipher suite " << calg << "_" << ec << ". No current session exist: preparing one and sending it.";

				p_session.prepare_session(_session_request_message.session_number(), calg, ec);
				do_send_session(identity, sender, p_session.next_session_parameters());
			}
			else
			{
				m_logger(log_level::trace) << "Current session has number " << p_session.current_session().parameters.session_number << " and " << sender << " requests session number " << _session_request_message.session_number();

				if (_session_request_message.session_number() > p_session.current_session().parameters.session_number)
				{
					m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " with session number " << _session_request_message.session_number() << " and cipher suite " << calg << "_" << ec << ". A current session exists but has the number " << p_session.current_session().parameters.session_number << ": preparing a new session and sending it.";

					// A new session is requested. Sending a new message.
					p_session.prepare_session(_session_request_message.session_number(), calg, ec);
					do_send_session(identity, sender, p_session.next_session_parameters());
				}
				else
				{
					m_logger(log_level::trace) << "Received a SESSION_REQUEST from " << sender << " with session number " << _session_request_message.session_number() << " and cipher suite " << calg << "_" << ec << ". A current session exists and has the number " << p_session.current_session().parameters.session_number << ": sending the current session.";

					// An old session is requested: sending the same message.
					do_send_session(identity, sender, p_session.current_session_parameters());
				}
			}
		}
	}

	std::set<server::ep_type> server::get_session_endpoints() const
	{
		// All get_session_endpoints() calls are done in the same strand so the following is thread-safe.
		std::set<ep_type> result;

		for (auto&& p_session: m_peer_sessions)
		{
			if (p_session.second.has_current_session())
			{
				result.insert(p_session.first);
			}
		}

		return result;
	}

	bool server::has_session_with_endpoint(const ep_type& host)
	{
		// All has_session_with_endpoint() calls are done in the same strand so the following is thread-safe.
		const auto p_session = m_peer_sessions.find(host);

		if (p_session != m_peer_sessions.end())
		{
			return p_session->second.has_current_session();
		}

		return false;
	}

	void server::do_get_session_endpoints(endpoints_handler_type handler)
	{
		// All do_get_session_endpoints() calls are done in the same strand so the following is thread-safe.
		handler(get_session_endpoints());
	}

	void server::do_has_session_with_endpoint(const ep_type& host, boolean_handler_type handler)
	{
		// All do_has_session_with_endpoint() calls are done in the same strand so the following is thread-safe.
		handler(has_session_with_endpoint(host));
	}

	void server::do_set_accept_session_request_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_accept_session_request_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_cipher_suites(cipher_suite_list_type cipher_suites, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_cipher_suites(cipher_suites);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_elliptic_curves(elliptic_curve_list_type elliptic_curves, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_elliptic_curves(elliptic_curves);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_request_message_received_callback(session_request_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_session_request_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_send_session(const identity_store& identity, const ep_type& target, const peer_session::session_parameters& parameters)
	{
		// All do_send_session() calls are done in the session strand so the following is thread-safe.
		m_logger(log_level::trace) << "Sending session message to " << target << " (session number: " << parameters.session_number << ", cipher suite: " << parameters.cipher_suite << ", elliptic curve: " << parameters.elliptic_curve << ").";

		peer_session& p_session = m_peer_sessions[target];
		const SharedBuffer send_buffer = m_session_buffers.empty() ? SharedBuffer(65536) : [this]() {
			const auto result = m_session_buffers.front();
			m_session_buffers.pop_front();

			return result;
		}();


		try
		{
			size_t size = 0;

			if (!!identity.signature_key())
			{
				size = session_message::write(
					buffer_cast<uint8_t*>(send_buffer),
					buffer_size(send_buffer),
					parameters.session_number,
					p_session.local_host_identifier(),
					parameters.cipher_suite,
					parameters.elliptic_curve,
					buffer_cast<const void*>(parameters.public_key),
					buffer_size(parameters.public_key),
					identity.signature_key()
				);
			}
			else
			{
				size = session_message::write(
					buffer_cast<uint8_t*>(send_buffer),
					buffer_size(send_buffer),
					parameters.session_number,
					p_session.local_host_identifier(),
					parameters.cipher_suite,
					parameters.elliptic_curve,
					buffer_cast<const void*>(parameters.public_key),
					buffer_size(parameters.public_key),
					buffer_cast<const uint8_t*>(identity.pre_shared_key()),
					buffer_size(identity.pre_shared_key())
				);
			}

			// disable buffer recycling from now (possible issue with SharedBuffer
			// which references itself due to lambda capture)
			async_send_to(
				/*
				SharedBuffer(send_buffer, [this](const SharedBuffer& buffer) {
					m_session_strand.post([this, buffer]() {
						m_session_buffers.push_back(buffer);
					});
				}),
				*/
				send_buffer,
				size,
				target,
				[] (const boost::system::error_code&) {}
			);
		}
		catch (const boost::system::system_error& ex)
		{
			m_logger(log_level::error) << "Error sending session to " << target << ": " << ex.what() << ".";
		}
	}

	void server::do_handle_session(SharedBuffer data, const identity_store& identity, const ep_type& sender, const session_message& _session_message)
	{
		// All do_handle_session() calls are done in the same strand so the following is thread-safe.

		if (!has_presentation_store_for(sender))
		{
			// No presentation_store for the given host.
			// We do nothing.
			m_logger(log_level::trace) << "Received a SESSION from " << sender << " but no presentation is available. Ignoring.";

			return;
		}

		// We make sure the signatures matches.
		if (!!m_presentation_store_map[sender].signature_certificate())
		{
			if (!_session_message.check_signature(m_presentation_store_map[sender].signature_certificate().public_key()))
			{
				m_logger(log_level::trace) << "Received a SESSION from " << sender << " with an invalid asymmetric signature. Ignoring.";

				return;
			}
		}
		else
		{
			const auto psk = m_presentation_store_map[sender].pre_shared_key();

			if (!_session_message.check_signature(buffer_cast<const uint8_t*>(psk), buffer_size(psk)))
			{
				m_logger(log_level::trace) << "Received a SESSION from " << sender << " with an invalid HMAC signature. Ignoring.";

				return;
			}
		}

		m_session_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server::do_handle_verified_session,
					this,
					identity,
					sender,
					_session_message
				)
			)
		);
	}

	void server::do_handle_verified_session(const identity_store& identity, const ep_type& sender, const session_message& _session_message)
	{
		// All do_handle_verified_session() calls are done in the session strand so the following is thread-safe.
		peer_session& p_session = m_peer_sessions[sender];

		if (!p_session.set_first_remote_host_identifier(_session_message.host_identifier()))
		{
			m_logger(log_level::trace) << "Received a SESSION from " << sender << " but the host identifier does not match (Received: " << _session_message.host_identifier() << ". Expected: " << *p_session.remote_host_identifier() << "). Ignoring.";

			// The host identifier does not match.
			return;
		}

		const bool session_is_new = !p_session.has_current_session();

		if (!session_is_new)
		{
			if (_session_message.session_number() == p_session.current_session().parameters.session_number)
			{
				// The session number matches the current session.
				if (p_session.current_session().parameters.cipher_suite != _session_message.cipher_suite())
				{
					m_logger(log_level::trace) << "Received a SESSION from " << sender << " with session number " << _session_message.session_number() << " and cipher suite " << _session_message.cipher_suite() << ". A session currently exists and has the same number but its cipher suite does not match (" << p_session.current_session().parameters.cipher_suite << "): requesting a new session.";

					// The parameters don't match the current session. Requesting a new one.
					do_request_session(identity, sender, &null_simple_handler);
				}
				else
				{
					m_logger(log_level::trace) << "Received a SESSION from " << sender << " with session number " << _session_message.session_number() << " and cipher suite " << _session_message.cipher_suite() << ". A session currently exists and has the same number and cipher suite. Ignoring.";
				}

				return;
			}
			else if (_session_message.session_number() < p_session.current_session().parameters.session_number)
			{
				m_logger(log_level::trace) << "Received a SESSION from " << sender << " with session number " << _session_message.session_number() << " and cipher suite " << _session_message.cipher_suite() << ". A session currently exists and has a higher number (" << p_session.current_session().parameters.session_number << "). Ignoring.";

				// This is an old session message. Ignore it.
				return;
			}
		}

		if (_session_message.cipher_suite() == cipher_suite_type::unsupported)
		{
			m_logger(log_level::trace) << "Received a SESSION from " << sender << " with session number " << _session_message.session_number() << " but an unsupported cipher suite. Failing session handshake.";

			if (m_session_failed_handler)
			{
				m_session_failed_handler(sender, session_is_new);
			}

			return;
		}

		bool can_accept = m_accept_session_messages_default;

		if (m_session_message_received_handler)
		{
			can_accept = m_session_message_received_handler(sender, _session_message.cipher_suite(), _session_message.elliptic_curve(), can_accept);
		}

		if (!can_accept)
		{
			m_logger(log_level::trace) << "Received a SESSION from " << sender << " but not allowed to accept (`m_accept_session_messages_default` is " << m_accept_session_messages_default << ").";
		}
		else
		{
			bool session_completed = true;

			try
			{
				if (!p_session.complete_session(_session_message.public_key(), _session_message.public_key_size()))
				{
					m_logger(log_level::trace) << "Received a SESSION from " << sender << " with session number " << _session_message.session_number() << " but no session was prepared yet. Preparing a new one.";

					// We received a session message but no session was prepared yet: we issue one and retry.
					p_session.prepare_session(_session_message.session_number(), _session_message.cipher_suite(), _session_message.elliptic_curve());

					if (!p_session.complete_session(_session_message.public_key(), _session_message.public_key_size()))
					{
						// Unable to complete the session.
						m_logger(log_level::warning) << "Unable to compute the session keys with " << sender << ".";

						return;
					}
				}
			}
			catch (const std::exception& ex)
			{
				session_completed = false;

				m_logger(log_level::error) << "Exception while computing the session keys with " << sender << ": " << ex.what() << ".";

				if (m_session_error_handler)
				{
					m_session_error_handler(sender, session_is_new, ex);
				}
			}

			if (session_completed)
			{
				m_logger(log_level::trace) << "Session established with " << sender << ". Sending acknowledgement session message back.";

				do_send_session(identity, sender, p_session.current_session_parameters());

				if (m_session_established_handler)
				{
					m_session_established_handler(sender, session_is_new, p_session.current_session().parameters.cipher_suite, p_session.current_session().parameters.elliptic_curve);
				}
			}
		}
	}

	void server::do_set_accept_session_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_accept_session_messages_default() calls are done in the same strand so the following is thread-safe.
		set_accept_session_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_message_received_callback(session_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_session_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_failed_callback(session_failed_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_failed_callback() calls are done in the same strand so the following is thread-safe.
		set_session_failed_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_error_callback(session_error_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_error_callback() calls are done in the same strand so the following is thread-safe.
		set_session_error_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_established_callback(session_established_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_established_callback() calls are done in the same strand so the following is thread-safe.
		set_session_established_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_session_lost_callback(session_lost_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_lost_callback() calls are done in the same strand so the following is thread-safe.
		set_session_lost_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_send_data(const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data, simple_handler_type handler)
	{
		// All do_send_data() calls are done in the session strand so the following is thread-safe.
		peer_session& p_session = m_peer_sessions[target];

		do_send_data_to_session(p_session, target, channel_number, data, handler);
	}

	void server::do_send_data_to_list(const std::set<ep_type>& targets, channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler)
	{
		// All do_send_data_to_list() calls are done in the session strand so the following is thread-safe.
		typedef results_gatherer<ep_type, boost::system::error_code, multiple_endpoints_handler_type> results_gatherer_type;

		boost::shared_ptr<results_gatherer_type> rg = boost::make_shared<results_gatherer_type>(handler, targets);

		for (auto&& item: m_peer_sessions)
		{
			if (targets.count(item.first) > 0)
			{
				do_send_data_to_session(item.second, item.first, channel_number, data, boost::bind(&results_gatherer_type::gather, rg, item.first, _1));
			}
		}
	}

	void server::do_send_data_to_all(channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler)
	{
		// All do_send_data_to_all() calls are done in the session strand so the following is thread-safe.
		do_send_data_to_list(get_session_endpoints(), channel_number, data, handler);
	}

	void server::do_send_data_to_session(peer_session& p_session, const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data, simple_handler_type handler)
	{
		// All do_send_data_to_session() calls are done in the session strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		if (!p_session.has_current_session())
		{
			handler(server_error::no_session_for_host);

			return;
		}

		// Get either a new buffer or an old, recycled one if possible.
		const SharedBuffer send_buffer = m_session_buffers.empty() ? SharedBuffer(65536) : [this]() {
			const auto result = m_session_buffers.front();
			m_session_buffers.pop_front();

			return result;
		}();

		try
		{
			const size_t size = data_message::write(
				buffer_cast<uint8_t*>(send_buffer),
				buffer_size(send_buffer),
				channel_number,
				p_session.increment_local_sequence_number(),
				p_session.current_session().parameters.cipher_suite.to_cipher_algorithm(),
				buffer_cast<const uint8_t*>(data),
				buffer_size(data),
				buffer_cast<const uint8_t*>(p_session.current_session().local_session_key),
				buffer_size(p_session.current_session().local_session_key),
				buffer_cast<const uint8_t*>(p_session.current_session().local_nonce_prefix),
				buffer_size(p_session.current_session().local_nonce_prefix)
			);

			// disable buffer recycling from now (possible issue with SharedBuffer
			// which references itself due to lambda capture)
			async_send_to(
				/*
				SharedBuffer(send_buffer, [this](const SharedBuffer& buffer) {
					m_session_strand.post([this, buffer]() {
						m_session_buffers.push_back(buffer);
					});
				}),
				*/
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	void server::do_send_contact_request(const ep_type& target, const hash_list_type& hash_list, simple_handler_type handler)
	{
		// All do_send_contact_request() calls are done in the session strand so the following is thread-safe.
		peer_session& p_session = m_peer_sessions[target];

		do_send_contact_request_to_session(p_session, target, hash_list, handler);
	}

	void server::do_send_contact_request_to_list(const std::set<ep_type>& targets, const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
	{
		// All do_send_contact_request_to_list() calls are done in the same strand so the following is thread-safe.
		typedef results_gatherer<ep_type, boost::system::error_code, multiple_endpoints_handler_type> results_gatherer_type;

		boost::shared_ptr<results_gatherer_type> rg = boost::make_shared<results_gatherer_type>(handler, targets);

		for (auto&& item: m_peer_sessions)
		{
			if (targets.count(item.first) > 0)
			{
				do_send_contact_request_to_session(item.second, item.first, hash_list, boost::bind(&results_gatherer_type::gather, rg, item.first, _1));
			}
		}
	}

	void server::do_send_contact_request_to_all(const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
	{
		// All do_send_contact_request_to_all() calls are done in the same strand so the following is thread-safe.
		do_send_contact_request_to_list(get_session_endpoints(), hash_list, handler);
	}

	void server::do_send_contact_request_to_session(peer_session& p_session, const ep_type& target, const hash_list_type& hash_list, simple_handler_type handler)
	{
		// All do_send_contact_request_to_session() calls are done in the same strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		if (!p_session.has_current_session())
		{
			handler(server_error::no_session_for_host);

			return;
		}

		const auto send_buffer = SharedBuffer(65536);

		try
		{
			const size_t size = data_message::write_contact_request(
				buffer_cast<uint8_t*>(send_buffer),
				buffer_size(send_buffer),
				p_session.increment_local_sequence_number(),
				p_session.current_session().parameters.cipher_suite.to_cipher_algorithm(),
				hash_list,
				buffer_cast<const uint8_t*>(p_session.current_session().local_session_key),
				buffer_size(p_session.current_session().local_session_key),
				buffer_cast<const uint8_t*>(p_session.current_session().local_nonce_prefix),
				buffer_size(p_session.current_session().local_nonce_prefix)
			);

			async_send_to(
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	void server::do_send_contact(const ep_type& target, const contact_map_type& contact_map, simple_handler_type handler)
	{
		// All do_send_contact() calls are done in the same strand so the following is thread-safe.
		peer_session& p_session = m_peer_sessions[target];

		do_send_contact_to_session(p_session, target, contact_map, handler);
	}

	void server::do_send_contact_to_list(const std::set<ep_type>& targets, const contact_map_type& contact_map, multiple_endpoints_handler_type handler)
	{
		// All do_send_contact_to_list() calls are done in the same strand so the following is thread-safe.
		typedef results_gatherer<ep_type, boost::system::error_code, multiple_endpoints_handler_type> results_gatherer_type;

		boost::shared_ptr<results_gatherer_type> rg = boost::make_shared<results_gatherer_type>(handler, targets);

		for (auto&& item: m_peer_sessions)
		{
			if (targets.count(item.first) > 0)
			{
				do_send_contact_to_session(item.second, item.first, contact_map, boost::bind(&results_gatherer_type::gather, rg, item.first, _1));
			}
		}
	}

	void server::do_send_contact_to_all(const contact_map_type& contact_map, multiple_endpoints_handler_type handler)
	{
		// All do_send_contact_to_all() calls are done in the same strand so the following is thread-safe.
		do_send_contact_to_list(get_session_endpoints(), contact_map, handler);
	}

	void server::do_send_contact_to_session(peer_session& p_session, const ep_type& target, const contact_map_type& contact_map, simple_handler_type handler)
	{
		// All do_send_contact_to_session() calls are done in the same strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		if (!p_session.has_current_session())
		{
			handler(server_error::no_session_for_host);

			return;
		}

		const auto send_buffer = SharedBuffer(65536);

		try
		{
			const size_t size = data_message::write_contact(
				buffer_cast<uint8_t*>(send_buffer),
				buffer_size(send_buffer),
				p_session.increment_local_sequence_number(),
				p_session.current_session().parameters.cipher_suite.to_cipher_algorithm(),
				contact_map,
				buffer_cast<const uint8_t*>(p_session.current_session().local_session_key),
				buffer_size(p_session.current_session().local_session_key),
				buffer_cast<const uint8_t*>(p_session.current_session().local_nonce_prefix),
				buffer_size(p_session.current_session().local_nonce_prefix)
			);

			async_send_to(
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	void server::do_handle_data(const identity_store& identity, const ep_type& sender, const data_message& _data_message)
	{
		// All do_handle_data() calls are done in the same strand so the following is thread-safe.
		peer_session& p_session = m_peer_sessions[sender];

		if (!p_session.has_current_session())
		{
			m_logger(log_level::trace) << "Received a data message from " << sender << " but no session exists. Ignoring.";

			return;
		}

		if (_data_message.sequence_number() <= p_session.current_session().remote_sequence_number)
		{
			// The message is outdated: we ignore it.
			m_logger(log_level::trace) << "Received a data message from " << sender << " but its sequence number is outdated (received: " << _data_message.sequence_number() << ", expecting: " << p_session.current_session().remote_sequence_number << "). Ignoring.";

			return;
		}

		// Get either a new buffer or an old, recycled one if possible.
		const SharedBuffer cleartext_buffer = m_session_buffers.empty() ? SharedBuffer(65536) : [this]() {
			const auto result = m_session_buffers.front();
			m_session_buffers.pop_front();

			return result;
		}();

		try
		{
			const size_t cleartext_len = _data_message.get_cleartext(
				buffer_cast<uint8_t*>(cleartext_buffer),
				buffer_size(cleartext_buffer),
				p_session.current_session().parameters.cipher_suite.to_cipher_algorithm(),
				buffer_cast<const uint8_t*>(p_session.current_session().remote_session_key),
				buffer_size(p_session.current_session().remote_session_key),
				buffer_cast<const uint8_t*>(p_session.current_session().remote_nonce_prefix),
				buffer_size(p_session.current_session().remote_nonce_prefix)
			);

			p_session.set_remote_sequence_number(_data_message.sequence_number());
			p_session.keep_alive();

			if (p_session.current_session().is_old())
			{
				// do_send_clear_session() and do_handle_data() are to be invoked through the same strand, so this is fine.
				p_session.prepare_session(p_session.next_session_number(), p_session.current_session().parameters.cipher_suite, p_session.current_session().parameters.elliptic_curve);
				do_send_session(identity, sender, p_session.next_session_parameters());
			}

			const message_type type = _data_message.type();

			if (type == MESSAGE_TYPE_KEEP_ALIVE)
			{
				// If the message is a keep alive then nothing is to be done and we avoid posting an empty call into the data strand.
				return;
			}

			// This call is fast so we hold on to the data_message a bit longer.
			do_handle_data_message(
				sender,
				type,
				/*
				SharedBuffer(cleartext_buffer, [this] (const SharedBuffer& buffer) {
					m_session_strand.post([this, buffer] () {
						m_session_buffers.push_back(buffer);
					});
				}),
				*/
				cleartext_buffer,
				buffer(cleartext_buffer, cleartext_len)
			);
		}
		catch (const boost::system::system_error& ex)
		{
			// This can happen if a message is decoded after a session rekeying.
			m_logger(log_level::error) << "Error deciphering data message from " << sender << ": " << ex.what();
		}
	}

	void server::do_handle_data_message(const ep_type& sender, message_type type, SharedBuffer buffer, boost::asio::const_buffer data)
	{
		// All do_handle_data_message() calls are done in the same strand as do_handle_data() so the following is thread-safe.
		// This call should remain *FAST*. That ims, it should only discard the message or trigger some deferred handling by another task.

		if (is_data_message_type(type))
		{
			// This is safe only because type is a DATA message type.
			const channel_number_type channel_number = to_channel_number(type);

			if (m_data_received_handler)
			{
				m_data_received_handler(sender, channel_number, buffer, data);
			}
		}
		else if (type == MESSAGE_TYPE_CONTACT_REQUEST)
		{
			const hash_list_type hash_list = data_message::parse_hash_list(buffer_cast<const uint8_t*>(data), buffer_size(data));

			m_presentation_strand.post(
				boost::bind(
					&server::do_handle_contact_request,
					this,
					sender,
					hash_list
				)
			);
		}
		else if (type == MESSAGE_TYPE_CONTACT)
		{
			const contact_map_type contact_map = data_message::parse_contact_map(buffer_cast<const uint8_t*>(data), buffer_size(data));

			m_contact_strand.post(
				boost::bind(
					&server::do_handle_contact,
					this,
					sender,
					contact_map
				)
			);
		}
	}

	void server::do_handle_contact_request(const ep_type& sender, const std::set<hash_type>& hash_list)
	{
		// All do_handle_contact_request() calls are done in the same strand so the following is thread-safe.
		contact_map_type contact_map;

		for (std::set<hash_type>::iterator hash_it = hash_list.begin(); hash_it != hash_list.end(); ++hash_it)
		{
			for (presentation_store_map::const_iterator it = m_presentation_store_map.begin(); it != m_presentation_store_map.end(); ++it)
			{
				// Contact requests do not work for PSK authenticated hosts.
				if (!it->second.signature_certificate())
				{
					continue;
				}

				const boost::optional<hash_type> hash = it->second.signature_certificate_hash();

				if (hash && (*hash == *hash_it))
				{
					if (!m_contact_request_message_received_handler || m_contact_request_message_received_handler(sender, it->second.signature_certificate(), *hash, it->first))
					{
						contact_map[*hash_it] = it->first;
					}
				}
			}
		}

		// Our contact map contains some answers: we send those.
		if (!contact_map.empty())
		{
			async_send_contact(sender, contact_map, &null_simple_handler);
		}
	}

	void server::do_handle_contact(const ep_type& sender, const contact_map_type& contact_map)
	{
		// All do_handle_contact() calls are done in the same strand so the following is thread-safe.

		if (m_contact_message_received_handler)
		{
			for (contact_map_type::const_iterator contact_it = contact_map.begin(); contact_it != contact_map.end(); ++contact_it)
			{
				m_contact_message_received_handler(sender, contact_it->first, contact_it->second);
			}
		}
	}

	void server::do_set_data_received_callback(data_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_data_received_callback() calls are done in the same strand so the following is thread-safe.
		set_data_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_contact_request_received_callback(contact_request_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_contact_request_received_callback() calls are done in the same strand so the following is thread-safe.
		set_contact_request_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_set_contact_received_callback(contact_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_contact_received_callback() calls are done in the same strand so the following is thread-safe.
		set_contact_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server::do_check_keep_alive(const boost::system::error_code& ec)
	{
		// All do_check_keep_alive() calls are done in the same strand so the following is thread-safe.
		if (ec != boost::asio::error::operation_aborted)
		{
			for (auto&& p_session: m_peer_sessions)
			{
				if (p_session.second.has_timed_out(SESSION_TIMEOUT))
				{
					if (p_session.second.clear())
					{
						if (m_session_lost_handler)
						{
							m_session_lost_handler(p_session.first, session_loss_reason::timeout);
						}
					}
				}
				else
				{
					do_send_keep_alive(p_session.first, &null_simple_handler);
				}
			}

			m_keep_alive_timer.expires_from_now(SESSION_KEEP_ALIVE_PERIOD);
			m_keep_alive_timer.async_wait(m_session_strand.wrap(boost::bind(&server::do_check_keep_alive, this, boost::asio::placeholders::error)));
		}
	}

	void server::do_send_keep_alive(const ep_type& target, simple_handler_type handler)
	{
		// All do_send_keep_alive() calls are done in the same strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		peer_session& p_session = m_peer_sessions[target];

		if (!p_session.has_current_session())
		{
			handler(server_error::no_session_for_host);

			return;
		}

		const auto send_buffer = SharedBuffer(1024);

		try
		{
			const size_t size = data_message::write_keep_alive(
				buffer_cast<uint8_t*>(send_buffer),
				buffer_size(send_buffer),
				p_session.increment_local_sequence_number(),
				p_session.current_session().parameters.cipher_suite.to_cipher_algorithm(),
				SESSION_KEEP_ALIVE_DATA_SIZE, // This is the count of random data to send.
				buffer_cast<const uint8_t*>(p_session.current_session().local_session_key),
				buffer_size(p_session.current_session().local_session_key),
				buffer_cast<const uint8_t*>(p_session.current_session().local_nonce_prefix),
				buffer_size(p_session.current_session().local_nonce_prefix)
			);

			async_send_to(
				send_buffer,
				size,
				target,
				handler
			);
		}
		catch (const boost::system::system_error& ex)
		{
			handler(ex.code());
		}
	}

	std::ostream& operator<<(std::ostream& os, server::session_loss_reason value)
	{
		switch (value)
		{
			case server::session_loss_reason::timeout:
				os << "timeout";
				break;
			case server::session_loss_reason::manual_termination:
				os << "manual termination";
				break;
			default:
				os << "unspecified reason";
				break;
		}

		return os;
	}
}

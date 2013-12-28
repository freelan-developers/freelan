/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
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

#include "server2.hpp"

#include "server_error.hpp"

#include "message.hpp"
#include "hello_message.hpp"
#include "presentation_message.hpp"
#include "session_request_message.hpp"
#include "clear_session_request_message.hpp"
#include "session_message.hpp"
#include "clear_session_message.hpp"
#include "data_message.hpp"

#include <boost/random.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/thread/future.hpp>

#include <cassert>

namespace fscp
{
	using boost::asio::buffer;
	using boost::asio::buffer_cast;
	using boost::asio::buffer_size;

	namespace
	{
		void null_simple_handler(const boost::system::error_code&)
		{
		}

		server2::ep_type& normalize(server2::ep_type& ep)
		{
			// If the endpoint is an IPv4 mapped address, return a real IPv4 address
			if (ep.address().is_v6())
			{
				boost::asio::ip::address_v6 address = ep.address().to_v6();

				if (address.is_v4_mapped())
				{
					ep = server2::ep_type(address.to_v4(), ep.port());
				}
			}

			return ep;
		}

		server2::ep_type normalize(const server2::ep_type& ep)
		{
			server2::ep_type result = ep;

			return normalize(result);
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

		cipher_algorithm_list_type get_default_cipher_capabilities()
		{
			cipher_algorithm_list_type result;

			result.push_back(cipher_algorithm_type::aes256_gcm);

			return result;
		}
	}

	// Public methods

	const cipher_algorithm_list_type server2::DEFAULT_CIPHER_CAPABILITIES = get_default_cipher_capabilities();

	server2::server2(boost::asio::io_service& io_service, const identity_store& identity) :
		m_identity_store(identity),
		m_socket(io_service),
		m_socket_strand(io_service),
		m_greet_strand(io_service),
		m_accept_hello_messages_default(true),
		m_hello_message_received_handler(),
		m_presentation_strand(io_service),
		m_presentation_message_received_handler(),
		m_session_strand(io_service),
		m_accept_session_request_messages_default(true),
		m_cipher_capabilities(DEFAULT_CIPHER_CAPABILITIES),
		m_session_request_message_received_handler(),
		m_accept_session_messages_default(true),
		m_session_message_received_handler()
	{
		// These calls are needed in C++03 to ensure that static initializations are done in a single thread.
		server_category();
	}

	void server2::open(const ep_type& listen_endpoint)
	{
		m_socket.open(listen_endpoint.protocol());

		if (listen_endpoint.address().is_v6())
		{
			// We accept both IPv4 and IPv6 addresses
			m_socket.set_option(boost::asio::ip::v6_only(false));
		}

		m_socket.bind(listen_endpoint);

		async_receive_from();
	}

	void server2::close()
	{
		cancel_all_greetings();

		m_socket.close();
	}

	void server2::async_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout)
	{
		m_greet_strand.post(boost::bind(&server2::do_greet, this, normalize(target), handler, timeout));
	}

	void server2::sync_set_accept_hello_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_hello_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_hello_message_received_callback(hello_message_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_hello_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::async_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		get_io_service().post(boost::bind(&server2::do_introduce_to, this, normalize(target), handler));
	}

	boost::system::error_code server2::sync_introduce_to(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_introduce_to(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	boost::optional<presentation_store> server2::get_presentation(const ep_type& target)
	{
		const presentation_store_map::const_iterator item = m_presentation_store_map.find(target);

		if (item != m_presentation_store_map.end())
		{
			return boost::make_optional<presentation_store>(item->second);
		}
		else
		{
			return boost::optional<presentation_store>();
		}
	}

	void server2::async_get_presentation(const ep_type& target, optional_presentation_store_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server2::do_get_presentation, this, normalize(target), handler));
	}

	boost::optional<presentation_store> server2::sync_get_presentation(const ep_type& target)
	{
		typedef boost::promise<boost::optional<presentation_store> > promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::optional<presentation_store>&) = &promise_type::set_value;

		async_get_presentation(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server2::set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate)
	{
		m_presentation_store_map[target] = presentation_store(signature_certificate, encryption_certificate);
	}

	void server2::async_set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate, void_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server2::do_set_presentation, this, normalize(target), signature_certificate, encryption_certificate, handler));
	}

	void server2::sync_set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_presentation(target, signature_certificate, encryption_certificate, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::clear_presentation(const ep_type& target)
	{
		m_presentation_store_map.erase(target);
	}

	void server2::async_clear_presentation(const ep_type& target, void_handler_type handler)
	{
		m_presentation_strand.post(boost::bind(&server2::do_clear_presentation, this, normalize(target), handler));
	}

	void server2::sync_clear_presentation(const ep_type& target)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_clear_presentation(target, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_presentation_message_received_callback(presentation_message_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_presentation_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::async_request_session(const ep_type& target, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server2::do_request_clear_session, this, normalize(target), handler));
	}

	void server2::async_close_session(const ep_type& target, simple_handler_type handler)
	{
		m_session_strand.post(boost::bind(&server2::do_close_session, this, normalize(target), handler));
	}

	boost::system::error_code server2::sync_close_session(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_close_session(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	std::vector<server2::ep_type> server2::sync_get_session_endpoints()
	{
		typedef boost::promise<std::vector<ep_type> > promise_type;
		promise_type promise;

		void (promise_type::*setter)(const std::vector<ep_type>&) = &promise_type::set_value;

		async_get_session_endpoints(boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	boost::system::error_code server2::sync_request_session(const ep_type& target)
	{
		typedef boost::promise<boost::system::error_code> promise_type;
		promise_type promise;

		void (promise_type::*setter)(const boost::system::error_code&) = &promise_type::set_value;

		async_request_session(target, boost::bind(setter, &promise, _1));

		return promise.get_future().get();
	}

	void server2::sync_set_accept_session_request_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_session_request_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_cipher_capabilities(const cipher_algorithm_list_type& cipher_capabilities)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_cipher_capabilities(cipher_capabilities, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_session_request_message_received_callback(session_request_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_request_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_accept_session_messages_default(bool value)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_accept_session_messages_default(value, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	void server2::sync_set_session_message_received_callback(session_received_handler_type callback)
	{
		typedef boost::promise<void> promise_type;
		promise_type promise;

		async_set_session_message_received_callback(callback, boost::bind(&promise_type::set_value, &promise));

		return promise.get_future().wait();
	}

	// Private methods

	void server2::do_async_receive_from()
	{
		boost::shared_ptr<ep_type> sender = boost::make_shared<ep_type>();

		socket_memory_pool::shared_buffer_type receive_buffer = m_socket_memory_pool.allocate_shared_buffer();

		// do_async_receive_from() is executed within the socket strand so this is safe.
		m_socket.async_receive_from(
			buffer(receive_buffer),
			*sender,
			boost::bind(
				&server2::handle_receive_from,
				this,
				sender,
				receive_buffer,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	void server2::handle_receive_from(boost::shared_ptr<ep_type> sender, socket_memory_pool::shared_buffer_type data, const boost::system::error_code& ec, size_t bytes_received)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			// Let's read again !
			async_receive_from();

			normalize(*sender);

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

							//TODO: Implement
							//handle_data_message_from(data_message, *sender);

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

							handle_presentation_message_from(presentation_message, *sender);

							break;
						}
						case MESSAGE_TYPE_SESSION_REQUEST:
						{
							session_request_message session_request_message(message, m_identity_store.encryption_key().size());

							handle_session_request_message_from(data, session_request_message, *sender);

							break;
						}
						case MESSAGE_TYPE_SESSION:
						{
							session_message session_message(message, m_identity_store.encryption_key().size());

							handle_session_message_from(data, session_message, *sender);
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

	server2::ep_type server2::to_socket_format(const server2::ep_type& ep)
	{
#ifdef WINDOWS
		if (m_socket.local_endpoint().address().is_v6() && ep.address().is_v4())
		{
			return server2::ep_type(boost::asio::ip::address_v6::v4_mapped(ep.address().to_v4()), ep.port());
		}
		else
		{
			return ep;
		}
#else
		static_cast<void>(socket);

		return ep;
#endif
	}

	uint32_t server2::ep_hello_context_type::generate_unique_number()
	{
		// The first call to this function is *NOT* thread-safe in C++03 !
		static boost::mt19937 rng(time(0));

		return rng();
	}

	server2::ep_hello_context_type::ep_hello_context_type() :
		m_current_hello_unique_number(generate_unique_number())
	{
	}

	uint32_t server2::ep_hello_context_type::next_hello_unique_number()
	{
		return m_current_hello_unique_number++;
	}

	template <typename WaitHandler>
	void server2::ep_hello_context_type::async_wait_reply(boost::asio::io_service& io_service, uint32_t hello_unique_number, const boost::posix_time::time_duration& timeout, WaitHandler handler)
	{
		const boost::shared_ptr<boost::asio::deadline_timer> timer = boost::make_shared<boost::asio::deadline_timer>(boost::ref(io_service), timeout);

		m_pending_requests[hello_unique_number] = pending_request_status(timer);

		timer->async_wait(handler);
	}

	bool server2::ep_hello_context_type::cancel_reply_wait(uint32_t hello_unique_number, bool success)
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

	void server2::ep_hello_context_type::cancel_all_reply_wait()
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

	bool server2::ep_hello_context_type::remove_reply_wait(uint32_t hello_unique_number, boost::posix_time::time_duration& duration)
	{
		pending_requests_map::iterator request = m_pending_requests.find(hello_unique_number);

		assert(request != m_pending_requests.end());

		const bool result = request->second.success;

		duration = boost::posix_time::microsec_clock::universal_time() - request->second.start_date;

		m_pending_requests.erase(request);

		return result;
	}

	void server2::do_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout)
	{
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline, boost::posix_time::time_duration());

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		const uint32_t hello_unique_number = ep_hello_context.next_hello_unique_number();

		greet_memory_pool::shared_buffer_type send_buffer = m_greet_memory_pool.allocate_shared_buffer();

		const size_t size = hello_message::write_request(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), hello_unique_number);

		async_send_to(
			buffer(send_buffer, size),
			target,
			m_greet_strand.wrap(
				make_shared_buffer_handler(
					send_buffer,
					boost::bind(
						&server2::do_greet_handler,
						this,
						target,
						hello_unique_number,
						handler,
						timeout,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
				)
			)
		);
	}

	void server2::do_greet_handler(const ep_type& target, uint32_t hello_unique_number, duration_handler_type handler, const boost::posix_time::time_duration& timeout, const boost::system::error_code& ec, size_t bytes_transferred)
	{
		// We don't care what the bytes_transferred value is: if an incomplete frame was sent, it is exactly the same as a network loss and we just wait for the timer expiration silently.
		static_cast<void>(bytes_transferred);

		if (ec)
		{
			handler(ec, boost::posix_time::time_duration());

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		ep_hello_context.async_wait_reply(get_io_service(), hello_unique_number, timeout, m_greet_strand.wrap(boost::bind(&server2::do_greet_timeout, this, target, hello_unique_number, handler, _1)));
	}

	void server2::do_greet_timeout(const ep_type& target, uint32_t hello_unique_number, duration_handler_type handler, const boost::system::error_code& ec)
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

	void server2::do_cancel_all_greetings()
	{
		// All do_cancel_all_greetings() calls are done in the same strand so the following is thread-safe.
		for (ep_hello_context_map::iterator hello_context = m_ep_hello_contexts.begin(); hello_context != m_ep_hello_contexts.end(); ++hello_context)
		{
			hello_context->second.cancel_all_reply_wait();
		}
	}

	void server2::handle_hello_message_from(const hello_message& _hello_message, const ep_type& sender)
	{
		switch (_hello_message.type())
		{
			case MESSAGE_TYPE_HELLO_REQUEST:
			{
				// We need to handle the response in the proper strand to avoid race conditions.
				m_greet_strand.post(boost::bind(&server2::do_handle_hello_request, this, sender, _hello_message.unique_number()));

				break;
			}
			case MESSAGE_TYPE_HELLO_RESPONSE:
			{
				// We need to handle the response in the proper strand to avoid race conditions.
				m_greet_strand.post(boost::bind(&server2::do_handle_hello_response, this, sender, _hello_message.unique_number()));

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

	void server2::do_handle_hello_request(const ep_type& sender, uint32_t hello_unique_number)
	{
		// All do_handle_hello_request() calls are done in the same strand so the following is thread-safe.
		bool can_reply = m_accept_hello_messages_default;

		if (m_hello_message_received_handler)
		{
			can_reply = m_hello_message_received_handler(sender, can_reply);
		}

		if (can_reply)
		{
			greet_memory_pool::shared_buffer_type send_buffer = m_greet_memory_pool.allocate_shared_buffer();

			const size_t size = hello_message::write_response(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), hello_unique_number);

			async_send_to(
				buffer(send_buffer, size),
				sender,
				make_shared_buffer_handler(
					send_buffer,
					boost::bind(
						&server2::handle_send_to,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
				)
			);
		}
	}

	void server2::do_handle_hello_response(const ep_type& sender, uint32_t hello_unique_number)
	{
		// All do_handle_hello_response() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[sender];

		ep_hello_context.cancel_reply_wait(hello_unique_number, true);
	}

	void server2::do_set_accept_hello_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_accept_hello_messages_default() calls are done in the same strand so the following is thread-safe.
		set_accept_hello_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_set_hello_message_received_callback(hello_message_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_hello_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	bool server2::has_presentation_store_for(const ep_type& ep) const
	{
		// This method should only be called from within the presentation strand.
		presentation_store_map::const_iterator presentation_store = m_presentation_store_map.find(ep);

		if (presentation_store != m_presentation_store_map.end())
		{
			return !(presentation_store->second.empty());
		}

		return false;
	}

	void server2::do_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		const presentation_memory_pool::shared_buffer_type send_buffer = m_presentation_memory_pool.allocate_shared_buffer();

		const size_t size = presentation_message::write(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), m_identity_store.signature_certificate(), m_identity_store.encryption_certificate());

		async_send_to(
			buffer(send_buffer, size),
			target,
			make_shared_buffer_handler(
				send_buffer,
				boost::bind(
					handler,
					_1
				)
			)
		);
	}

	void server2::do_get_presentation(const ep_type& target, optional_presentation_store_handler_type handler)
	{
		// All do_get_presentation() calls are done in the same strand so the following is thread-safe.
		handler(get_presentation(target));
	}

	void server2::do_set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate, void_handler_type handler)
	{
		// All do_set_presentation() calls are done in the same strand so the following is thread-safe.
		set_presentation(target, signature_certificate, encryption_certificate);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_clear_presentation(const ep_type& target, void_handler_type handler)
	{
		// All do_set_presentation() calls are done in the same strand so the following is thread-safe.
		clear_presentation(target);

		if (handler)
		{
			handler();
		}
	}

	void server2::handle_presentation_message_from(const presentation_message& _presentation_message, const ep_type& sender)
	{
		m_presentation_strand.post(
			boost::bind(
				&server2::do_handle_presentation,
				this,
				sender,
				_presentation_message.signature_certificate(),
				_presentation_message.encryption_certificate()
			)
		);
	}

	void server2::do_handle_presentation(const ep_type& sender, cert_type signature_certificate, cert_type encryption_certificate)
	{
		// All do_handle_presentation() calls are done in the same strand so the following is thread-safe.
		const bool is_new = !has_presentation_store_for(sender);

		if (m_presentation_message_received_handler)
		{
			if (!m_presentation_message_received_handler(sender, signature_certificate, encryption_certificate, is_new))
			{
				return;
			}
		}

		m_presentation_store_map[sender] = presentation_store(signature_certificate, encryption_certificate);
	}

	void server2::do_set_presentation_message_received_callback(presentation_message_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_presentation_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_presentation_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	cipher_algorithm_type server2::get_first_common_supported_cipher_algorithm(const cipher_algorithm_list_type& reference, const cipher_algorithm_list_type& capabilities, cipher_algorithm_type default_value = cipher_algorithm_type::unsupported)
	{
		for (cipher_algorithm_list_type::const_iterator it = reference.begin(); it != reference.end(); ++it)
		{
			if (std::find(capabilities.begin(), capabilities.end(), *it) != capabilities.end())
			{
				return it->value();
			}
		}

		return default_value;
	}

	void server2::do_request_clear_session(const ep_type& target, simple_handler_type handler)
	{
		// All do_request_clear_session() calls are done in the same strand so the following is thread-safe.
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		session_pair& session = m_session_map[target];

		const session_store::session_number_type session_number = session.has_remote_session() ? session.remote_session().session_number() + 1 : 0;

		const socket_memory_pool::shared_buffer_type cleartext_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t size = clear_session_request_message::write(
			buffer_cast<uint8_t*>(cleartext_buffer),
			buffer_size(cleartext_buffer),
			session_number,
			session.generate_local_challenge(),
			m_cipher_capabilities
		);

		m_presentation_strand.post(
			make_shared_buffer_handler(
				cleartext_buffer,
				boost::bind(
					&server2::do_request_session,
					this,
					target,
					handler,
					buffer(cleartext_buffer, size)
				)
			)
		);
	}

	void server2::do_request_session(const ep_type& target, simple_handler_type handler, boost::asio::const_buffer cleartext)
	{
		// All do_request_session() calls are done in the same strand so the following is thread-safe.

		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		if (!has_presentation_store_for(target))
		{
			// We don't have any presentation_store for the specified target.
			// Doing nothing.

			handler(server_error::no_presentation_for_host);

			return;
		}

		const socket_memory_pool::shared_buffer_type send_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t size = session_request_message::write(
			buffer_cast<uint8_t*>(send_buffer),
			buffer_size(send_buffer),
			buffer_cast<const uint8_t*>(cleartext),
			buffer_size(cleartext),
			m_presentation_store_map[target].encryption_certificate().public_key(),
			m_identity_store.signature_key()
		);

		async_send_to(
			buffer(send_buffer, size),
			target,
			make_shared_buffer_handler(
				send_buffer,
				boost::bind(
					handler,
					boost::asio::placeholders::error
				)
			)
		);
	}

	void server2::do_close_session(const ep_type& target, simple_handler_type handler)
	{
		// All do_close_session() calls are done in the same strand so the following is thread-safe.

		if (m_session_map[target].clear_remote_session())
		{
			handler(server_error::success);

			//TODO: Callback that the session was lost.
		}
		else
		{
			handler(server_error::no_session_for_host);
		}
	}

	void server2::handle_session_request_message_from(socket_memory_pool::shared_buffer_type data, const session_request_message& _session_request_message, const ep_type& sender)
	{
		// The make_shared_buffer_handler() call below is necessary so that the reference to session_request_message remains valid.
		m_presentation_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server2::do_handle_session_request,
					this,
					sender,
					boost::ref(_session_request_message)
				)
			)
		);
	}

	void server2::do_handle_session_request(const ep_type& sender, const session_request_message& _session_request_message)
	{
		if (!has_presentation_store_for(sender))
		{
			// No presentation_store for the given host.
			// We do nothing.

			return;
		}

		// All do_handle_session_request() calls are done in the same strand so the following is thread-safe.
		_session_request_message.check_signature(m_presentation_store_map[sender].signature_certificate().public_key());

		socket_memory_pool::shared_buffer_type cleartext_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t cleartext_len = _session_request_message.get_cleartext(buffer_cast<uint8_t*>(cleartext_buffer), buffer_size(cleartext_buffer), m_identity_store.encryption_key());

		clear_session_request_message clear_session_request_message(buffer_cast<const uint8_t*>(cleartext_buffer), cleartext_len);

		handle_clear_session_request_message_from(cleartext_buffer, clear_session_request_message, sender);
	}

	void server2::handle_clear_session_request_message_from(socket_memory_pool::shared_buffer_type data, const clear_session_request_message& _clear_session_request_message, const ep_type& sender)
	{
		// The make_shared_buffer_handler() call below is necessary so that the reference to session_request_message remains valid.
		m_session_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server2::do_handle_clear_session_request,
					this,
					sender,
					boost::ref(_clear_session_request_message)
				)
			)
		);
	}

	void server2::do_handle_clear_session_request(const ep_type& sender, const clear_session_request_message& _clear_session_request_message)
	{
		// All do_handle_clear_session_request() calls are done in the same strand so the following is thread-safe.
		bool can_reply = m_accept_session_request_messages_default;

		// Get the associated session, creating one if none exists.
		session_pair& session = m_session_map[sender];

		const cipher_algorithm_list_type cipher_capabilities = _clear_session_request_message.cipher_capabilities();

		const cipher_algorithm_type calg = get_first_common_supported_cipher_algorithm(m_cipher_capabilities, cipher_capabilities);

		if (m_session_request_message_received_handler)
		{
			can_reply = m_session_request_message_received_handler(sender, cipher_capabilities, m_accept_session_request_messages_default);
		}

		if (can_reply)
		{
			session.set_remote_challenge(_clear_session_request_message.challenge());
			session.set_local_cipher_algorithm(calg);

			do_send_clear_session(sender, _clear_session_request_message.session_number());
		}
	}

	void server2::do_get_session_endpoints(endpoints_handler_type handler)
	{
		// All do_get_session_endpoints() calls are done in the same strand so the following is thread-safe.

		std::vector<ep_type> result;

		result.reserve(m_session_map.size());

		for (session_pair_map::const_iterator pair = m_session_map.begin(); pair != m_session_map.end(); ++pair)
		{
			if ((pair->second.has_local_session()) && (pair->second.has_remote_session()))
			{
				result.push_back(pair->first);
			}
		}

		handler(result);
	}

	void server2::do_set_accept_session_request_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_accept_session_request_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_set_cipher_capabilities(cipher_algorithm_list_type cipher_capabilities, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_cipher_capabilities(cipher_capabilities);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_set_session_request_message_received_callback(session_request_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_hello_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_session_request_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_send_clear_session(const ep_type& target, session_store::session_number_type session_number)
	{
		// All do_send_clear_session() calls are done in the same strand so the following is thread-safe.
		session_pair& session = m_session_map[target];

		session.renew_local_session(session_number);

		const socket_memory_pool::shared_buffer_type cleartext_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t size = clear_session_message::write(
			buffer_cast<uint8_t*>(cleartext_buffer),
			buffer_size(cleartext_buffer),
			session.local_session().session_number(),
			session.remote_challenge(),
			session.local_cipher_algorithm(),
			session.local_session().encryption_key(),
			session.local_session().encryption_key_size(),
			session.local_session().nonce_prefix(),
			session.local_session().nonce_prefix_size()
		);

		m_presentation_strand.post(
			make_shared_buffer_handler(
				cleartext_buffer,
				boost::bind(
					&server2::do_send_session,
					this,
					target,
					buffer(cleartext_buffer, size)
				)
			)
		);
	}

	void server2::do_send_session(const ep_type& target, boost::asio::const_buffer cleartext)
	{
		if (!has_presentation_store_for(target))
		{
			// We don't have any presentation_store for the specified target.
			// Doing nothing.

			return;
		}

		// All do_send_session() calls are done in the same strand so the following is thread-safe.
		const socket_memory_pool::shared_buffer_type send_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t size = session_message::write(
			buffer_cast<uint8_t*>(send_buffer),
			buffer_size(send_buffer),
			buffer_cast<const uint8_t*>(cleartext),
			buffer_size(cleartext),
			m_presentation_store_map[target].encryption_certificate().public_key(),
			m_identity_store.signature_key()
		);

		async_send_to(
			buffer(send_buffer, size),
			target,
			make_shared_buffer_handler(
				send_buffer,
				boost::bind(
					&server2::handle_send_to,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			)
		);
	}

	void server2::handle_session_message_from(socket_memory_pool::shared_buffer_type data, const session_message& _session_message, const ep_type& sender)
	{
		// The make_shared_buffer_handler() call below is necessary so that the reference to session_request_message remains valid.
		m_presentation_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server2::do_handle_session,
					this,
					sender,
					boost::ref(_session_message)
				)
			)
		);
	}

	void server2::do_handle_session(const ep_type& sender, const session_message& _session_message)
	{
		// All do_handle_session() calls are done in the same strand so the following is thread-safe.

		if (!has_presentation_store_for(sender))
		{
			// No presentation_store for the given host.
			// We do nothing.

			return;
		}

		_session_message.check_signature(m_presentation_store_map[sender].signature_certificate().public_key());

		socket_memory_pool::shared_buffer_type cleartext_buffer = m_socket_memory_pool.allocate_shared_buffer();

		const size_t cleartext_len = _session_message.get_cleartext(buffer_cast<uint8_t*>(cleartext_buffer), buffer_size(cleartext_buffer), m_identity_store.encryption_key());

		clear_session_message clear_session_message(buffer_cast<const uint8_t*>(cleartext_buffer), cleartext_len);

		handle_clear_session_message_from(cleartext_buffer, clear_session_message, sender);
	}

	void server2::handle_clear_session_message_from(socket_memory_pool::shared_buffer_type data, const clear_session_message& _clear_session_message, const ep_type& sender)
	{
		// The make_shared_buffer_handler() call below is necessary so that the reference to session_request_message remains valid.
		m_session_strand.post(
			make_shared_buffer_handler(
				data,
				boost::bind(
					&server2::do_handle_clear_session,
					this,
					sender,
					boost::ref(_clear_session_message)
				)
			)
		);
	}

	void server2::do_handle_clear_session(const ep_type& sender, const clear_session_message& _clear_session_message)
	{
		// All do_handle_clear_session() calls are done in the same strand so the following is thread-safe.
		session_pair& session_pair = m_session_map[sender];

		// FIXME: Handle the possible overflow for session numbers ! Even if it
		// will happen in a *very long* time, it can still happen and will result
		// in a session loss.
		if (
			_clear_session_message.challenge() == session_pair.local_challenge() &&
			(
				!session_pair.has_remote_session() ||
				(session_pair.remote_session().session_number() < _clear_session_message.session_number())
			)
		)
		{
			bool can_accept = m_accept_session_messages_default;

			if (m_session_message_received_handler)
			{
				can_accept = m_session_message_received_handler(sender, _clear_session_message.cipher_algorithm(), can_accept);
			}

			if (can_accept)
			{
				const bool session_is_new = !session_pair.has_remote_session();

				const algorithm_info_type local = { session_pair.local_cipher_algorithm() };
				const algorithm_info_type remote = { _clear_session_message.cipher_algorithm() };

				//TODO: Remove this
				static_cast<void>(session_is_new);
				static_cast<void>(local);
				static_cast<void>(remote);

				if (_clear_session_message.cipher_algorithm() == cipher_algorithm_type::unsupported)
				{
					//TODO: Implement this
					//session_failed(sender, session_is_new, local, remote);
				}
				else
				{
					session_store _session_store(
						_clear_session_message.session_number(),
						_clear_session_message.cipher_algorithm(),
						_clear_session_message.encryption_key(),
						_clear_session_message.encryption_key_size(),
						_clear_session_message.nonce_prefix(),
						_clear_session_message.nonce_prefix_size()
					);

					session_pair.set_remote_session(_session_store);

					//TODO: Implement this
					//session_established(sender, session_is_new, local, remote);
				}
			}
		}
	}

	void server2::do_set_accept_session_messages_default(bool value, void_handler_type handler)
	{
		// All do_set_accept_session_messages_default() calls are done in the same strand so the following is thread-safe.
		set_accept_session_messages_default(value);

		if (handler)
		{
			handler();
		}
	}

	void server2::do_set_session_message_received_callback(session_received_handler_type callback, void_handler_type handler)
	{
		// All do_set_session_message_received_callback() calls are done in the same strand so the following is thread-safe.
		set_session_message_received_callback(callback);

		if (handler)
		{
			handler();
		}
	}
}

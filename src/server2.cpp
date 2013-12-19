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
#include "hello_message.hpp"

#include <boost/random.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

namespace fscp
{
	using boost::asio::buffer;
	using boost::asio::buffer_cast;
	using boost::asio::buffer_size;

	namespace
	{
		server2::ep_type to_socket_format(const boost::asio::ip::udp::socket& socket, const server2::ep_type& ep)
		{
#ifdef WINDOWS
			if (socket.local_endpoint().address().is_v6() && ep.address().is_v4())
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
	}

	server2::server2(boost::asio::io_service& io_service, const identity_store& identity) :
		m_identity_store(identity),
		m_socket(io_service),
		m_socket_strand(io_service),
		m_greet_strand(io_service)
	{
		// These calls are needed in C++03 to ensure that static initializations are done in a single thread.
		server_category();
		ep_hello_context_type::generate_unique_number();
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
	}

	void server2::close()
	{
		// We clear all pending hello requests.
		m_greet_strand.post(boost::bind(&ep_hello_context_map::clear, &m_ep_hello_contexts));

		m_socket.close();
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

		m_pending_hello_requests[hello_unique_number] = timer;

		timer->async_wait(handler);
	}

	bool server2::ep_hello_context_type::cancel_reply_wait(uint32_t hello_unique_number)
	{
		bool result = false;

		pending_hello_requests_map::iterator timer = m_pending_hello_requests.find(hello_unique_number);

		if (timer != m_pending_hello_requests.end())
		{
			if (timer->second->cancel() > 0)
			{
				result = true;
			}

			m_pending_hello_requests.erase(timer);
		}

		return result;
	}

	void server2::do_greet(const ep_type& target, simple_handler_type handler, const boost::posix_time::time_duration& timeout)
	{
		if (!m_socket.is_open())
		{
			handler(server_error::server_offline);

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		const uint32_t hello_unique_number = ep_hello_context.next_hello_unique_number();

		memory_pool::shared_buffer_type send_buffer = m_memory_pool.allocate_shared_buffer();

		const size_t size = hello_message::write_request(buffer_cast<uint8_t*>(send_buffer), buffer_size(send_buffer), hello_unique_number);

		async_send_to(buffer(send_buffer, size), target, m_greet_strand.wrap(boost::bind(&server2::do_greet_handler, this, target, hello_unique_number, handler, timeout, _1, _2)));
	}

	void server2::do_greet_handler(const ep_type& target, uint32_t hello_unique_number, simple_handler_type handler, const boost::posix_time::time_duration& timeout, const boost::system::error_code& ec, size_t bytes_transferred)
	{
		// We don't care what the bytes_transferred value is: if an incomplete frame was sent, it is exactly the same as a network loss and we just wait for the timer expiration silently.
		static_cast<void>(bytes_transferred);

		if (!ec)
		{
			handler(ec);

			return;
		}

		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		ep_hello_context.async_wait_reply(get_io_service(), hello_unique_number, timeout, m_greet_strand.wrap(boost::bind(&server2::do_greet_timeout, this, target, hello_unique_number, handler, _1)));
	}

	void server2::do_greet_timeout(const ep_type& target, uint32_t hello_unique_number, simple_handler_type handler, const boost::system::error_code& ec)
	{
		// All do_greet() calls are done in the same strand so the following is thread-safe.
		ep_hello_context_type& ep_hello_context = m_ep_hello_contexts[target];

		// This should return false, always.
		ep_hello_context.cancel_reply_wait(hello_unique_number);

		handler(ec);
	}
}

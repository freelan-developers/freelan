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

#include "server.hpp"

#include "message.hpp"
#include "hello_message.hpp"

#include <boost/bind.hpp>

#include <iostream>

using namespace boost;

namespace fscp
{
	server::server(asio::io_service& io_service, const asio::ip::udp::endpoint& listen_endpoint) :
		m_socket(io_service, listen_endpoint),
		m_hello_current_unique_number(0),
		m_accept_hello_messages_default(true),
		m_hello_message_callback(0)
	{
		async_receive();
	}

	void server::greet(const boost::asio::ip::udp::endpoint& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout)
	{
		m_socket.get_io_service().post(bind(&server::do_greet, this, target, callback, timeout));
	}

	/* Common */

	void server::async_receive()
	{
		m_socket.async_receive_from(asio::buffer(m_recv_buffer), m_sender_endpoint, bind(&server::handle_receive_from, this, asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void server::handle_receive_from(const system::error_code& error, size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			message message(m_recv_buffer.data(), bytes_recvd);

			switch (message.type())
			{
				case MESSAGE_TYPE_HELLO_REQUEST:
				case MESSAGE_TYPE_HELLO_RESPONSE:
					{
						hello_message hello_message(message);

						handle_hello_message_from(hello_message, m_sender_endpoint);

						break;
					}
				default:
					{
						break;
					}
			}

			async_receive();
		}
	}

	/* Hello messages */

	void server::do_greet(const boost::asio::ip::udp::endpoint& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout)
	{
		hello_request _hello_request(m_hello_current_unique_number, target, callback);
		_hello_request.start_timeout(get_io_service(), timeout);
		erase_expired_hello_requests(m_hello_request_list);
		m_hello_request_list.push_back(_hello_request);

		size_t size = hello_message::write_request(m_send_buffer.data(), m_send_buffer.size(), _hello_request.unique_number());

		m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);

		m_hello_current_unique_number++;
	}

	void server::handle_hello_message_from(const hello_message& _hello_message, const boost::asio::ip::udp::endpoint& sender)
	{
		switch (_hello_message.type())
		{
			case MESSAGE_TYPE_HELLO_REQUEST:
				{
					bool can_reply = m_accept_hello_messages_default;

					if (m_hello_message_callback)
					{
						can_reply = m_hello_message_callback(sender, m_accept_hello_messages_default);
					}

					if (can_reply)
					{
						size_t size = hello_message::write_response(m_send_buffer.data(), m_send_buffer.size(), _hello_message);

						m_socket.send_to(asio::buffer(m_send_buffer.data(), size), sender);
					}

					break;
				}
			case MESSAGE_TYPE_HELLO_RESPONSE:
				{
					hello_request_list::iterator _hello_request = find_hello_request(m_hello_request_list, _hello_message.unique_number(), sender);

					if (_hello_request != m_hello_request_list.end())
					{
						_hello_request->trigger();
						m_hello_request_list.erase(_hello_request);
					}

					break;
				}
			default:
				{
					break;
				}
		}
	}
}

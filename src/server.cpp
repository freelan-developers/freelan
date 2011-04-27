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
#include "presentation_message.hpp"
#include "session_request_message.hpp"
#include "session_message.hpp"
#include "clear_session_message.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <iostream>

using namespace boost;

namespace fscp
{
	server::server(asio::io_service& io_service, const ep_type& listen_endpoint, const identity_store& _identity) :
		m_socket(io_service, listen_endpoint),
		m_identity_store(_identity),
		m_hello_current_unique_number(0),
		m_accept_hello_messages_default(true),
		m_hello_message_callback(0),
		m_presentation_message_callback(0),
		m_accept_session_request_messages_default(true),
		m_session_request_message_callback(0)
	{
		async_receive();
	}

	void server::close()
	{
		get_io_service().post(bind(&server::do_close, this));
	}

	void server::greet(const ep_type& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout)
	{
		get_io_service().post(bind(&server::do_greet, this, target, callback, timeout));
	}

	void server::introduce_to(const ep_type& target)
	{
		get_io_service().post(bind(&server::do_introduce_to, this, target));
	}

	const presentation_store& server::get_presentation(const ep_type& target) const
	{
		std::map<ep_type, presentation_store>::const_iterator presentation_it = m_presentation_map.find(target);

		if (presentation_it != m_presentation_map.end())
		{
			return presentation_it->second;
		}

		throw std::runtime_error("no such host");
	}

	void server::set_presentation(const ep_type& target, cert_type sig_cert, cert_type enc_cert)
	{
		assert(sig_cert);

		if (!enc_cert)
		{
			enc_cert = sig_cert;
		}

		get_io_service().post(bind(&server::do_set_presentation, this, target, sig_cert, enc_cert));
	}

	void server::clear_presentation(const ep_type& target)
	{
		get_io_service().post(bind(&server::do_clear_presentation, this, target));
	}

	void server::request_session(const ep_type& target)
	{
		get_io_service().post(bind(&server::do_request_session, this, target));
	}

	/* Common */

	void server::do_close()
	{
		m_hello_request_list.clear();
		m_socket.close();
	}

	void server::async_receive()
	{
		m_socket.async_receive_from(asio::buffer(m_recv_buffer), m_sender_endpoint, bind(&server::handle_receive_from, this, asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void server::handle_receive_from(const system::error_code& error, size_t bytes_recvd)
	{
		if (m_socket.is_open())
		{
			if (!error && bytes_recvd > 0)
			{
				try
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
						case MESSAGE_TYPE_PRESENTATION:
							{
								presentation_message presentation_message(message);

								handle_presentation_message_from(presentation_message, m_sender_endpoint);

								break;
							}
						case MESSAGE_TYPE_SESSION_REQUEST:
							{
								session_request_message session_request_message(message);

								handle_session_request_message_from(session_request_message, m_sender_endpoint);

								break;
							}
						case MESSAGE_TYPE_SESSION:
							{
								session_message session_message(message);

								handle_session_message_from(session_message, m_sender_endpoint);
							}
						default:
							{
								break;
							}
					}
				}
				catch (std::runtime_error&)
				{
				}

				async_receive();
			}
		}
	}

	/* Hello messages */

	void server::do_greet(const ep_type& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout)
	{
		if (m_socket.is_open())
		{
			boost::shared_ptr<hello_request> _hello_request(new hello_request(get_io_service(), m_hello_current_unique_number, target, callback, timeout));

			erase_expired_hello_requests(m_hello_request_list);

			m_hello_request_list.push_back(_hello_request);

			size_t size = hello_message::write_request(m_send_buffer.data(), m_send_buffer.size(), _hello_request->unique_number());

			m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);

			m_hello_current_unique_number++;
		}
	}

	void server::handle_hello_message_from(const hello_message& _hello_message, const ep_type& sender)
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
						(*_hello_request)->cancel_timeout(true);
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

	/* Presentation messages */

	void server::do_introduce_to(const ep_type& target)
	{
		if (m_socket.is_open())
		{
			size_t size = presentation_message::write(m_send_buffer.data(), m_send_buffer.size(), m_identity_store.signature_certificate(), m_identity_store.encryption_certificate());

			m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);
		}
	}

	void server::do_set_presentation(const ep_type& target, cert_type sig_cert, cert_type enc_cert)
	{
		m_presentation_map[target] = presentation_store(sig_cert, enc_cert);
	}

	void server::do_clear_presentation(const ep_type& target)
	{
		m_presentation_map.erase(target);
	}

	void server::handle_presentation_message_from(const presentation_message& _presentation_message, const ep_type& sender)
	{
		bool accept = true;

		if (m_presentation_message_callback)
		{
			if (!m_presentation_message_callback(sender, _presentation_message.signature_certificate(), _presentation_message.encryption_certificate(), m_presentation_map.find(sender) == m_presentation_map.end()))
			{
				accept = false;
			}
		}

		if (accept)
		{
			do_set_presentation(sender, _presentation_message.signature_certificate(), _presentation_message.encryption_certificate());
		}
	}

	/* Session request messages */

	void server::do_request_session(const ep_type& target)
	{
		if (m_socket.is_open())
		{
			size_t size = session_request_message::write(m_send_buffer.data(), m_send_buffer.size());

			m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);
		}
	}

	void server::handle_session_request_message_from(const session_request_message& /*_session_request_message*/, const ep_type& sender)
	{
		bool can_reply = m_accept_session_request_messages_default;

		if (m_session_request_message_callback)
		{
			can_reply = m_session_request_message_callback(sender, m_accept_session_request_messages_default);
		}

		if (can_reply)
		{
			session_pair& session = m_session_map[sender];

			session.renew_local_session();
		}
	}

	/* Session messages */

	void server::handle_session_message_from(const session_message&, const ep_type&)
	{
	}

	void server::handle_clear_session_message_from(const session_message&, const ep_type&)
	{
	}
}

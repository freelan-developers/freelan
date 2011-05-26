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
#include "clear_session_request_message.hpp"
#include "session_message.hpp"
#include "clear_session_message.hpp"
#include "data_message.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <iostream>

using namespace boost;

namespace fscp
{
	server::server(asio::io_service& io_service, const ep_type& listen_endpoint, const identity_store& _identity) :
		m_data(0),
		m_socket(io_service, listen_endpoint),
		m_identity_store(_identity),
		m_hello_current_unique_number(0),
		m_accept_hello_messages_default(true),
		m_hello_message_callback(0),
		m_presentation_message_callback(0),
		m_accept_session_request_messages_default(true),
		m_session_request_message_callback(0),
		m_accept_session_messages_default(true),
		m_session_message_callback(0),
		m_session_established_callback(0),
		m_session_lost_callback(0),
		m_data_message_callback(0),
		m_keep_alive_timer(io_service, SESSION_KEEP_ALIVE_PERIOD)
	{
		async_receive();
		m_keep_alive_timer.async_wait(boost::bind(&server::do_check_keep_alive, this, boost::asio::placeholders::error));
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

	void server::close_session(const ep_type& host)
	{
		get_io_service().post(bind(&server::do_close_session, this, host));
	}

	void server::send_data(const ep_type& target, const void* buf, size_t buf_len)
	{
		m_data_map[target].push(buf, buf_len);

		get_io_service().post(bind(&server::do_send_data, this, target));
	}

	void server::send_data_to_all(const void* buf, size_t buf_len)
	{
		for (session_pair_map::const_iterator session_pair = m_session_map.begin(); session_pair != m_session_map.end(); ++session_pair)
		{
			if (session_pair->second.has_remote_session())
			{
				send_data(session_pair->first, buf, buf_len);
			}
		}
	}

	/* Common */

	void server::do_close()
	{
		m_keep_alive_timer.cancel();
		m_hello_request_list.clear();
		m_socket.close();
	}

	void server::async_receive()
	{
		m_socket.async_receive_from(asio::buffer(m_recv_buffer), m_sender_endpoint, bind(&server::handle_receive_from, this, asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void server::handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
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
						case MESSAGE_TYPE_DATA:
							{
								data_message data_message(message);

								handle_data_message_from(data_message, m_sender_endpoint);

								break;
							}
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
			boost::shared_ptr<hello_request> _hello_request(new hello_request(*this, m_hello_current_unique_number, target, callback, timeout));

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
						can_reply = m_hello_message_callback(*this, sender, m_accept_hello_messages_default);
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
			if (!m_presentation_message_callback(*this, sender, _presentation_message.signature_certificate(), _presentation_message.encryption_certificate(), m_presentation_map.find(sender) == m_presentation_map.end()))
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
			session_pair& session = m_session_map[target];

			session_store::session_number_type session_number = session.has_remote_session() ? session.remote_session().session_number() + 1 : 0;

			std::vector<uint8_t> cleartext = clear_session_request_message::write<uint8_t>(session_number);

			size_t size = session_request_message::write(m_send_buffer.data(), m_send_buffer.size(), &cleartext[0], cleartext.size(), m_presentation_map[target].encryption_certificate().public_key(), m_identity_store.signature_key());

			m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);
		}
	}

	void server::handle_session_request_message_from(const session_request_message& _session_request_message, const ep_type& sender)
	{
		_session_request_message.check_signature(m_presentation_map[sender].signature_certificate().public_key());

		std::vector<uint8_t> cleartext = _session_request_message.get_cleartext<uint8_t>(m_identity_store.encryption_key());

		clear_session_request_message clear_session_request_message(&cleartext[0], cleartext.size());

		handle_clear_session_request_message_from(clear_session_request_message, sender);
	}

	void server::handle_clear_session_request_message_from(const clear_session_request_message& _clear_session_request_message, const ep_type& sender)
	{
		bool can_reply = m_accept_session_request_messages_default;

		if (m_session_request_message_callback)
		{
			can_reply = m_session_request_message_callback(*this, sender, m_accept_session_request_messages_default);
		}

		if (can_reply)
		{
			do_send_session(sender, _clear_session_request_message.session_number());
		}
	}

	/* Session messages */

	void server::do_send_session(const ep_type& target, session_store::session_number_type session_number)
	{
		session_pair& session = m_session_map[target];

		session.renew_local_session(session_number);

		std::vector<uint8_t> cleartext = clear_session_message::write<uint8_t>(
		                                     session.local_session().session_number(),
		                                     session.local_session().seal_key(),
		                                     session.local_session().seal_key_size(),
		                                     session.local_session().encryption_key(),
		                                     session.local_session().encryption_key_size()
		                                 );

		size_t size = session_message::write(m_send_buffer.data(), m_send_buffer.size(), &cleartext[0], cleartext.size(), m_presentation_map[target].encryption_certificate().public_key(), m_identity_store.signature_key());

		m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);
	}

	void server::handle_session_message_from(const session_message& _session_message, const ep_type& sender)
	{
		_session_message.check_signature(m_presentation_map[sender].signature_certificate().public_key());

		std::vector<uint8_t> cleartext = _session_message.get_cleartext<uint8_t>(m_identity_store.encryption_key());

		clear_session_message clear_session_message(&cleartext[0], cleartext.size());

		handle_clear_session_message_from(clear_session_message, sender);
	}

	void server::handle_clear_session_message_from(const clear_session_message& _clear_session_message, const ep_type& sender)
	{
		session_pair& session_pair = m_session_map[sender];

		if (!session_pair.has_remote_session() || session_pair.remote_session().session_number() < _clear_session_message.session_number())
		{
			bool can_accept = m_accept_session_messages_default;

			if (m_session_message_callback)
			{
				can_accept = m_session_message_callback(*this, sender, m_accept_session_messages_default);
			}

			if (can_accept)
			{
				bool session_is_new = !session_pair.has_remote_session();

				session_store _session_store(
				    _clear_session_message.session_number(),
				    _clear_session_message.seal_key(),
				    _clear_session_message.seal_key_size(),
				    _clear_session_message.encryption_key(),
				    _clear_session_message.encryption_key_size()
				);

				session_pair.set_remote_session(_session_store);

				if (session_is_new)
				{
					session_established(sender);
				}
			}
		}
	}

	void server::session_established(const ep_type& host)
	{
		if (m_session_established_callback)
		{
			m_session_established_callback(*this, host);
		}
	}

	void server::session_lost(const ep_type& host)
	{
		if (m_session_lost_callback)
		{
			m_session_lost_callback(*this, host);
		}
	}

	void server::do_close_session(const ep_type& host)
	{
		if (m_session_map[host].clear_remote_session())
		{
			session_lost(host);
		}
	}

	/* Data messages */

	void server::do_send_data(const ep_type& target)
	{
		if (m_socket.is_open())
		{
			session_pair& session_pair = m_session_map[target];

			if (session_pair.has_remote_session())
			{
				data_store& data_store = m_data_map[target];

				for(; !data_store.empty(); data_store.pop())
				{
					size_t size = data_message::write(
					                  m_send_buffer.data(),
					                  m_send_buffer.size(),
														session_pair.remote_session().session_number(),
					                  session_pair.remote_session().sequence_number(),
					                  &data_store.front()[0],
					                  data_store.front().size(),
					                  session_pair.remote_session().seal_key(),
					                  session_pair.remote_session().seal_key_size(),
					                  session_pair.remote_session().encryption_key(),
					                  session_pair.remote_session().encryption_key_size()
					              );

					session_pair.remote_session().increment_sequence_number();

					m_socket.send_to(asio::buffer(m_send_buffer.data(), size), target);
				}
			}
		}
	}

	void server::handle_data_message_from(const data_message& _data_message, const ep_type& sender)
	{
		session_pair& session_pair = m_session_map[sender];

		if (session_pair.has_local_session())
		{
			if (_data_message.sequence_number() > session_pair.local_session().sequence_number())
			{
				_data_message.check_seal(
				    m_data_buffer.data(),
				    m_data_buffer.size(),
				    session_pair.local_session().seal_key(),
				    session_pair.local_session().seal_key_size()
				);

				size_t cnt = _data_message.get_cleartext(
				                 m_data_buffer.data(),
				                 m_data_buffer.size(),
												 session_pair.local_session().session_number(),
				                 session_pair.local_session().encryption_key(),
				                 session_pair.local_session().encryption_key_size()
				             );

				session_pair.local_session().set_sequence_number(_data_message.sequence_number());

				if (session_pair.local_session().is_old())
				{
					do_send_session(sender, session_pair.local_session().session_number() + 1);
				}

				session_pair.keep_alive();

				if (m_data_message_callback)
				{
					m_data_message_callback(*this, sender, m_data_buffer.data(), cnt);
				}
			}
		}
	}

	void server::do_check_keep_alive(const boost::system::error_code& error_code)
	{
		if (error_code != boost::asio::error::operation_aborted)
		{
			for (session_pair_map::iterator session_pair = m_session_map.begin(); session_pair != m_session_map.end(); ++session_pair)
			{
				if (session_pair->second.has_timed_out(SESSION_TIMEOUT))
				{
					if (session_pair->second.clear_remote_session())
					{
						session_lost(session_pair->first);
					}
				}
				else
				{
					do_send_keep_alive(session_pair->first);
				}
			}

			m_keep_alive_timer.expires_from_now(SESSION_KEEP_ALIVE_PERIOD);
			m_keep_alive_timer.async_wait(boost::bind(&server::do_check_keep_alive, this, boost::asio::placeholders::error));
		}
	}

	void server::do_send_keep_alive(const ep_type& /*target*/)
	{
		//TODO: Implement this method
	}
}

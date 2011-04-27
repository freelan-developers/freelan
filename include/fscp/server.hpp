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
 * \file server.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#ifndef FSCP_SERVER_HPP
#define FSCP_SERVER_HPP

#include "hello_request.hpp"
#include "identity_store.hpp"
#include "presentation_store.hpp"
#include "session_pair.hpp"

#include <boost/asio.hpp>
#include <boost/function.hpp>

#include <stdint.h>

namespace fscp
{
	class hello_message;
	class presentation_message;
	class session_request_message;

	/**
	 * \brief A FSCP server.
	 */
	class server
	{
		public:

			/**
			 * \brief The endpoint type.
			 */
			typedef boost::asio::ip::udp::endpoint ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef cryptoplus::x509::certificate cert_type;

			/**
			 * \brief Hello message callback type.
			 * \param sender The endpoint that sent the hello message.
			 * \param default_accept The default return value.
			 * \return true to reply to the hello message, false to ignore it.
			 */
			typedef boost::function<bool (const ep_type& sender, bool default_accept)> hello_message_callback;

			/**
			 * \brief Presentation message callback type.
			 * \param sender The endpoint that sent the presentation message.
			 * \param sig_cert The signature certificate.
			 * \param enc_cert The encryption certificate.
			 * \param default_accept The default return value.
			 * \return true to accept the presentation message for the originating host.
			 */
			typedef boost::function<bool (const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool default_accept)> presentation_message_callback;

			/**
			 * \brief Session request message callback type.
			 * \param sender The endpoint that sent the session request message.
			 * \param default_accept The default return value.
			 * \return true to accept the session request.
			 */
			typedef boost::function<bool (const ep_type& sender, bool default_accept)> session_request_message_callback;

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param listen_endpoint The listen endpoint.
			 * \param identity The identity store.
			 */
			server(boost::asio::io_service& io_service, const ep_type& listen_endpoint, const identity_store& identity);

			/**
			 * \brief Close the server.
			 */
			void close();

			/**
			 * \brief Get the associated io_service.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service();

			/**
			 * \brief Get the associated socket.
			 * \return The associated socket.
			 */
			boost::asio::ip::udp::socket& socket();

			/**
			 * \brief Get the identity store.
			 * \return The identity store.
			 */
			const identity_store& identity() const;

			/**
			 * \brief Set the default behavior when a hello message arrives.
			 * \param value If false, hello messages will be ignored. Default is true.
			 */
			void set_accept_hello_messages_default(bool value);

			/**
			 * \brief Set the hello message callback.
			 * \param callback The callback.
			 */
			void set_hello_message_callback(hello_message_callback callback);

			/**
			 * \brief Greet a host.
			 * \param target The target host.
			 * \param callback The callback to call on response.
			 * \param timeout The maximum time to wait for the response. Default value is 3 seconds.
			 */
			void greet(const ep_type& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

			/**
			 * \brief Set the presentation message callback.
			 * \param callback The callback.
			 */
			void set_presentation_message_callback(presentation_message_callback callback);

			/**
			 * \brief Introduce to a host.
			 * \param target The target host.
			 */
			void introduce_to(const ep_type& target);

			/**
			 * \brief Get the presentation parameters of a specified host.
			 * \param target The target host.
			 * \return The presentation parameters of the specified host.
			 * \warning IF no presentation parameters exist for the specified host, a std::runtime_error is thrown.
			 */
			const presentation_store& get_presentation(const ep_type& target) const;

			/**
			 * \brief Set the presentation parameters for a given host.
			 * \param target The target host.
			 * \param sig_cert The signature certificate. Cannot be null.
			 * \param enc_cert The encryption certificate. If null, the default, sig_cert is taken.
			 * \see clear_presentation()
			 */
			void set_presentation(const ep_type& target, cert_type sig_cert, cert_type enc_cert = cert_type());

			/**
			 * \brief Clear the presentation parameters for a given host.
			 * \param target The target host.
			 * \see set_presentation()
			 */
			void clear_presentation(const ep_type& target);

			/**
			 * \brief Set the default behavior when a session request message arrives.
			 * \param value If false, session request messages will be ignored. Default is true.
			 */
			void set_accept_session_request_messages_default(bool value);

			/**
			 * \brief Set the session request message callback.
			 * \param callback The callback.
			 */
			void set_session_request_message_callback(session_request_message_callback callback);

			/**
			 * \brief Request a session to a host.
			 * \param target The target host.
			 */
			void request_session(const ep_type& target);

		private:

			void do_close();
			void async_receive();
			void handle_receive_from(const boost::system::error_code&, size_t);

			boost::asio::ip::udp::socket m_socket;
			boost::array<uint8_t, 65536> m_recv_buffer;
			boost::array<uint8_t, 65536> m_send_buffer;
			ep_type m_sender_endpoint;
			identity_store m_identity_store;

		private:

			void do_greet(const ep_type& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout);
			void handle_hello_message_from(const hello_message&, const ep_type&);

			hello_request_list m_hello_request_list;
			uint32_t m_hello_current_unique_number;
			bool m_accept_hello_messages_default;
			hello_message_callback m_hello_message_callback;

		private:

			void do_introduce_to(const ep_type&);
			void do_set_presentation(const ep_type&, cert_type, cert_type);
			void do_clear_presentation(const ep_type&);
			void handle_presentation_message_from(const presentation_message&, const ep_type&);

			presentation_message_callback m_presentation_message_callback;
			std::map<ep_type, presentation_store> m_presentation_map;

		private:

			typedef std::map<ep_type, session_pair> session_pair_map;

			void do_request_session(const ep_type&);
			void handle_session_request_message_from(const session_request_message&, const ep_type&);

			session_pair_map m_session_map;
			bool m_accept_session_request_messages_default;
			session_request_message_callback m_session_request_message_callback;
	};

	inline boost::asio::io_service& server::get_io_service()
	{
		return socket().get_io_service();
	}

	inline boost::asio::ip::udp::socket& server::socket()
	{
		return m_socket;
	}

	inline const identity_store& server::identity() const
	{
		return m_identity_store;
	}

	inline void server::set_accept_hello_messages_default(bool value)
	{
		m_accept_hello_messages_default = value;
	}

	inline void server::set_hello_message_callback(hello_message_callback callback)
	{
		m_hello_message_callback = callback;
	}

	inline void server::set_presentation_message_callback(presentation_message_callback callback)
	{
		m_presentation_message_callback = callback;
	}

	inline void server::set_accept_session_request_messages_default(bool value)
	{
		m_accept_session_request_messages_default = value;
	}

	inline void server::set_session_request_message_callback(session_request_message_callback callback)
	{
		m_session_request_message_callback = callback;
	}
}

#endif /* FSCP_SERVER_HPP */

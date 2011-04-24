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

#include <boost/asio.hpp>
#include <boost/function.hpp>

#include <stdint.h>

namespace fscp
{
	class hello_message;

	/**
	 * \brief A FSCP server.
	 */
	class server
	{
		public:

			/**
			 * \brief Hello message callback type.
			 * \param sender The endpoint that sent the hello message.
			 * \param default_accept The default return value.
			 * \return true to reply to the hello message, false to ignore it.
			 */
			typedef boost::function<bool (const boost::asio::ip::udp::endpoint& sender, bool default_accept)> hello_message_callback;

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param listen_endpoint The listen endpoint.
			 * \param identity The identity store.
			 */
			server(boost::asio::io_service& io_service, const boost::asio::ip::udp::endpoint& listen_endpoint, const identity_store& identity);

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
			void greet(const boost::asio::ip::udp::endpoint& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

		private:

			void do_close();
			void async_receive();
			void handle_receive_from(const boost::system::error_code&, size_t);

			boost::asio::ip::udp::socket m_socket;
			boost::array<uint8_t, 65536> m_recv_buffer;
			boost::array<uint8_t, 65536> m_send_buffer;
			boost::asio::ip::udp::endpoint m_sender_endpoint;
			identity_store m_identity_store;

		private:

			void do_greet(const boost::asio::ip::udp::endpoint& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout);
			void handle_hello_message_from(const hello_message&, const boost::asio::ip::udp::endpoint&);

			hello_request_list m_hello_request_list;
			uint32_t m_hello_current_unique_number;
			bool m_accept_hello_messages_default;
			hello_message_callback m_hello_message_callback;
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
}

#endif /* FSCP_SERVER_HPP */

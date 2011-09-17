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
#include "data_store.hpp"

#include <boost/asio.hpp>
#include <boost/function.hpp>

#include <stdint.h>

namespace fscp
{
	class hello_message;
	class presentation_message;
	class session_request_message;
	class clear_session_request_message;
	class session_message;
	class clear_session_message;
	class data_message;

	/**
	 * \brief A FSCP server.
	 *
	 * This class is *NOT* thread-safe !
	 *
	 * Once an instance is running into a io_service, calling any method (except close()) is undefined behavior.
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
			 * \param server The server instance that called the callback function.
			 * \param sender The endpoint that sent the hello message.
			 * \param default_accept The default return value.
			 * \return true to reply to the hello message, false to ignore it.
			 */
			typedef boost::function<bool (server& server, const ep_type& sender, bool default_accept)> hello_message_callback;

			/**
			 * \brief Presentation message callback type.
			 * \param server The server instance that called the callback function.
			 * \param sender The endpoint that sent the presentation message.
			 * \param sig_cert The signature certificate.
			 * \param enc_cert The encryption certificate.
			 * \param default_accept The default return value.
			 * \return true to accept the presentation message for the originating host.
			 */
			typedef boost::function<bool (server& server, const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool default_accept)> presentation_message_callback;

			/**
			 * \brief Session request message callback type.
			 * \param server The server instance that called the callback function.
			 * \param sender The endpoint that sent the session request message.
			 * \param default_accept The default return value.
			 * \return true to accept the session request.
			 */
			typedef boost::function<bool (server& server, const ep_type& sender, bool default_accept)> session_request_message_callback;

			/**
			 * \brief Session message callback type.
			 * \param server The server instance that called the callback function.
			 * \param sender The endpoint that sent the session message.
			 * \param default_accept The default return value.
			 * \return true to accept the session.
			 */
			typedef boost::function<bool (server& server, const ep_type& sender, bool default_accept)> session_message_callback;

			/**
			 * \brief Data message callback type.
			 * \param server The server instance that called the callback function.
			 * \param sender The endpoint that sent the data message.
			 * \param data The sent data.
			 */
			typedef boost::function<void (server& server, const ep_type& sender, boost::asio::const_buffer data)> data_message_callback;

			/**
			 * \brief A session established callback.
			 * \param server The server instance that called the callback function.
			 * \param host The host with which a session is established.
			 */
			typedef boost::function<void (server& server, const ep_type& host)> session_established_callback;

			/**
			 * \brief A session lost callback.
			 * \param server The server instance that called the callback function.
			 * \param host The host with which a session was lost.
			 */
			typedef boost::function<void (server& server, const ep_type& host)> session_lost_callback;

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param identity The identity store.
			 */
			server(boost::asio::io_service& io_service, const identity_store& identity);

			/**
			 * \brief Open the server.
			 * \param listen_endpoint The listen endpoint.
			 */
			void open(const ep_type& listen_endpoint);

			/**
			 * \brief Close the server.
			 *
			 * This method can be called from another thread.
			 */
			void close();

			/**
			 * \brief Determine whether the socket is open.
			 * \return true if the socket is open.
			 */
			bool is_open() const;

			/**
			 * \brief Set the attached data.
			 * \param data The attached data.
			 */
			template <typename T>
			void set_data(T* data);

			/**
			 * \brief Get the attached data.
			 * \return The attached data.
			 * \warning A static_cast is used, so the caller has to make sure that the type specified is the same than the one used during the call to set_data().
			 */
			template <typename T>
			T* get_data() const;

			/**
			 * \brief Get the associated io_service.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service();

			/**
			 * \brief Get the associated socket.
			 * \return The associated socket.
			 */
			const boost::asio::ip::udp::socket& socket() const;

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
			void async_greet(const ep_type& target, hello_request::callback_type callback, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

			/**
			 * \brief Set the presentation message callback.
			 * \param callback The callback.
			 */
			void set_presentation_message_callback(presentation_message_callback callback);

			/**
			 * \brief Introduce to a host.
			 * \param target The target host.
			 */
			void async_introduce_to(const ep_type& target);

			/**
			 * \brief Get the presentation parameters of a specified host.
			 * \param target The target host.
			 * \return The presentation parameters of the specified host.
			 * \warning If no presentation parameters exist for the specified host, a std::runtime_error is thrown.
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
			void async_request_session(const ep_type& target);

			/**
			 * \brief Set the default behavior when a session message arrives.
			 * \param value If false, session messages will be ignored. Default is true.
			 */
			void set_accept_session_messages_default(bool value);

			/**
			 * \brief Set the session message callback.
			 * \param callback The callback.
			 */
			void set_session_message_callback(session_message_callback callback);

			/**
			 * \brief Set the session established callback.
			 * \param callback The callback.
			 */
			void set_session_established_callback(session_established_callback callback);

			/**
			 * \brief Set the session lost callback.
			 * \param callback The callback.
			 */
			void set_session_lost_callback(session_lost_callback callback);

			/**
			 * \brief Check if a session is established with the specified host.
			 * \param host The host.
			 * \return true if a session is currently active.
			 */
			bool has_session(const ep_type& host) const;

			/**
			 * \brief Close any existing session with the specified host.
			 * \param host The host.
			 */
			void async_close_session(const ep_type& host);

			/**
			 * \brief Send data to a host.
			 * \param target The target host.
			 * \param data The data to send.
			 */
			void async_send_data(const ep_type& target, boost::asio::const_buffer data);

			/**
			 * \brief Send data to a list of hosts.
			 * \param begin An iterator to the first target.
			 * \param end An iterator past the last target.
			 * \param data The data to send.
			 */
			template <typename T>
			void async_send_data_to_list(const T& begin, const T& end, boost::asio::const_buffer data);

			/**
			 * \brief Send data to all the hosts.
			 * \param data The data to send.
			 */
			void async_send_data_to_all(boost::asio::const_buffer data);

			/**
			 * \brief Set the data message callback.
			 * \param callback The callback.
			 */
			void set_data_message_callback(data_message_callback callback);

		private:

			void async_receive();
			void handle_receive_from(const boost::system::error_code&, size_t);

			void* m_data;
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

			typedef std::map<ep_type, presentation_store> presentation_store_map;

			void do_introduce_to(const ep_type&);
			void handle_presentation_message_from(const presentation_message&, const ep_type&);

			presentation_message_callback m_presentation_message_callback;
			presentation_store_map m_presentation_map;

		private:

			typedef std::map<ep_type, session_pair> session_pair_map;

			void do_request_session(const ep_type&);
			void handle_session_request_message_from(const session_request_message&, const ep_type&);
			void handle_clear_session_request_message_from(const clear_session_request_message&, const ep_type&);

			session_pair_map m_session_map;
			bool m_accept_session_request_messages_default;
			session_request_message_callback m_session_request_message_callback;

		private:

			void do_send_session(const ep_type&, session_store::session_number_type);
			void handle_session_message_from(const session_message&, const ep_type&);
			void handle_clear_session_message_from(const clear_session_message&, const ep_type&);
			void session_established(const ep_type&);
			void session_lost(const ep_type&);
			void do_close_session(const ep_type&);

			bool m_accept_session_messages_default;
			session_message_callback m_session_message_callback;
			session_established_callback m_session_established_callback;
			session_lost_callback m_session_lost_callback;

		private:

			typedef std::map<ep_type, data_store> data_store_map;

			void do_send_data(const ep_type&);
			void handle_data_message_from(const data_message&, const ep_type&);

			boost::array<uint8_t, 65536> m_data_buffer;
			data_store_map m_data_map;
			data_message_callback m_data_message_callback;

		private:

			void do_check_keep_alive(const boost::system::error_code&);
			void do_send_keep_alive(const ep_type&);

			boost::asio::deadline_timer m_keep_alive_timer;
	};

	inline bool server::is_open() const
	{
		return m_socket.is_open();
	}

	template <typename T>
	inline void server::set_data(T* data)
	{
		m_data = data;
	}

	template <typename T>
	inline T* server::get_data() const
	{
		return static_cast<T*>(m_data);
	}

	inline boost::asio::io_service& server::get_io_service()
	{
		return m_socket.get_io_service();
	}

	inline const boost::asio::ip::udp::socket& server::socket() const
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

	inline void server::clear_presentation(const ep_type& target)
	{
		m_presentation_map.erase(target);
	}

	inline void server::set_accept_session_request_messages_default(bool value)
	{
		m_accept_session_request_messages_default = value;
	}

	inline void server::set_session_request_message_callback(session_request_message_callback callback)
	{
		m_session_request_message_callback = callback;
	}

	inline void server::set_accept_session_messages_default(bool value)
	{
		m_accept_session_messages_default = value;
	}

	inline void server::set_session_message_callback(session_message_callback callback)
	{
		m_session_message_callback = callback;
	}

	inline void server::set_session_established_callback(session_established_callback callback)
	{
		m_session_established_callback = callback;
	}

	inline void server::set_session_lost_callback(session_lost_callback callback)
	{
		m_session_lost_callback = callback;
	}

	template <typename T>
	inline void server::async_send_data_to_list(const T& begin, const T& end, boost::asio::const_buffer data)
	{
		for (T it = begin; it != end; ++it)
		{
			async_send_data(*it, data);
		}
	}

	inline void server::set_data_message_callback(data_message_callback callback)
	{
		m_data_message_callback = callback;
	}
}

#endif /* FSCP_SERVER_HPP */

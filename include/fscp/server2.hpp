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

#ifndef FSCP_SERVER_HPP_2
#define FSCP_SERVER_HPP_2

#include "identity_store.hpp"
#include "memory_pool.hpp"
#include "presentation_store.hpp"
#include "session_pair.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>

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
	 * All the public methods are thread-safe, unless otherwise specified.
	 *
	 * async_* methods are designed to be run from inside handlers (or callbacks).
	 * sync_* methods are designed to be run outside of the server running threads while the server is running.
	 */
	class server2
	{
		public:

			// General purpose type definitions

			/**
			 * \brief The endpoint type.
			 */
			typedef boost::asio::ip::udp::endpoint ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef cryptoplus::x509::certificate cert_type;

			// Handlers

			/**
			 * \brief A void operation handler.
			 */
			typedef boost::function<void ()> void_handler_type;

			/**
			 * \brief A simple operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&)> simple_handler_type;

			/**
			 * \brief A duration operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&, const boost::posix_time::time_duration& duration)> duration_handler_type;

			/**
			 * \brief An optional presentation store handler.
			 */
			typedef boost::function<void (const boost::optional<presentation_store>&)> optional_presentation_store_handler_type;

			// Callbacks

			/**
			 * \brief A handler for when hello requests are received.
			 * \param sender The endpoint that sent the hello message.
			 * \param default_accept The default return value.
			 * \return true to reply to the hello message, false to ignore it.
			 */
			typedef boost::function<bool (const ep_type& sender, bool default_accept)> hello_message_received_handler_type;

			/**
			 * \brief A handler for when presentation requests are received.
			 * \param sender The endpoint that sent the presentation message.
			 * \param sig_cert The signature certificate.
			 * \param enc_cert The encryption certificate.
			 * \param is_new True if the presentation is new.
			 * \return true to accept the presentation message for the originating host.
			 */
			typedef boost::function<bool (const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool is_new)> presentation_message_received_handler_type;

			/**
			 * \brief A handler for when session requests are received.
			 * \param sender The endpoint that sent the session request message.
			 * \param calg_capabilities The cipher algorithm capabilities of the remote host.
			 * \param default_accept The default return value.
			 * \return true to accept the session request.
			 */
			typedef boost::function<bool (const ep_type& sender, const cipher_algorithm_list_type& calg_capabilities, bool default_accept)> session_request_received_handler_type;

			// Static variables

			static const cipher_algorithm_list_type DEFAULT_CIPHER_CAPABILITIES;

			// Public methods

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param identity The identity store.
			 */
			server2(boost::asio::io_service& io_service, const identity_store& identity);

			/**
			 * \brief Get the associated io_service.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service()
			{
				return m_socket.get_io_service();
			}

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
			 * \brief Greet an host.
			 * \param target The target to greet.
			 * \param handler The handler to call when a reply was received, an error occured or the request timed out.
			 * \param timeout The maximum time to wait for a reply.
			 */
			void async_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

			/**
			 * \brief Cancel all pending greetings.
			 */
			void cancel_all_greetings()
			{
				m_greet_strand.post(boost::bind(&server2::do_cancel_all_greetings, this));
			}

			/**
			 * \brief Set the default acceptance behavior of incoming hello messages.
			 * \param value The default value.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_accept_hello_messages_default(bool value)
			{
				m_accept_hello_messages_default = value;
			}

			/**
			 * \brief Set the default acceptance behavior of incoming hello messages.
			 * \param value The default value.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_accept_hello_messages_default(bool value, void_handler_type handler = void_handler_type())
			{
				m_greet_strand.post(boost::bind(&server2::do_set_accept_hello_messages_default, this, value, handler));
			}

			/**
			 * \brief Set the default acceptance behavior of incoming hello messages.
			 * \param value The default value.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_accept_hello_messages_default(bool value);

			/**
			 * \brief Set the hello message received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_hello_message_received_callback(hello_message_received_handler_type callback)
			{
				m_hello_message_received_handler = callback;
			}

			/**
			 * \brief Set the hello message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_hello_message_received_callback(hello_message_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_greet_strand.post(boost::bind(&server2::do_set_hello_message_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the hello message received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_hello_message_received_callback(hello_message_received_handler_type callback);

			/**
			 * \brief Send a presentation message to the specified target.
			 * \param target The target host.
			 * \param handler The handler to call when then presentation message is sent or an error occured.
			 */
			void async_introduce_to(const ep_type& target, simple_handler_type handler);

			/**
			 * \brief Send a presentation message to the specified target.
			 * \param target The target host.
			 * \return An error_code.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::system::error_code sync_introduce_to(const ep_type& target);

			/**
			 * \brief Get the presentation store associated to a target.
			 * \param target The target host.
			 * \return The presentation for the given host.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			boost::optional<presentation_store> get_presentation(const ep_type& target);

			/**
			 * \brief Get the presentation store associated to a target.
			 * \param target The target host.
			 * \param handler The handler to call with the presentation store instance.
			 */
			void async_get_presentation(const ep_type& target, optional_presentation_store_handler_type handler);

			/**
			 * \brief Get the presentation store associated to a target.
			 * \param target The target host.
			 * \return The presentation for the given host.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::optional<presentation_store> sync_get_presentation(const ep_type& target);

			/**
			 * \brief Set the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param signature_certificate The signature certificate.
			 * \param encryption_certificate The encryption certificate to use, if different from the signature certificate.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate = cert_type());

			/**
			 * \brief Set the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param signature_certificate The signature certificate.
			 * \param encryption_certificate The encryption certificate to use, if different from the signature certificate.
			 * \param handler The handler to call when the presentation was set for the given host.
			 */
			void async_set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate = cert_type(), void_handler_type handler = void_handler_type());

			/**
			 * \brief Set the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param signature_certificate The signature certificate.
			 * \param encryption_certificate The encryption certificate to use, if different from the signature certificate.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_presentation(const ep_type& target, cert_type signature_certificate, cert_type encryption_certificate = cert_type());

			/**
			 * \brief Clear the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void clear_presentation(const ep_type& target);

			/**
			 * \brief Clear the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param handler The handler to call when the presentation was cleared for the given host.
			 */
			void async_clear_presentation(const ep_type& target, void_handler_type handler = void_handler_type());

			/**
			 * \brief Clear the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_clear_presentation(const ep_type& target);

			/**
			 * \brief Set the presentation message received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_presentation_message_received_callback(presentation_message_received_handler_type callback)
			{
				m_presentation_message_received_handler = callback;
			}

			/**
			 * \brief Set the presentation message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_presentation_message_received_callback(presentation_message_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_presentation_strand.post(boost::bind(&server2::do_set_presentation_message_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the presentation message received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_presentation_message_received_callback(presentation_message_received_handler_type callback);

			/**
			 * \brief Sends a session request to an host.
			 * \param target The target to greet.
			 * \param handler The handler to call when the request was sent or an error occured.
			 */
			void async_request_session(const ep_type& target, simple_handler_type handler);

			/**
			 * \brief Sends a session request to an host.
			 * \param target The target host.
			 * \return An error_code.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::system::error_code sync_request_session(const ep_type& target);

			/**
			 * \brief Set the default acceptance behavior of incoming session requests.
			 * \param value The default value.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_accept_session_request_messages_default(bool value)
			{
				m_accept_session_request_messages_default = value;
			}

			/**
			 * \brief Set the default acceptance behavior of incoming session requests.
			 * \param value The default value.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_accept_session_request_messages_default(bool value, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server2::do_set_accept_session_request_messages_default, this, value, handler));
			}

			/**
			 * \brief Set the default acceptance behavior of incoming session requests.
			 * \param value The default value.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_accept_session_request_messages_default(bool value);

			/**
			 * \brief Set the cipher capabilities.
			 * \param cipher_capabilities The cipher capabilities.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_cipher_capabilities(const cipher_algorithm_list_type& cipher_capabilities)
			{
				m_cipher_capabilities = cipher_capabilities;
			}

			/**
			 * \brief Set the cipher capabilities.
			 * \param cipher_capabilities The cipher capabilities.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_cipher_capabilities(const cipher_algorithm_list_type& cipher_capabilities, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server2::do_set_cipher_capabilities, this, cipher_capabilities, handler));
			}

			/**
			 * \brief Set the cipher capabilities.
			 * \param cipher_capabilities The cipher capabilities.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_cipher_capabilities(const cipher_algorithm_list_type& cipher_capabilities);

			/**
			 * \brief Set the session request message received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_request_message_received_callback(session_request_received_handler_type callback)
			{
				m_session_request_message_received_handler = callback;
			}

			/**
			 * \brief Set the session request message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_request_message_received_callback(session_request_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server2::do_set_session_request_message_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session request message received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_request_message_received_callback(session_request_received_handler_type callback);

		private:

			const identity_store m_identity_store;

		private:

			typedef memory_pool<65536, 32> socket_memory_pool;

			void async_receive_from()
			{
				m_socket_strand.post(boost::bind(&server2::do_async_receive_from, this));
			}

			void do_async_receive_from();
			void handle_receive_from(boost::shared_ptr<ep_type>, socket_memory_pool::shared_buffer_type, const boost::system::error_code&, size_t);

			ep_type to_socket_format(const server2::ep_type& ep);

			template <typename ConstBufferSequence, typename WriteHandler>
			void async_send_to(const ConstBufferSequence& data, const ep_type& target, WriteHandler handler)
			{
				m_socket_strand.post(boost::bind(&boost::asio::ip::udp::socket::async_send_to<ConstBufferSequence, WriteHandler>, &m_socket, data, to_socket_format(target), 0, handler));
			}

			void handle_send_to(const boost::system::error_code&, size_t) {};

			boost::asio::ip::udp::socket m_socket;
			boost::asio::strand m_socket_strand;
			socket_memory_pool m_socket_memory_pool;

		private: // HELLO messages

			/**
			 * @brief Represents a context for an endpoint.
			 */
			class ep_hello_context_type
			{
				public:

					/**
					 * @brief Generate an unique number.
					 * @warning The first invocation of this function is *NOT* thread-safe in C++03.
					 */
					static uint32_t generate_unique_number();

					/**
					 * @brief Create a new context.
					 */
					ep_hello_context_type();

					/**
					 * @brief Returns the current hello unique number and generates a new one.
					 * @return The current hello unique number.
					 */
					uint32_t next_hello_unique_number();

					/**
					 * @brief Asynchronously waits for a hello reply.
					 * @param io_service The io_service instance to use for the wait.
					 * @param hello_unique_number The unique hello number.
					 * @param timeout The time to wait for the reply.
					 * @param handler The handler to call upon timeout or cancellation.
					 */
					template <typename WaitHandler>
					void async_wait_reply(boost::asio::io_service& io_service, uint32_t hello_unique_number, const boost::posix_time::time_duration& timeout, WaitHandler handler);

					/**
					 * @brief Cancel a hello reply wait timer.
					 * @param hello_unique_number The hello reply number.
					 * @param success Whether the cancel is the result of a received reply.
					 * @return true if the timer was cancelled or false if it was too late to do so.
					 */
					bool cancel_reply_wait(uint32_t hello_unique_number, bool success);

					/**
					 * @brief Cancel all pending hello request wait timers.
					 *
					 * This call is similar to calling cancel_reply_wait(<num>, false) for all hello unique numbers.
					 */
					void cancel_all_reply_wait();

					/**
					 * @brief Remove a hello reply wait from the pending list.
					 * @param hello_unique_number The hello reply number.
					 * @param duration A variable whose value after the call will be the time elapsed since the creation of the request.
					 * @return The success status of the request.
					 */
					bool remove_reply_wait(uint32_t hello_unique_number, boost::posix_time::time_duration& duration);

				private:

					struct pending_request_status
					{
						pending_request_status() :
							timer(),
							start_date(boost::posix_time::microsec_clock::universal_time()),
							success(false)
						{}

						pending_request_status(boost::shared_ptr<boost::asio::deadline_timer> _timer) :
							timer(_timer),
							start_date(boost::posix_time::microsec_clock::universal_time()),
							success(false)
						{}

						boost::shared_ptr<boost::asio::deadline_timer> timer;
						boost::posix_time::ptime start_date;
						bool success;
					};

					typedef std::map<uint32_t, pending_request_status> pending_requests_map;

					uint32_t m_current_hello_unique_number;
					pending_requests_map m_pending_requests;
			};

			typedef memory_pool<16> greet_memory_pool;
			typedef std::map<ep_type, ep_hello_context_type> ep_hello_context_map;

			void do_greet(const ep_type&, duration_handler_type, const boost::posix_time::time_duration&);
			void do_greet_handler(const ep_type&, uint32_t, duration_handler_type, const boost::posix_time::time_duration&, const boost::system::error_code&, size_t);
			void do_greet_timeout(const ep_type&, uint32_t, duration_handler_type, const boost::system::error_code&);
			void do_cancel_all_greetings();

			void handle_hello_message_from(const hello_message&, const ep_type&);
			void do_handle_hello_request(const ep_type&, uint32_t);
			void do_handle_hello_response(const ep_type&, uint32_t);

			void do_set_accept_hello_messages_default(bool, void_handler_type);
			void do_set_hello_message_received_callback(hello_message_received_handler_type, void_handler_type);

			ep_hello_context_map m_ep_hello_contexts;
			boost::asio::strand m_greet_strand;
			greet_memory_pool m_greet_memory_pool;

			bool m_accept_hello_messages_default;
			hello_message_received_handler_type m_hello_message_received_handler;

		private: // PRESENTATION messages

			typedef memory_pool<4096, 4> presentation_memory_pool;
			typedef std::map<ep_type, presentation_store> presentation_store_map;

			bool has_presentation_store_for(const ep_type&) const;
			void do_introduce_to(const ep_type&, simple_handler_type);
			void do_get_presentation(const ep_type&, optional_presentation_store_handler_type);
			void do_set_presentation(const ep_type&, cert_type, cert_type, void_handler_type);
			void do_clear_presentation(const ep_type&, void_handler_type);
			void handle_presentation_message_from(const presentation_message&, const ep_type&);
			void do_handle_presentation(const ep_type&, cert_type, cert_type);

			void do_set_presentation_message_received_callback(presentation_message_received_handler_type, void_handler_type);

			boost::asio::strand m_presentation_strand;
			presentation_memory_pool m_presentation_memory_pool;

			presentation_store_map m_presentation_store_map;

			bool m_accept_presentation_messages_default;
			presentation_message_received_handler_type m_presentation_message_received_handler;

		private: // SESSION_REQUEST messages

			typedef std::map<ep_type, session_pair> session_pair_map;

			static cipher_algorithm_type get_first_common_supported_cipher_algorithm(const cipher_algorithm_list_type&, const cipher_algorithm_list_type&, cipher_algorithm_type);

			void do_request_clear_session(const ep_type&, simple_handler_type);
			void do_request_session(const ep_type&, simple_handler_type, boost::asio::const_buffer);
			void handle_session_request_message_from(socket_memory_pool::shared_buffer_type, const session_request_message&, const ep_type&);
			void do_handle_session_request(const ep_type&, const session_request_message&);
			void handle_clear_session_request_message_from(socket_memory_pool::shared_buffer_type, const clear_session_request_message&, const ep_type&);
			void do_handle_clear_session_request(const ep_type&, const clear_session_request_message&);

			void do_set_accept_session_request_messages_default(bool, void_handler_type);
			void do_set_cipher_capabilities(cipher_algorithm_list_type, void_handler_type);
			void do_set_session_request_message_received_callback(session_request_received_handler_type, void_handler_type);

			boost::asio::strand m_session_strand;

			session_pair_map m_session_map;

			bool m_accept_session_request_messages_default;
			cipher_algorithm_list_type m_cipher_capabilities;
			session_request_received_handler_type m_session_request_message_received_handler;

		private: // SESSION messages

			void do_send_clear_session(const ep_type&, session_store::session_number_type);
			void do_send_session(const ep_type&, boost::asio::const_buffer);

		private: // DATA messages

			void handle_data_message_from(const data_message&, const ep_type&);
	};
}

#endif /* FSCP_SERVER_HPP */

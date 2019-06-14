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
 * \file server.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#ifndef FSCP_SERVER_HPP
#define FSCP_SERVER_HPP

#include <boost/asio.hpp>

#include "identity_store.hpp"
#include "shared_buffer.hpp"
#include "presentation_store.hpp"
#include "peer_session.hpp"
#include "logger.hpp"

#ifdef USE_UPNP
#include "miniupnpcplus/upnp_device.hpp"
#endif

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>

#include <set>
#include <map>
#include <queue>
#include <iostream>

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
	class server
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

			/**
			 * \brief The socket type.
			 */
			typedef boost::asio::ip::udp::socket socket_type;

			// Handlers

			/**
			 * \brief A void operation handler.
			 */
			typedef boost::function<void ()> void_handler_type;

			/**
			 * \brief An identity handler.
			 */
			typedef boost::function<void (const identity_store&)> identity_handler_type;

			/**
			 * \brief A simple operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&)> simple_handler_type;

			/**
			 * \brief A boolean operation handler.
			 */
			typedef boost::function<void (bool)> boolean_handler_type;

			/**
			 * \brief An operation handler for multiple endpoints.
			 */
			typedef boost::function<void (const std::map<ep_type, boost::system::error_code>&)> multiple_endpoints_handler_type;

			/**
			 * \brief A duration operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&, const boost::posix_time::time_duration& duration)> duration_handler_type;

			/**
			 * \brief An optional presentation store handler.
			 */
			typedef boost::function<void (const boost::optional<presentation_store>&)> optional_presentation_store_handler_type;

			/**
			 * \brief An endpoints handler.
			 */
			typedef boost::function<void (const std::set<ep_type>&)> endpoints_handler_type;

			// Callbacks

			/**
			 * \brief A handler for when hello requests are received.
			 * \param sender The endpoint that sent the hello message.
			 * \param default_accept The default return value.
			 * \return true to reply to the hello message, false to ignore it.
			 */
			typedef boost::function<bool (const ep_type& sender, bool default_accept)> hello_message_received_handler_type;

			/**
			 * \brief A presentation status type.
			 */
			enum presentation_status_type
			{
				PS_FIRST,
				PS_NEW,
				PS_SAME
			};

			/**
			 * \brief A handler for when presentation requests are received.
			 * \param sender The endpoint that sent the presentation message.
			 * \param sig_cert The signature certificate.
			 * \param status The presentation status.
			 * \param has_session Tell if a session currently exists with the host.
			 * \return true to accept the presentation message for the originating host.
			 */
			typedef boost::function<bool (const ep_type& sender, cert_type sig_cert, presentation_status_type status, bool has_session)> presentation_message_received_handler_type;

			/**
			 * \brief A handler for when session requests are received.
			 * \param sender The endpoint that sent the session request message.
			 * \param cs_cap The cipher suite capabilities of the remote host.
			 * \param ec_cap The elliptic curve capabilities of the remote host.
			 * \param default_accept The default return value.
			 * \return true to accept the session request.
			 */
			typedef boost::function<bool (const ep_type& sender, const cipher_suite_list_type& cs_cap, const elliptic_curve_list_type& ec_cap, bool default_accept)> session_request_received_handler_type;

			/**
			 * \brief A handler for when session messages are received.
			 * \param sender The endpoint that sent the session message.
			 * \param cs The cipher suite used for the session.
			 * \param ec The elliptic curve used for the session.
			 * \param default_accept The default return value.
			 * \return true to accept the session.
			 */
			typedef boost::function<bool (const ep_type& sender, cipher_suite_type cs, elliptic_curve_type ec, bool default_accept)> session_received_handler_type;

			/**
			 * \brief A handler for when a session establishment failed.
			 * \param host The host with which the session establishment failed.
			 * \param is_new A flag that indicates whether the session would have been a new session or a renewal.
			 */
			typedef boost::function<void (const ep_type& host, bool is_new)> session_failed_handler_type;

			/**
			 * \brief A handler for when a session establishment encountered an error.
			 * \param host The host with which the session establishment failed.
			 * \param is_new A flag that indicates whether the session would have been a new session or a renewal.
			 */
			typedef boost::function<void (const ep_type& host, bool is_new, const std::exception&)> session_error_handler_type;

			/**
			 * \brief A handler for when a session was established.
			 * \param host The host with which the session was established.
			 * \param is_new A flag that indicates whether the session is a new session or a renewal.
			 * \param cipher_suite The cipher suite used in the session.
			 * \param elliptic_curve The elliptic curve used in the session.
			 */
			typedef boost::function<void (const ep_type& host, bool is_new, const cipher_suite_type& cipher_suite, const elliptic_curve_type& elliptic_curve)> session_established_handler_type;

			/**
			 * \brief A reason for the session loss.
			 */
			enum class session_loss_reason
			{
				timeout,
				manual_termination
			};

			/**
			 * \brief A handler for when a session was lost.
			 * \param host The host with which a session was lost.
			 */
			typedef boost::function<void (const ep_type& host, session_loss_reason)> session_lost_handler_type;

			/**
			 * \brief A handler for when data is available.
			 * \param sender The endpoint that sent the data message.
			 * \param channel_number The channel number.
			 * \param buffer The buffer that own the data. Must be release as soon as possible to avoid memory starvation.
			 * \param data The sent data.
			 */
			typedef boost::function<void (const ep_type& sender, channel_number_type channel_number, SharedBuffer buffer, boost::asio::const_buffer data)> data_received_handler_type;

			/**
			 * \brief A handler for when contact requests are received.
			 * \param sender The sender of the request.
			 * \param cert The certificate associated to the answer.
			 * \param hash The requested hash.
			 * \param answer The answer endpoint.
			 * \return true to allow the request to be answered.
			 */
			typedef boost::function<bool (const ep_type& sender, cert_type cert, hash_type hash, const ep_type& answer)> contact_request_received_handler_type;

			/**
			 * \brief A handler for when contacts are received.
			 * \param sender The sender of the information.
			 * \param hash The hash.
			 * \param answer The answer endpoint.
			 */
			typedef boost::function<void (const ep_type& sender, hash_type hash, const ep_type& answer)> contact_received_handler_type;

			// Public methods

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param _logger The logger to use. It must remain valid during the lifetime of the fscp::server.
			 * \param identity The identity store.
			 */
			server(boost::asio::io_service& io_service, fscp::logger& _logger, const identity_store& identity);

			/**
			 * \brief Get the underlying socket.
			 */
			socket_type& get_socket()
			{
				return m_socket;
			}

			/**
			 * \brief Get the associated io_service.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service()
			{
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
				return reinterpret_cast<boost::asio::io_context&>(get_socket().get_executor().context());
#else
				return get_socket().get_io_service();
#endif
			}

			/**
			 * \brief Get the identity of the server.
			 * \return The identity.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			const identity_store& get_identity() const
			{
				return m_identity_store;
			}

			/**
			 * \brief Get the identity of the server.
			 * \param handler The handler to call when the result is available.
			 */
			void async_get_identity(identity_handler_type handler)
			{
				m_socket_strand.post(boost::bind(&server::do_get_identity, this, handler));
			}

			/**
			 * \brief Get the identity of the server.
			 * \return The identity.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			identity_store sync_get_identity();

			/**
			 * \brief Set the identity of the server.
			 * \param identity The identity.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_identity(const identity_store& identity)
			{
				m_identity_store = identity;
			}

			/**
			 * \brief Set the identity of the server.
			 * \param identity The identity.
			 * \param handler The handler to call when the change is effective.
			 */
			void async_set_identity(const identity_store& identity, void_handler_type handler = void_handler_type())
			{
				m_socket_strand.post(boost::bind(&server::do_set_identity, this, identity, handler));
			}

			/**
			 * \brief Set the identity of the server.
			 * \param identity The identity.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_identity(const identity_store& identity);

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

#ifdef USE_UPNP
			/**
			 * \brief Use UPnP to punch hole NAT.
			 * \param port local port.
			 */
			void upnp_punch_hole(uint16_t port);
#endif

			/**
			 * \brief Greet an host.
			 * \param target The target to greet.
			 * \param handler The handler to call when a reply was received, an error occurred or the request timed out.
			 * \param timeout The maximum time to wait for a reply.
			 */
			void async_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

			/**
			 * \brief Cancel all pending greetings.
			 */
			void cancel_all_greetings()
			{
				m_greet_strand.post(boost::bind(&server::do_cancel_all_greetings, this));
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
				m_greet_strand.post(boost::bind(&server::do_set_accept_hello_messages_default, this, value, handler));
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
			 * \brief Set maximum hello message from one host per second.
			 * \param max_per_second value to set.
			 */
			void set_hello_max_per_second(size_t max_per_second)
			{
				m_hello_max_per_second = max_per_second;
			}

			/**
			 * \brief Set the hello message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_hello_message_received_callback(hello_message_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_greet_strand.post(boost::bind(&server::do_set_hello_message_received_callback, this, callback, handler));
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
			 * \param handler The handler to call when the presentation message is sent or an error occurred.
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
			 * \brief Resend a presentation message to all the known peers.
			 * \param handler The handler to call when all the presentation messages were sent or an error occurred.
			 */
			void async_reintroduce_to_all(multiple_endpoints_handler_type handler);

			/**
			 * \brief Send a presentation message to the specified target.
			 * \return A map of error codes.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_reintroduce_to_all();

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
			 * \param pre_shared_key The pre-shared key to use for this host.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key);

			/**
			 * \brief Set the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param signature_certificate The signature certificate.
			 * \param pre_shared_key The pre-shared key to use for this host.
			 * \param handler The handler to call when the presentation was set for the given host.
			 */
			void async_set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key, void_handler_type handler = void_handler_type());

			/**
			 * \brief Set the presentation for the given host.
			 * \param target The host to set the presentation for.
			 * \param signature_certificate The signature certificate.
			 * \param pre_shared_key The pre-shared key to use for this host.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_presentation(const ep_type& target, cert_type signature_certificate, const cryptoplus::buffer& pre_shared_key);

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
			 * \brief Set maximum presentation message from one host per second.
			 * \param max_per_second value to set.
			 */
			void set_presentation_max_per_second(size_t max_per_second)
			{
				m_presentation_max_per_second = max_per_second;
			}

			/**
			 * \brief Set the presentation message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_presentation_message_received_callback(presentation_message_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_presentation_strand.post(boost::bind(&server::do_set_presentation_message_received_callback, this, callback, handler));
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
			 * \param handler The handler to call when the request was sent or an error occurred.
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
			 * \brief Close an established session.
			 * \param target The remote host corresponding to the session to close.
			 * \param handler The handler to call when the session has been closed.
			 */
			void async_close_session(const ep_type& target, simple_handler_type handler);

			/**
			 * \brief Close an established session.
			 * \param target The remote host corresponding to the session to close.
			 * \return An error code indicating the result of the operation.
			 */
			boost::system::error_code sync_close_session(const ep_type& target);

			/**
			 * \brief Get a list of endpoints to which the server has an active session.
			 * \param handler The handler to call with the endpoints list.
			 */
			void async_get_session_endpoints(endpoints_handler_type handler)
			{
				m_session_strand.post(boost::bind(&server::do_get_session_endpoints, this, handler));
			}

			/**
			 * \brief Get a list of endpoints to which the server has an active session.
			 * \return The list of endpoints.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::set<ep_type> sync_get_session_endpoints();

			/**
			 * \brief Check if a session exists with the specified endpoint.
			 * \param handler The handler to call with the result.
			 */
			void async_has_session_with_endpoint(const ep_type& host, boolean_handler_type handler)
			{
				m_session_strand.post(boost::bind(&server::do_has_session_with_endpoint, this, host, handler));
			}

			/**
			 * \brief Check if a session exists with the specified endpoint.
			 * \param handler The handler to call with the result.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			bool sync_has_session_with_endpoint(const ep_type& host);

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
				m_session_strand.post(boost::bind(&server::do_set_accept_session_request_messages_default, this, value, handler));
			}

			/**
			 * \brief Set the default acceptance behavior of incoming session requests.
			 * \param value The default value.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_accept_session_request_messages_default(bool value);

			/**
			 * \brief Set the cipher suites.
			 * \param cipher_suites The cipher suites.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_cipher_suites(const cipher_suite_list_type& cipher_suites)
			{
				m_cipher_suites = cipher_suites;
			}

			/**
			 * \brief Set the cipher suites.
			 * \param cipher_suites The cipher suites.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_cipher_suites(const cipher_suite_list_type& cipher_suites, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_cipher_suites, this, cipher_suites, handler));
			}

			/**
			 * \brief Set the cipher suites.
			 * \param cipher_suites The cipher suites.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_cipher_suites(const cipher_suite_list_type& cipher_suites);

			/**
			 * \brief Set the elliptic curves.
			 * \param elliptic_curves The elliptic curves.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_elliptic_curves(const elliptic_curve_list_type& elliptic_curves)
			{
				m_elliptic_curves = get_supported_elliptic_curves(elliptic_curves);
			}

			/**
			 * \brief Set the elliptic curves.
			 * \param elliptic_curves The elliptic curves.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_elliptic_curves(const elliptic_curve_list_type& elliptic_curves, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_elliptic_curves, this, elliptic_curves, handler));
			}

			/**
			 * \brief Set the elliptic curves.
			 * \param elliptic_curves The elliptic curves.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_elliptic_curves(const elliptic_curve_list_type& elliptic_curves);

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
				m_session_strand.post(boost::bind(&server::do_set_session_request_message_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session request message received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_request_message_received_callback(session_request_received_handler_type callback);

			/**
			 * \brief Set the default acceptance behavior of incoming sessions.
			 * \param value The default value.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_accept_session_messages_default(bool value)
			{
				m_accept_session_messages_default = value;
			}

			/**
			 * \brief Set the default acceptance behavior of incoming sessions.
			 * \param value The default value.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_accept_session_messages_default(bool value, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_accept_session_messages_default, this, value, handler));
			}

			/**
			 * \brief Set the default acceptance behavior of incoming sessions.
			 * \param value The default value.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_accept_session_messages_default(bool value);

			/**
			 * \brief Set the session message received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_message_received_callback(session_received_handler_type callback)
			{
				m_session_message_received_handler = callback;
			}

			/**
			 * \brief Set the session message received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_message_received_callback(session_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_session_message_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session message received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_message_received_callback(session_received_handler_type callback);

			/**
			 * \brief Set the session failed callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_failed_callback(session_failed_handler_type callback)
			{
				m_session_failed_handler = callback;
			}

			/**
			 * \brief Set the session failed callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_failed_callback(session_failed_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_session_failed_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session failed callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_failed_callback(session_failed_handler_type callback);

			/**
			 * \brief Set the session error callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_error_callback(session_error_handler_type callback)
			{
				m_session_error_handler = callback;
			}

			/**
			 * \brief Set the session error callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_error_callback(session_error_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_session_error_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session failed callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_error_callback(session_error_handler_type callback);

			/**
			 * \brief Set the session established callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_established_callback(session_established_handler_type callback)
			{
				m_session_established_handler = callback;
			}

			/**
			 * \brief Set the session established callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_established_callback(session_established_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_session_established_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session established callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_established_callback(session_established_handler_type callback);

			/**
			 * \brief Set the session lost callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_session_lost_callback(session_lost_handler_type callback)
			{
				m_session_lost_handler = callback;
			}

			/**
			 * \brief Set the session lost callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_session_lost_callback(session_lost_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_session_lost_callback, this, callback, handler));
			}

			/**
			 * \brief Set the session lost callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_session_lost_callback(session_lost_handler_type callback);

			/**
			 * \brief Send data to a host.
			 * \param target The target host.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \param handler The handler to call when the data was sent or an error occurred.
			 */
			void async_send_data(const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data, simple_handler_type handler);

			/**
			 * \brief Send data to a host.
			 * \param target The target host.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \return The error code associated to the send operation.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::system::error_code sync_send_data(const ep_type& target, channel_number_type channel_number, boost::asio::const_buffer data);

			/**
			 * \brief Send data to a list of hosts.
			 * \param targets The list of hosts.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_data_to_list(const std::set<ep_type>& targets, channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler);

			/**
			 * \brief Send data to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			template <typename EPIterator>
			void async_send_data_to_list(EPIterator begin, EPIterator end, channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler)
			{
				async_send_data_to_list(std::set<ep_type>(begin, end), channel_number, data, handler);
			}

			/**
			 * \brief Send data to a list of hosts.
			 * \param targets The list of hosts.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_data_to_list(const std::set<ep_type>& targets, channel_number_type channel_number, boost::asio::const_buffer data);

			/**
			 * \brief Send data to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			template <typename EPIterator>
			std::map<ep_type, boost::system::error_code> sync_send_data_to_list(EPIterator begin, EPIterator end, channel_number_type channel_number, boost::asio::const_buffer data)
			{
				return sync_send_data_to_list(std::set<ep_type>(begin, end), channel_number, data);
			}

			/**
			 * \brief Send data to all hosts.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_data_to_all(channel_number_type channel_number, boost::asio::const_buffer data, multiple_endpoints_handler_type handler)
			{
				m_session_strand.post(boost::bind(&server::do_send_data_to_all, this, channel_number, data, handler));
			}

			/**
			 * \brief Send data to all hosts.
			 * \param channel_number The channel number.
			 * \param data The data to send.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_data_to_all(channel_number_type channel_number, boost::asio::const_buffer data);

			/**
			 * \brief Send a contact request to an host.
			 * \param target The target host.
			 * \param hash_list The hashes to request.
			 * \param handler The handler to call when the data was sent or an error occurred.
			 */
			void async_send_contact_request(const ep_type& target, const hash_list_type& hash_list, simple_handler_type handler);

			/**
			 * \brief Send a contact request to an host.
			 * \param target The target host.
			 * \param hash_list The hashes to request.
			 * \return The error code associated to the send operation.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::system::error_code sync_send_contact_request(const ep_type& target, const hash_list_type& hash_list);

			/**
			 * \brief Send a contact request to a list of hosts.
			 * \param targets The list of hosts.
			 * \param hash_list The hashes to request.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_contact_request_to_list(const std::set<ep_type>& targets, const hash_list_type& hash_list, multiple_endpoints_handler_type handler);

			/**
			 * \brief Send a contact request to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param hash_list The hashes to request.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			template <typename EPIterator>
			void async_send_contact_request_to_list(EPIterator begin, EPIterator end, const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
			{
				async_send_contact_request_to_list(std::set<ep_type>(begin, end), hash_list, handler);
			}

			/**
			 * \brief Send a contact request to a list of hosts.
			 * \param targets The list of hosts.
			 * \param hash_list The hashes to request.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_contact_request_to_list(const std::set<ep_type>& targets, const hash_list_type& hash_list);

			/**
			 * \brief Send a contact request to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param hash_list The hashes to request.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			template <typename EPIterator>
			std::map<ep_type, boost::system::error_code> sync_send_contact_request_to_list(EPIterator begin, EPIterator end, const hash_list_type& hash_list)
			{
				return sync_send_contact_request_to_list(std::set<ep_type>(begin, end), hash_list);
			}

			/**
			 * \brief Send a contact request to all hosts.
			 * \param hash_list The hashes to request.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_contact_request_to_all(const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
			{
				m_session_strand.post(boost::bind(&server::do_send_contact_request_to_all, this, hash_list, handler));
			}

			/**
			 * \brief Send a contact request to all hosts.
			 * \param hash_list The hashes to request.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_contact_request_to_all(const hash_list_type& hash_list);

			/**
			 * \brief Send contact information to an host.
			 * \param target The target host.
			 * \param contact_map The contact information.
			 * \param handler The handler to call when the data was sent or an error occurred.
			 */
			void async_send_contact(const ep_type& target, const contact_map_type& contact_map, simple_handler_type handler);

			/**
			 * \brief Send contact information to an host.
			 * \param target The target host.
			 * \param contact_map The contact information.
			 * \return The error code associated to the send operation.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			boost::system::error_code sync_send_contact(const ep_type& target, const contact_map_type& contact_map);

			/**
			 * \brief Send contact information to a list of hosts.
			 * \param targets The list of hosts.
			 * \param contact_map The contact information.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_contact_to_list(const std::set<ep_type>& targets, const contact_map_type& contact_map, multiple_endpoints_handler_type handler);

			/**
			 * \brief Send contact information to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param contact_map The contact information.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			template <typename EPIterator>
			void async_send_contact_to_list(EPIterator begin, EPIterator end, const contact_map_type& contact_map, multiple_endpoints_handler_type handler)
			{
				async_send_contact_to_list(std::set<ep_type>(begin, end), contact_map, handler);
			}

			/**
			 * \brief Send contact information to a list of hosts.
			 * \param targets The list of hosts.
			 * \param contact_map The contact information.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_contact_to_list(const std::set<ep_type>& targets, const contact_map_type& contact_map);

			/**
			 * \brief Send contact information to a list of hosts.
			 * \param begin An iterator to the first endpoint of the list.
			 * \param end An iterator past the last endpoint of the list.
			 * \param contact_map The contact information.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			template <typename EPIterator>
			std::map<ep_type, boost::system::error_code> sync_send_contact_to_list(EPIterator begin, EPIterator end, const contact_map_type& contact_map)
			{
				return sync_send_contact_to_list(std::set<ep_type>(begin, end), contact_map);
			}

			/**
			 * \brief Send contact information to all hosts.
			 * \param contact_map The contact information.
			 * \param handler The handler to call when the data was sent to all hosts or an error occurred.
			 */
			void async_send_contact_to_all(const contact_map_type& contact_map, multiple_endpoints_handler_type handler)
			{
				m_session_strand.post(boost::bind(&server::do_send_contact_to_all, this, contact_map, handler));
			}

			/**
			 * \brief Send contact information to all hosts.
			 * \param contact_map The contact information.
			 * \return A map of individual results.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			std::map<ep_type, boost::system::error_code> sync_send_contact_to_all(const contact_map_type& contact_map);

			/**
			 * \brief Set the data received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_data_received_callback(data_received_handler_type callback)
			{
				m_data_received_handler = callback;
			}

			/**
			 * \brief Set the data received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_data_received_callback(data_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_session_strand.post(boost::bind(&server::do_set_data_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the data received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_data_received_callback(data_received_handler_type callback);

			/**
			 * \brief Set the contact request received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_contact_request_received_callback(contact_request_received_handler_type callback)
			{
				m_contact_request_message_received_handler = callback;
			}

			/**
			 * \brief Set the contact request received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_contact_request_received_callback(contact_request_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_presentation_strand.post(boost::bind(&server::do_set_contact_request_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the contact request received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_contact_request_received_callback(contact_request_received_handler_type callback);

			/**
			 * \brief Set the contact received callback.
			 * \param callback The callback.
			 * \warning This method is *NOT* thread-safe and should be called only before the server is started.
			 */
			void set_contact_received_callback(contact_received_handler_type callback)
			{
				m_contact_message_received_handler = callback;
			}

			/**
			 * \brief Set the contact received callback.
			 * \param callback The callback.
			 * \param handler The handler to call when the change was made effective.
			 */
			void async_set_contact_received_callback(contact_received_handler_type callback, void_handler_type handler = void_handler_type())
			{
				m_contact_strand.post(boost::bind(&server::do_set_contact_received_callback, this, callback, handler));
			}

			/**
			 * \brief Set the contact received callback.
			 * \param callback The callback.
			 * \warning If the io_service is not being run, the call will block undefinitely.
			 * \warning This function must **NEVER** be called from inside a thread that runs one of the server's handlers.
			 */
			void sync_set_contact_received_callback(contact_received_handler_type callback);

		private:
			fscp::logger& m_logger;

		private:

			identity_store m_identity_store;

			void do_get_identity(identity_handler_type);
			void do_set_identity(const identity_store&, void_handler_type);

		private:
			elliptic_curve_list_type get_supported_elliptic_curves(const elliptic_curve_list_type& curves);

			void async_receive_from()
			{
				m_socket_strand.post(boost::bind(&server::do_async_receive_from, this));
			}

			void do_async_receive_from();
			void handle_receive_from(const identity_store&, boost::shared_ptr<ep_type>, SharedBuffer, const boost::system::error_code&, size_t);

			ep_type to_socket_format(const ep_type& ep);

			void async_send_to(const SharedBuffer& data, const size_t size, const ep_type& target, simple_handler_type handler)
			{
				const void_handler_type write_handler = [this, data, size, target, handler] () {
					m_socket.async_send_to(buffer(data, size), to_socket_format(target), 0, [data, handler] (const boost::system::error_code& ec, size_t) {
						handler(ec);
					});
				};

				m_write_queue_strand.post(boost::bind(&server::push_write, this, write_handler));
			}

			void push_write(void_handler_type);
			void pop_write();

			socket_type m_socket;
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_socket_strand;
			boost::asio::io_context::strand m_write_queue_strand;
#else
			boost::asio::strand m_socket_strand;
			boost::asio::strand m_write_queue_strand;
#endif
			std::queue<void_handler_type> m_write_queue;
			std::list<SharedBuffer> m_socket_buffers;

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

			typedef std::map<ep_type, ep_hello_context_type> ep_hello_context_map;

			void do_greet(const ep_type&, duration_handler_type, const boost::posix_time::time_duration&);
			void do_greet_handler(const ep_type&, uint32_t, duration_handler_type, const boost::posix_time::time_duration&, const boost::system::error_code&);
			void do_greet_timeout(const ep_type&, uint32_t, duration_handler_type, const boost::system::error_code&);
			void do_cancel_all_greetings();

			void handle_hello_message_from(const hello_message&, const ep_type&);
			void do_handle_hello_request(const ep_type&, uint32_t);
			void do_handle_hello_response(const ep_type&, uint32_t);

			void do_set_accept_hello_messages_default(bool, void_handler_type);
			void do_set_hello_message_received_callback(hello_message_received_handler_type, void_handler_type);

			/**
			 * \brief Reset hello limit.
			 * \param ec error code.
			 */
			void do_hello_reset_limit(const boost::system::error_code& ec);

			ep_hello_context_map m_ep_hello_contexts;
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_greet_strand;
#else
			boost::asio::strand m_greet_strand;
#endif
			bool m_accept_hello_messages_default;
			hello_message_received_handler_type m_hello_message_received_handler;

			/**
			 * \brief Current number of session request sent per endpoint.
			 */
			std::map<ep_type, size_t> m_hello_requests_map;

			/**
			 * \brief Timer for reesting hello requests limit.
			 */
			boost::asio::deadline_timer m_hello_limit_timer;

			/**
			 * \brief Maximum hello message from one host per second.
			 */
			size_t m_hello_max_per_second;

		private: // PRESENTATION messages

			typedef std::map<ep_type, presentation_store> presentation_store_map;

			bool has_presentation_store_for(const ep_type&) const;
			void do_introduce_to(const ep_type&, simple_handler_type);
			void do_reintroduce_to_all(multiple_endpoints_handler_type);
			void do_get_presentation(const ep_type&, optional_presentation_store_handler_type);
			void do_set_presentation(const ep_type&, cert_type, const cryptoplus::buffer&, void_handler_type);
			void do_clear_presentation(const ep_type&, void_handler_type);
			void handle_presentation_message_from(const identity_store&, const presentation_message&, const ep_type&);
			void do_handle_presentation(const identity_store& identity, const ep_type&, bool, cert_type);

			void do_set_presentation_message_received_callback(presentation_message_received_handler_type, void_handler_type);

			/**
			 * \brief Reset presentation limit.
			 * \param ec error code.
			 */
			void do_presentation_reset_limit(const boost::system::error_code& ec);

			// This strand is also used by session requests and session messages during the cipherment/decipherment phase.
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_presentation_strand;
#else
			boost::asio::strand m_presentation_strand;
#endif
			presentation_store_map m_presentation_store_map;
			presentation_message_received_handler_type m_presentation_message_received_handler;

			/**
			 * \brief Current number of session request sent per endpoint.
			 */
			std::map<ep_type, size_t> m_presentation_requests_map;

			/**
			 * \brief Timer for reesting presentation requests limit.
			 */
			boost::asio::deadline_timer m_presentation_limit_timer;

			/**
			 * \brief Maximum presentation message from one host per second.
			 */
			size_t m_presentation_max_per_second;

		private: // SESSION_REQUEST messages

			typedef std::map<ep_type, peer_session> peer_session_map_type;

			static cipher_suite_type get_first_common_supported_cipher_suite(const cipher_suite_list_type&, const cipher_suite_list_type&, cipher_suite_type);
			static elliptic_curve_type get_first_common_supported_elliptic_curve(const elliptic_curve_list_type&, const elliptic_curve_list_type&, elliptic_curve_type);

			void do_request_session(const identity_store&, const ep_type&, simple_handler_type);
			void do_close_session(const ep_type&, simple_handler_type);
			void do_handle_session_request(SharedBuffer, const identity_store&, const ep_type&, const session_request_message&);
			void do_handle_verified_session_request(const identity_store&, const ep_type&, const session_request_message&);

			std::set<ep_type> get_session_endpoints() const;
			bool has_session_with_endpoint(const ep_type&);
			void do_get_session_endpoints(endpoints_handler_type);
			void do_has_session_with_endpoint(const ep_type&, boolean_handler_type);
			void do_set_accept_session_request_messages_default(bool, void_handler_type);
			void do_set_cipher_suites(cipher_suite_list_type, void_handler_type);
			void do_set_elliptic_curves(elliptic_curve_list_type, void_handler_type);
			void do_set_session_request_message_received_callback(session_request_received_handler_type, void_handler_type);

			// This strand is common to session requests, session messages and data messages.
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_session_strand;
#else
			boost::asio::strand m_session_strand;
#endif

			peer_session_map_type m_peer_sessions;
			std::list<SharedBuffer> m_session_buffers;

			bool m_accept_session_request_messages_default;
			cipher_suite_list_type m_cipher_suites;
			elliptic_curve_list_type m_elliptic_curves;
			session_request_received_handler_type m_session_request_message_received_handler;

		private: // SESSION messages

			void do_send_session(const identity_store&, const ep_type&, const peer_session::session_parameters&);
			void do_handle_session(SharedBuffer, const identity_store&, const ep_type&, const session_message&);
			void do_handle_verified_session(const identity_store&, const ep_type&, const session_message&);

			void do_set_accept_session_messages_default(bool, void_handler_type);
			void do_set_session_message_received_callback(session_received_handler_type, void_handler_type);
			void do_set_session_failed_callback(session_failed_handler_type, void_handler_type);
			void do_set_session_error_callback(session_error_handler_type, void_handler_type);
			void do_set_session_established_callback(session_established_handler_type, void_handler_type);
			void do_set_session_lost_callback(session_lost_handler_type, void_handler_type);

			bool m_accept_session_messages_default;
			session_received_handler_type m_session_message_received_handler;
			session_failed_handler_type m_session_failed_handler;
			session_error_handler_type m_session_error_handler;
			session_established_handler_type m_session_established_handler;
			session_lost_handler_type m_session_lost_handler;

		private: // DATA messages

			void do_send_data(const ep_type&, channel_number_type, boost::asio::const_buffer, simple_handler_type);
			void do_send_data_to_list(const std::set<ep_type>&, channel_number_type, boost::asio::const_buffer, multiple_endpoints_handler_type);
			void do_send_data_to_all(channel_number_type, boost::asio::const_buffer, multiple_endpoints_handler_type);
			void do_send_data_to_session(peer_session&, const ep_type&, channel_number_type, boost::asio::const_buffer, simple_handler_type);
			void do_send_contact_request(const ep_type&, const hash_list_type&, simple_handler_type);
			void do_send_contact_request_to_list(const std::set<ep_type>&, const hash_list_type&, multiple_endpoints_handler_type);
			void do_send_contact_request_to_all(const hash_list_type&, multiple_endpoints_handler_type);
			void do_send_contact_request_to_session(peer_session&, const ep_type&, const hash_list_type&, simple_handler_type);
			void do_send_contact(const ep_type&, const contact_map_type&, simple_handler_type);
			void do_send_contact_to_list(const std::set<ep_type>&, const contact_map_type&, multiple_endpoints_handler_type);
			void do_send_contact_to_all(const contact_map_type&, multiple_endpoints_handler_type);
			void do_send_contact_to_session(peer_session&, const ep_type&, const contact_map_type&, simple_handler_type);
			void handle_data_message_from(const identity_store&, SharedBuffer, const data_message&, const ep_type&);
			void do_handle_data(const identity_store&, const ep_type&, const data_message&);
			void do_handle_data_message(const ep_type&, message_type, SharedBuffer, boost::asio::const_buffer);
			void do_handle_contact_request(const ep_type&, const std::set<hash_type>&);
			void do_handle_contact(const ep_type&, const contact_map_type&);

			void do_set_data_received_callback(data_received_handler_type, void_handler_type);
			void do_set_contact_request_received_callback(contact_request_received_handler_type, void_handler_type);
			void do_set_contact_received_callback(contact_received_handler_type, void_handler_type);

#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_contact_strand;
#else
			boost::asio::strand m_contact_strand;
#endif

			data_received_handler_type m_data_received_handler;
			contact_request_received_handler_type m_contact_request_message_received_handler;
			contact_received_handler_type m_contact_message_received_handler;

		private: // Keep-alive

			void do_check_keep_alive(const boost::system::error_code&);
			void do_send_keep_alive(const ep_type&, simple_handler_type);

			boost::asio::deadline_timer m_keep_alive_timer;

		private: // Misc

#ifdef USE_UPNP
			boost::shared_ptr<miniupnpcplus::upnp_device> m_upnp;
#endif

			friend std::ostream& operator<<(std::ostream& os, presentation_status_type status)
			{
				switch (status)
				{
					case PS_FIRST:
						return os << "first presentation";
					case PS_NEW:
						return os << "new presentation";
					case PS_SAME:
						return os << "same presentation";
				}

				assert(false);
				return os << "invalid presentation status";
			}
	};

	std::ostream& operator<<(std::ostream& os, server::session_loss_reason value);
}

#endif /* FSCP_SERVER_HPP */

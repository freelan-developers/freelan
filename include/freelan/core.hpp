/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file core.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan core class.
 */

#ifndef FREELAN_CORE_HPP
#define FREELAN_CORE_HPP

#include <vector>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>

#include <cryptoplus/x509/store.hpp>
#include <cryptoplus/x509/store_context.hpp>
#include <asiotap/asiotap.hpp>
#include <asiotap/osi/arp_proxy.hpp>
#include <asiotap/osi/dhcp_proxy.hpp>
#include <asiotap/osi/complex_filter.hpp>
#include <fscp/fscp.hpp>

#include "configuration.hpp"
#include "switch.hpp"
#include "logger.hpp"

namespace freelan
{
	class client;

	/**
	 * \brief The core class.
	 */
	class core
	{
		public:

			/**
			 * \brief The ethernet address type.
			 */
			typedef freelan::tap_adapter_configuration::ethernet_address_type ethernet_address_type;

			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef fscp::server::cert_type cert_type;

			/**
			 * \brief The certificate revocation list type.
			 */
			typedef security_configuration::crl_type crl_type;

			/**
			 * \brief The identity store type.
			 */
			typedef fscp::identity_store identity_store;

			/**
			 * \brief The contact period.
			 */
			static const boost::posix_time::time_duration CONTACT_PERIOD;

			/**
			 * \brief The dynamic contact period.
			 */
			static const boost::posix_time::time_duration DYNAMIC_CONTACT_PERIOD;

			/**
			 * \brief The default service.
			 */
			static const std::string DEFAULT_SERVICE;

			/**
			 * \brief The configuration change callback.
			 */
			typedef boost::function<void (const freelan::configuration&)> configuration_update_callback;

			/**
			 * \brief The open callback.
			 */
			typedef boost::function<void ()> open_callback;

			/**
			 * \brief The close callback.
			 */
			typedef boost::function<void ()> close_callback;

			/**
			 * \brief A session established callback.
			 * \param host The host with which a session is established.
			 */
			typedef boost::function<void (const ep_type& host)> session_established_callback;

			/**
			 * \brief A session lost callback.
			 * \param host The host with which a session was lost.
			 */
			typedef boost::function<void (const ep_type& host)> session_lost_callback;

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to bind to.
			 * \param configuration The configuration to use.
			 * \param _logger The logger to use for logging.
			 */
			core(boost::asio::io_service& io_service, const freelan::configuration& configuration, const freelan::logger& _logger);

			/**
			 * \brief Get the configuration.
			 * \return The current configuration.
			 */
			const freelan::configuration& configuration() const;

			/**
			 * \brief Check if the core has a tap adapter.
			 * \return true if the core has a tap adapter.
			 */
			bool has_tap_adapter() const;

			/**
			 * \brief Get the associated tap adapter.
			 * \return The associated tap adapter.
			 *
			 * \warning Calling this method while has_tap_adapter() is false is undefined behavior.
			 */
			const asiotap::tap_adapter& tap_adapter() const;

			/**
			 * \brief Check if the core has a server.
			 * \return true if the core has a server.
			 */
			bool has_server() const;

			/**
			 * \brief Get the associated server.
			 * \return The associated server.
			 *
			 * \warning Calling this method while has_server() is false is undefined behavior.
			 */
			const fscp::server& server() const;

			/**
			 * \brief Get the associated logger instance.
			 * \return The associated logger instance.
			 */
			freelan::logger& logger();

			/**
			 * \brief Set the configuration update callback.
			 * \param callback The callback.
			 *
			 * This callback is called when the configuration is updated.
			 *
			 * This is a very good time to save the configuration on disk if you need to.
			 */
			void set_configuration_update_callback(configuration_update_callback callback);

			/**
			 * \brief Set the open callback.
			 * \param callback The callback.
			 *
			 * This callback is called when the core was just opened.
			 */
			void set_open_callback(open_callback callback);

			/**
			 * \brief Set the close callback.
			 * \param callback The callback.
			 *
			 * This callback is called when the core was just closed.
			 */
			void set_close_callback(close_callback callback);

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
			 * \brief Open the current core instance.
			 */
			void open();

			/**
			 * \brief Close the current core instance.
			 */
			void close();

			/**
			 * \brief Add a log entry to the attached logger.
			 * \param level The log level of the entry. If level is inferior to the
			 * current log level of the attached logger, the call will have no
			 * effect.
			 * \param msg The message to log. It will be copied and can be deleted
			 * safely at any time.
			 */
			void log(freelan::log_level level, const std::string& msg);

		private:

			// Setting up
			boost::asio::io_service& m_io_service;

			// The running flag
			volatile bool m_running;
			void do_close();

			// FSCP methods
			void async_greet(const ep_type&);
			bool on_hello_request(const ep_type&, bool);
			void on_hello_response(const ep_type&, const boost::posix_time::time_duration&, bool);
			bool on_presentation(const ep_type&, cert_type, cert_type, bool);
			bool on_session_request(const ep_type&, bool);
			void on_session_established(const ep_type&);
			void on_session_lost(const ep_type&);
			void on_data(const ep_type&, fscp::channel_number_type, boost::asio::const_buffer);
			bool on_contact_request(const ep_type&, cert_type, const ep_type&);
			void on_contact(const ep_type&, cert_type, const ep_type&);
			void on_ethernet_data(const ep_type&, boost::asio::const_buffer);
			void on_network_error(const ep_type&, const boost::system::error_code&);

			// Tap adapter methods
			void tap_adapter_read_done(asiotap::tap_adapter&, const boost::system::error_code&, size_t);

			// Other methods
			void do_greet(const ep_type& ep);
			void do_greet(const boost::system::error_code&, boost::asio::ip::udp::resolver::iterator, const freelan::fscp_configuration::endpoint&);
			void do_contact();
			void do_contact(const fscp_configuration::endpoint&);
			void do_dynamic_contact();
			void do_dynamic_contact(cert_type cert);
			void do_periodic_contact(const boost::system::error_code&);
			void do_periodic_dynamic_contact(const boost::system::error_code&);
			void do_check_certificate_expiration(const boost::system::error_code&);

			// Members
			freelan::configuration m_configuration;
			freelan::logger m_logger;

			// FSCP
			void create_server();
			boost::scoped_ptr<fscp::server> m_server;
			boost::asio::ip::udp::resolver m_resolver;
			boost::asio::deadline_timer m_contact_timer;
			boost::asio::deadline_timer m_dynamic_contact_timer;

			// Tap adapter
			void create_tap_adapter();
			boost::scoped_ptr<asiotap::tap_adapter> m_tap_adapter;
			boost::array<unsigned char, 65536> m_tap_adapter_buffer;

			// User callbacks
			configuration_update_callback m_configuration_update_callback;
			open_callback m_open_callback;
			close_callback m_close_callback;
			session_established_callback m_session_established_callback;
			session_lost_callback m_session_lost_callback;

			// Filters
			asiotap::osi::filter<asiotap::osi::ethernet_frame> m_ethernet_filter;
			asiotap::osi::complex_filter<asiotap::osi::arp_frame, asiotap::osi::ethernet_frame>::type m_arp_filter;
			asiotap::osi::complex_filter<asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_ipv4_filter;
			asiotap::osi::complex_filter<asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_udp_filter;
			asiotap::osi::complex_filter<asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_bootp_filter;
			asiotap::osi::complex_filter<asiotap::osi::dhcp_frame, asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_dhcp_filter;

			// Proxies
			typedef asiotap::osi::proxy<asiotap::osi::arp_frame> arp_proxy_type;
			typedef asiotap::osi::proxy<asiotap::osi::dhcp_frame> dhcp_proxy_type;
			boost::shared_ptr<arp_proxy_type> m_arp_proxy;
			boost::shared_ptr<dhcp_proxy_type> m_dhcp_proxy;
			boost::array<unsigned char, 2048> m_proxy_buffer;

			// Proxies related methods
			void on_proxy_data(boost::asio::const_buffer);
			bool on_arp_request(const boost::asio::ip::address_v4&, ethernet_address_type&);

			// Switch
			switch_ m_switch;

			typedef std::map<ep_type, switch_::port_type> endpoint_switch_port_map_type;
			endpoint_switch_port_map_type m_endpoint_switch_port_map;

			switch_::port_type m_tap_adapter_switch_port;

			// Certificate validation
			static const int ex_data_index;
			static int certificate_validation_callback(int, X509_STORE_CTX*);
			bool certificate_validation_method(bool, cryptoplus::x509::store_context);
			bool certificate_is_valid(cert_type cert);
			cryptoplus::x509::store m_ca_store;

			// Client
			void async_renew_certificate();
			void renew_certificate_callback(identity_store);
			boost::shared_ptr<client> m_client;
			boost::asio::deadline_timer m_certificate_expiration_timer;
	};

	inline const freelan::configuration& core::configuration() const
	{
		return m_configuration;
	}

	inline bool core::has_tap_adapter() const
	{
		return static_cast<bool>(m_tap_adapter);
	}

	inline const asiotap::tap_adapter& core::tap_adapter() const
	{
		return *m_tap_adapter;
	}

	inline bool core::has_server() const
	{
		return static_cast<bool>(m_server);
	}

	inline const fscp::server& core::server() const
	{
		return *m_server;
	}

	inline freelan::logger& core::logger()
	{
		return m_logger;
	}

	inline void core::set_configuration_update_callback(configuration_update_callback callback)
	{
		m_configuration_update_callback = callback;
	}

	inline void core::set_open_callback(open_callback callback)
	{
		m_open_callback = callback;
	}

	inline void core::set_close_callback(close_callback callback)
	{
		m_close_callback = callback;
	}

	inline void core::set_session_established_callback(session_established_callback callback)
	{
		m_session_established_callback = callback;
	}

	inline void core::set_session_lost_callback(session_lost_callback callback)
	{
		m_session_lost_callback = callback;
	}
}

#endif /* FREELAN_CORE_HPP */

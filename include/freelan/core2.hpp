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

#ifndef FREELAN_CORE2_HPP
#define FREELAN_CORE2_HPP

#include "configuration.hpp"
#include "logger.hpp"

#include <fscp/fscp.hpp>

#include <asiotap/asiotap.hpp>
#include <asiotap/osi/arp_proxy.hpp>
#include <asiotap/osi/dhcp_proxy.hpp>
#include <asiotap/osi/complex_filter.hpp>

#include <cryptoplus/x509/store.hpp>
#include <cryptoplus/x509/store_context.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

namespace freelan
{
	/**
	 * \brief The core class.
	 * All the public methods are thread-safe, unless otherwise specified.
	 *
	 * async_* methods are designed to be run from inside handlers (or callbacks).
	 * sync_* methods are designed to be run outside of the core running threads while the core is running.
	 */
	class core
	{
		public:

			// General purpose type definitions

			/**
			 * \brief The ethernet address type.
			 */
			typedef freelan::tap_adapter_configuration::ethernet_address_type ethernet_address_type;

			/**
			 * \brief The low-level endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef fscp::server::cert_type cert_type;

			/**
			 * \brief The certificate list type.
			 */
			typedef std::vector<cert_type> cert_list_type;

			/**
			 * \brief The certificate revocation list type.
			 */
			typedef security_configuration::crl_type crl_type;

			/**
			 * \brief The hash type.
			 */
			typedef fscp::hash_type hash_type;

			/**
			 * \brief The hash list type.
			 */
			typedef fscp::hash_list_type hash_list_type;

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
			 * \brief An operation handler for multiple endpoints.
			 */
			typedef boost::function<void (const std::map<ep_type, boost::system::error_code>&)> multiple_endpoints_handler_type;

			/**
			 * \brief A duration operation handler.
			 */
			typedef boost::function<void (const ep_type&, const boost::system::error_code&, const boost::posix_time::time_duration& duration)> duration_handler_type;

			// Public constants

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

			// Public methods

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to bind to.
			 * \param configuration The configuration to use.
			 * \param _logger The logger to use for logging.
			 */
			core(boost::asio::io_service& io_service, const freelan::configuration& configuration, const freelan::logger& _logger);

			/**
			 * \brief Open the core.
			 * \see close
			 */
			void open();

			/**
			 * \brief Close the core.
			 */
			void close();


		private:
			bool is_banned(const boost::asio::ip::address& address) const;

			boost::asio::io_service& m_io_service;
			freelan::configuration m_configuration;
			freelan::logger m_logger;
			boost::asio::ip::udp::resolver m_resolver;

		private: /* FSCP server */

			void open_server(const ep_type&);
			void close_server();

			void async_contact(const endpoint& target, duration_handler_type handler);
			void async_contact(const endpoint& target);
			void async_contact_all();
			void async_dynamic_contact_all();
			void async_send_contact_request_to_all(const fscp::hash_list_type&, multiple_endpoints_handler_type);
			void async_send_contact_request_to_all(const fscp::hash_list_type&);
			void async_introduce_to(const ep_type&, simple_handler_type);
			void async_introduce_to(const ep_type&);
			void async_request_session(const ep_type&, simple_handler_type);
			void async_request_session(const ep_type&);

			void do_contact(const ep_type&, duration_handler_type);

			void do_handle_contact(const endpoint&, const ep_type&, const boost::system::error_code&, const boost::posix_time::time_duration&);
			void do_handle_periodic_contact(const boost::system::error_code&);
			void do_handle_periodic_dynamic_contact(const boost::system::error_code&);
			void do_handle_send_contact_request(const ep_type&, const boost::system::error_code&);
			void do_handle_send_contact_request_to_all(const std::map<ep_type, boost::system::error_code>&);
			void do_handle_introduce_to(const ep_type&, const boost::system::error_code&);
			void do_handle_request_session(const ep_type&, const boost::system::error_code&);

			bool do_handle_hello_received(const ep_type&, bool);
			bool do_handle_contact_request_received(const ep_type&, cert_type, hash_type, const ep_type&);
			void do_handle_contact_received(const ep_type&, hash_type, const ep_type&);
			bool do_handle_presentation_received(const ep_type&, cert_type, cert_type, bool);
			bool do_handle_session_request_received(const ep_type&, const fscp::cipher_algorithm_list_type&, bool);
			bool do_handle_session_received(const ep_type&, fscp::cipher_algorithm_type, bool);
			void do_handle_session_failed(const ep_type&, bool, const fscp::algorithm_info_type&, const fscp::algorithm_info_type&);
			void do_handle_session_established(const ep_type&, bool, const fscp::algorithm_info_type&, const fscp::algorithm_info_type&);
			void do_handle_session_lost(const ep_type&);
			void do_handle_data_received(const ep_type&, fscp::channel_number_type, boost::asio::const_buffer);

			boost::scoped_ptr<fscp::server> m_server;
			boost::asio::deadline_timer m_contact_timer;
			boost::asio::deadline_timer m_dynamic_contact_timer;

		private: /* Certificate validation */

			static const int ex_data_index;
			static int certificate_validation_callback(int, X509_STORE_CTX*);

			bool certificate_validation_method(bool, cryptoplus::x509::store_context);
			bool certificate_is_valid(cert_type);

			cryptoplus::x509::store m_ca_store;

		private: /* TAP adapter */

			typedef asiotap::osi::proxy<asiotap::osi::arp_frame> arp_proxy_type;
			typedef asiotap::osi::proxy<asiotap::osi::dhcp_frame> dhcp_proxy_type;

			void open_tap_adapter();
			void close_tap_adapter();

			void do_handle_proxy_data(boost::asio::const_buffer);
			bool do_handle_arp_request(const boost::asio::ip::address_v4&, ethernet_address_type&);

			boost::scoped_ptr<asiotap::tap_adapter> m_tap_adapter;

			asiotap::osi::filter<asiotap::osi::ethernet_frame> m_ethernet_filter;
			asiotap::osi::complex_filter<asiotap::osi::arp_frame, asiotap::osi::ethernet_frame>::type m_arp_filter;
			asiotap::osi::complex_filter<asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_ipv4_filter;
			asiotap::osi::complex_filter<asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_udp_filter;
			asiotap::osi::complex_filter<asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_bootp_filter;
			asiotap::osi::complex_filter<asiotap::osi::dhcp_frame, asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type m_dhcp_filter;

			boost::scoped_ptr<arp_proxy_type> m_arp_proxy;
			boost::scoped_ptr<dhcp_proxy_type> m_dhcp_proxy;
			boost::array<unsigned char, 2048> m_proxy_buffer;
	};
}

#endif /* FREELAN_CORE_HPP */

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
#include "switch.hpp"
#include "router.hpp"
#include "message.hpp"

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
#include <boost/date_time/posix_time/posix_time.hpp>

 #include <queue>

namespace freelan
{
	/**
	 * \brief The core class.
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
			 * \brief The resolver type.
			 */
			typedef boost::asio::ip::udp::resolver resolver_type;

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
			 * \brief An IO operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&, size_t)> io_handler_type;

			/**
			 * \brief An operation handler for multiple endpoints.
			 */
			typedef boost::function<void (const std::map<ep_type, boost::system::error_code>&)> multiple_endpoints_handler_type;

			/**
			 * \brief A duration operation handler.
			 */
			typedef boost::function<void (const ep_type&, const boost::system::error_code&, const boost::posix_time::time_duration& duration)> duration_handler_type;

			// Callbacks

			/**
			 * \brief The log callback.
			 */
			typedef logger::log_handler_type log_handler_type;

			/**
			 * \brief The core opened callback.
			 */
			typedef boost::function<void ()> core_opened_handler_type;

			/**
			 * \brief The core closed callback.
			 */
			typedef boost::function<void ()> core_closed_handler_type;

			/**
			 * \brief A session failed callback.
			 * \param host The host with which a session is established.
			 * \param is_new A flag that indicates whether the session is a new session or a session renewal.
			 * \param local The local algorithms.
			 * \param remote The remote algorithms.
			 */
			typedef boost::function<void (const ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)> session_failed_handler_type;

			/**
			 * \brief A session established callback.
			 * \param host The host with which a session is established.
			 * \param is_new A flag that indicates whether the session is a new session or a session renewal.
			 * \param local The local algorithms.
			 * \param remote The remote algorithms.
			 */
			typedef boost::function<void (const ep_type& host, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)> session_established_handler_type;

			/**
			 * \brief A session lost callback.
			 * \param host The host with which a session was lost.
			 */
			typedef boost::function<void (const ep_type& host)> session_lost_handler_type;

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
			 */
			core(boost::asio::io_service& io_service, const freelan::configuration& configuration);

			/**
			 * \brief Set the function to call when a log entry is emitted.
			 * \param callback The callback.
			 *
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_log_callback(log_handler_type callback);

			/**
			 * \brief Set the logger's level.
			 * \param level The log level.
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_log_level(log_level level)
			{
				m_logger.set_level(level);
			}

			/**
			 * \brief Set the function to call when the core was just opened.
			 * \param callback The callback.
			 *
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_core_opened_callback(core_opened_handler_type callback);

			/**
			 * \brief Set the function to call when the core was just closed.
			 * \param callback The callback.
			 *
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_close_callback(core_closed_handler_type callback);

			/**
			 * \brief Set the session failed callback.
			 * \param callback The callback.
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_session_failed_callback(session_failed_handler_type callback);

			/**
			 * \brief Set the session established callback.
			 * \param callback The callback.
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_session_established_callback(session_established_handler_type callback);

			/**
			 * \brief Set the session lost callback.
			 * \param callback The callback.
			 * \warning This method can only be called when the core is NOT running.
			 */
			void set_session_lost_callback(session_lost_handler_type callback);

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

			boost::asio::io_service& m_io_service;
			const freelan::configuration m_configuration;
			boost::asio::strand m_logger_strand;
			freelan::logger m_logger;

		private: /* Callbacks */

			void do_handle_log(log_level, const std::string&, const boost::posix_time::ptime&);

			log_handler_type m_log_callback;
			core_opened_handler_type m_core_opened_callback;
			core_closed_handler_type m_core_closed_callback;
			session_failed_handler_type m_session_failed_callback;
			session_established_handler_type m_session_established_callback;
			session_lost_handler_type m_session_lost_callback;

		private: /* General purpose */

			bool is_banned(const boost::asio::ip::address& address) const;

		private: /* FSCP server */

			void open_server();
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
			void do_handle_data_received(const ep_type&, fscp::channel_number_type, fscp::server::shared_buffer_type, boost::asio::const_buffer);
			void do_handle_message(const ep_type&, fscp::server::shared_buffer_type, const message&);

			boost::shared_ptr<fscp::server> m_server;
			boost::asio::deadline_timer m_contact_timer;
			boost::asio::deadline_timer m_dynamic_contact_timer;

		private: /* Certificate validation */

			static const int ex_data_index;
			static int certificate_validation_callback(int, X509_STORE_CTX*);

			bool certificate_validation_method(bool, cryptoplus::x509::store_context);
			bool certificate_is_valid(cert_type);

			cryptoplus::x509::store m_ca_store;
			boost::mutex m_ca_store_mutex;

		private: /* TAP adapter */

			typedef asiotap::osi::filter<asiotap::osi::ethernet_frame> ethernet_filter_type;
			typedef asiotap::osi::complex_filter<asiotap::osi::arp_frame, asiotap::osi::ethernet_frame>::type arp_filter_type;
			typedef asiotap::osi::complex_filter<asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type ipv4_filter_type;
			typedef asiotap::osi::complex_filter<asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type udp_filter_type;
			typedef asiotap::osi::complex_filter<asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type bootp_filter_type;
			typedef asiotap::osi::complex_filter<asiotap::osi::dhcp_frame, asiotap::osi::bootp_frame, asiotap::osi::udp_frame, asiotap::osi::ipv4_frame, asiotap::osi::ethernet_frame>::type dhcp_filter_type;
			typedef asiotap::osi::const_helper<asiotap::osi::arp_frame> arp_helper_type;
			typedef asiotap::osi::const_helper<asiotap::osi::dhcp_frame> dhcp_helper_type;
			typedef asiotap::osi::proxy<asiotap::osi::arp_frame> arp_proxy_type;
			typedef asiotap::osi::proxy<asiotap::osi::dhcp_frame> dhcp_proxy_type;
			typedef fscp::memory_pool<65536, 8> tap_adapter_memory_pool;
			typedef fscp::memory_pool<2048, 2> proxy_memory_pool;

			void open_tap_adapter();
			void close_tap_adapter();

			void async_read_tap();

			template <typename WriteHandler>
			void async_write_tap(boost::asio::const_buffer data, WriteHandler handler)
			{
				void_handler_type write_handler = boost::bind(&asiotap::tap_adapter::async_write<WriteHandler>, m_tap_adapter, data, handler);

				m_tap_write_queue_strand.post(boost::bind(&core::push_tap_write, this, write_handler));
			}

			void async_write_tap(boost::asio::const_buffer data, simple_handler_type handler)
			{
				async_write_tap<io_handler_type>(data, boost::bind(handler, _1));
			}

			void push_tap_write(void_handler_type);
			void pop_tap_write();

			void do_read_tap();

			void do_handle_tap_adapter_read(tap_adapter_memory_pool::shared_buffer_type, const boost::system::error_code&, size_t);
			void do_handle_tap_adapter_write(const boost::system::error_code&);
			void do_handle_arp_frame(const arp_helper_type&);
			void do_handle_dhcp_frame(const dhcp_helper_type&);
			bool do_handle_arp_request(const boost::asio::ip::address_v4&, ethernet_address_type&);

			boost::shared_ptr<asiotap::tap_adapter> m_tap_adapter;
			boost::asio::strand m_tap_adapter_strand;
			boost::asio::strand m_proxies_strand;
			tap_adapter_memory_pool m_tap_adapter_memory_pool;
			std::queue<void_handler_type> m_tap_write_queue;
			boost::asio::strand m_tap_write_queue_strand;

			ethernet_filter_type m_ethernet_filter;
			arp_filter_type m_arp_filter;
			ipv4_filter_type m_ipv4_filter;
			udp_filter_type m_udp_filter;
			bootp_filter_type m_bootp_filter;
			dhcp_filter_type m_dhcp_filter;

			boost::scoped_ptr<arp_proxy_type> m_arp_proxy;
			boost::scoped_ptr<dhcp_proxy_type> m_dhcp_proxy;
			proxy_memory_pool m_proxy_memory_pool;

		private: /* Switch & router */

			typedef std::map<ep_type, switch_::port_type> endpoint_switch_port_map_type;
			typedef std::map<ep_type, router::port_type> endpoint_router_port_map_type;

			void async_register_switch_port(const ep_type& host)
			{
				m_switch_strand.post(boost::bind(&core::do_register_switch_port, this, host));
			}

			void async_unregister_switch_port(const ep_type& host)
			{
				m_switch_strand.post(boost::bind(&core::do_unregister_switch_port, this, host));
			}

			void async_register_router_port(const ep_type& host)
			{
				m_router_strand.post(boost::bind(&core::do_register_router_port, this, host));
			}

			void async_unregister_router_port(const ep_type& host)
			{
				m_router_strand.post(boost::bind(&core::do_unregister_router_port, this, host));
			}

			template <typename WriteHandler>
			void async_write_switch(const port_index_type& index, boost::asio::const_buffer data, WriteHandler handler)
			{
				m_switch_strand.post(boost::bind(&core::do_write_switch, this, index, data, handler));
			}

			template <typename WriteHandler>
			void async_write_router(const port_index_type& index, boost::asio::const_buffer data, WriteHandler handler)
			{
				m_router_strand.post(boost::bind(&core::do_write_router, this, index, data, handler));
			}

			void do_register_switch_port(const ep_type&);
			void do_register_router_port(const ep_type&);
			void do_unregister_switch_port(const ep_type&);
			void do_unregister_router_port(const ep_type&);
			void do_write_switch(const port_index_type&, boost::asio::const_buffer, switch_::multi_write_handler_type);
			void do_write_router(const port_index_type&, boost::asio::const_buffer, router::port_type::write_handler_type);

			boost::asio::strand m_switch_strand;
			boost::asio::strand m_router_strand;

			switch_ m_switch;
			router m_router;
	};
}

#endif /* FREELAN_CORE_HPP */

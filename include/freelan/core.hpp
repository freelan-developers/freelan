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
	/**
	 * \brief The core class.
	 */
	class core
	{
		public:

			/**
			 * \brief The ethernet address type.
			 */
			typedef freelan::configuration::ethernet_address_type ethernet_address_type;

			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef fscp::server::cert_type cert_type;

			/**
			 * \brief The identity store type.
			 */
			typedef fscp::identity_store identity_store;

			/**
			 * \brief The contact period.
			 */
			static const boost::posix_time::time_duration CONTACT_PERIOD;

			/**
			 * \brief The default service.
			 */
			static const std::string DEFAULT_SERVICE;

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
			 * \param os The output stream to use for logging.
			 */
			core(boost::asio::io_service& io_service, const freelan::configuration& configuration, logger::stream_type& os);

			/**
			 * \brief Get the configuration.
			 * \return The current configuration.
			 */
			const freelan::configuration& configuration() const;

			/**
			 * \brief Get the associated tap adapter.
			 * \return The associated tap adapter.
			 */
			const asiotap::tap_adapter& tap_adapter() const;

			/**
			 * \brief Get the associated server.
			 * \return The associated server.
			 */
			const fscp::server& server() const;

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

		private:

			// fscp::server related methods
			void async_greet(const ep_type&);
			bool on_hello_request(const ep_type&, bool);
			void on_hello_response(const ep_type&, const boost::posix_time::time_duration&, bool);
			bool on_presentation(const ep_type&, cert_type, cert_type, bool);
			bool on_session_request(const ep_type&, bool);
			void on_session_established(const ep_type&);
			void on_session_lost(const ep_type&);
			void on_data(const ep_type&, boost::asio::const_buffer);

			// asiotap::tap_adapter related methods
			void tap_adapter_read_done(asiotap::tap_adapter&, const boost::system::error_code&, size_t);

			// Other methods
			void do_greet(const boost::system::error_code&, boost::asio::ip::udp::resolver::iterator);
			void do_contact();
			void do_contact(const boost::system::error_code&);

			// Members
			freelan::configuration m_configuration;
			fscp::server m_server;
			boost::asio::ip::udp::resolver m_resolver;
			asiotap::tap_adapter m_tap_adapter;
			boost::array<unsigned char, 65536> m_tap_adapter_buffer;
			boost::asio::deadline_timer m_contact_timer;

			// User callbacks
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

			// Logger
			logger m_logger;
	};

	inline const freelan::configuration& core::configuration() const
	{
		return m_configuration;
	}

	inline const asiotap::tap_adapter& core::tap_adapter() const
	{
		return m_tap_adapter;
	}

	inline const fscp::server& core::server() const
	{
		return m_server;
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

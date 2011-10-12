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
 * \file core.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan core class.
 */

#include "core.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <asiotap/osi/ethernet_helper.hpp>

#include "os.hpp"

namespace freelan
{
	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);

	const std::string core::DEFAULT_SERVICE = "12000";

	core::core(boost::asio::io_service& io_service, const freelan::configuration& _configuration, const logger& log) :
		m_running(false),
		m_configuration(_configuration),
		m_server(io_service, *m_configuration.identity),
		m_resolver(io_service),
		m_tap_adapter(io_service),
		m_contact_timer(io_service, CONTACT_PERIOD),
		m_arp_filter(m_ethernet_filter),
		m_ipv4_filter(m_ethernet_filter),
		m_udp_filter(m_ipv4_filter),
		m_bootp_filter(m_udp_filter),
		m_dhcp_filter(m_bootp_filter),
		m_logger(log)
	{
		m_server.set_hello_message_callback(boost::bind(&core::on_hello_request, this, _1, _2));
		m_server.set_presentation_message_callback(boost::bind(&core::on_presentation, this, _1, _2, _3, _4));
		m_server.set_session_request_message_callback(boost::bind(&core::on_session_request, this, _1, _2));
		m_server.set_session_established_callback(boost::bind(&core::on_session_established, this, _1));
		m_server.set_session_lost_callback(boost::bind(&core::on_session_lost, this, _1));
		m_server.set_data_message_callback(boost::bind(&core::on_data, this, _1, _2));
	}

	void core::open()
	{
		m_logger(LOG_DEBUG) << "Core is opening." << endl;

		typedef boost::asio::ip::udp::resolver::query query;
		
		m_server.open(m_configuration.listen_on->resolve(m_resolver, m_configuration.hostname_resolution_protocol, query::address_configured | query::passive, DEFAULT_SERVICE));

		m_tap_adapter.open();

		// IPv4 address
		if (m_configuration.tap_adapter_ipv4_address_prefix_length)
		{
#ifdef WINDOWS
			// Quick fix for Windows:
			// Directly setting the IPv4 address/prefix length doesn't work like it should on Windows.
			// We disable direct setting if DHCP is enabled.

			if (!m_configuration.enable_dhcp_proxy)
			{
				m_tap_adapter.add_ip_address_v4(m_configuration.tap_adapter_ipv4_address_prefix_length->address, m_configuration.tap_adapter_ipv4_address_prefix_length->prefix_length);
			}
#else
			m_tap_adapter.add_ip_address_v4(m_configuration.tap_adapter_ipv4_address_prefix_length->address, m_configuration.tap_adapter_ipv4_address_prefix_length->prefix_length);
#endif
		}

		// IPv6 address
		if (m_configuration.tap_adapter_ipv6_address_prefix_length)
		{
			m_tap_adapter.add_ip_address_v6(m_configuration.tap_adapter_ipv6_address_prefix_length->address, m_configuration.tap_adapter_ipv6_address_prefix_length->prefix_length);
		}

		m_tap_adapter.set_connected_state(true);

		m_tap_adapter.async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(m_tap_adapter), _1, _2));

		do_contact();
		m_contact_timer.async_wait(boost::bind(&core::do_contact, this, boost::asio::placeholders::error));

		// The ARP proxy
		if (m_configuration.enable_arp_proxy)
		{
			m_arp_proxy.reset(new arp_proxy_type(boost::asio::buffer(m_proxy_buffer), boost::bind(&core::on_proxy_data, this, _1), m_arp_filter));
			m_arp_proxy->set_arp_request_callback(boost::bind(&core::on_arp_request, this, _1, _2));
		}
		else
		{
			m_arp_proxy.reset();
		}

		// The DHCP proxy
		if (m_configuration.enable_dhcp_proxy)
		{
			m_dhcp_proxy.reset(new dhcp_proxy_type(boost::asio::buffer(m_proxy_buffer), boost::bind(&core::on_proxy_data, this, _1), m_dhcp_filter));
			m_dhcp_proxy->set_hardware_address(m_tap_adapter.ethernet_address());

			if (m_configuration.dhcp_server_ipv4_address_prefix_length)
			{
				m_dhcp_proxy->set_software_address(m_configuration.dhcp_server_ipv4_address_prefix_length->address);
			}

			if (m_configuration.tap_adapter_ipv4_address_prefix_length)
			{
				m_dhcp_proxy->add_entry(m_tap_adapter.ethernet_address(), m_configuration.tap_adapter_ipv4_address_prefix_length->address, m_configuration.tap_adapter_ipv4_address_prefix_length->prefix_length);
			}
		}
		else
		{
			m_dhcp_proxy.reset();
		}

		m_running = true;
	}

	void core::close()
	{
		if (m_running)
		{
			m_running = false;

			m_logger(LOG_DEBUG) << "Core is closing." << endl;

			m_dhcp_proxy.reset();
			m_arp_proxy.reset();

			m_contact_timer.cancel();

			m_tap_adapter.cancel();
			m_tap_adapter.set_connected_state(false);

			// IPv6 address
			if (m_configuration.tap_adapter_ipv6_address_prefix_length)
			{
				m_tap_adapter.remove_ip_address_v6(m_configuration.tap_adapter_ipv6_address_prefix_length->address, m_configuration.tap_adapter_ipv6_address_prefix_length->prefix_length);
			}

			// IPv4 address
			if (m_configuration.tap_adapter_ipv4_address_prefix_length)
			{
				m_tap_adapter.remove_ip_address_v4(m_configuration.tap_adapter_ipv4_address_prefix_length->address, m_configuration.tap_adapter_ipv4_address_prefix_length->prefix_length);
			}

			m_tap_adapter.close();

			m_server.close();
		}
	}

	void core::async_greet(const ep_type& target)
	{
		m_server.async_greet(target, boost::bind(&core::on_hello_response, this, _1, _2, _3), m_configuration.hello_timeout);
	}

	bool core::on_hello_request(const ep_type& sender, bool default_accept)
	{
		m_logger(LOG_DEBUG) << "Received HELLO_REQUEST from " << sender << "." << endl;

		if (default_accept)
		{
			m_server.async_introduce_to(sender);

			return true;
		}

		return false;
	}

	void core::on_hello_response(const ep_type& sender, const boost::posix_time::time_duration& time_duration, bool success)
	{
		if (success)
		{
			m_logger(LOG_DEBUG) << "Received HELLO_RESPONSE from " << sender << ". Latency: " << time_duration << "." << endl;

			m_server.async_introduce_to(sender);
		} else
		{
			m_logger(LOG_DEBUG) << "Received no HELLO_RESPONSE from " << sender << ". Timeout: " << time_duration << "." << endl;
		}
	}

	bool core::on_presentation(const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool is_new)
	{
		m_logger(LOG_DEBUG) << "Received PRESENTATION from " << sender << ". Signature: " << sig_cert.subject().oneline() << ". Cipherment: " << enc_cert.subject().oneline() << ". New presentation: " << is_new << "." << endl;

		// TODO: Here we should check for the certificates validity.
		// For now, let's assume they are valid.
		if (true)
		{
			m_server.async_request_session(sender);
			return true;
		}

		return false;
	}

	bool core::on_session_request(const ep_type& sender, bool default_accept)
	{
		m_logger(LOG_DEBUG) << "Received SESSION_REQUEST from " << sender << "." << endl;

		if (default_accept)
		{
			return true;
		}

		return false;
	}

	void core::on_session_established(const ep_type& sender)
	{
		m_logger(LOG_INFORMATION) << "Session established with " << sender << "." << endl;

		if (m_session_established_callback)
		{
			m_session_established_callback(sender);
		}
	}

	void core::on_session_lost(const ep_type& sender)
	{
		m_logger(LOG_INFORMATION) << "Session with " << sender << " lost." << endl;

		if (m_session_lost_callback)
		{
			m_session_lost_callback(sender);
		}
	}

	void core::on_data(const ep_type& sender, boost::asio::const_buffer data)
	{
		(void)sender;

		if (m_configuration.routing_method == configuration::RM_SWITCH)
		{
			try
			{
				// We read the source ethernet address and update the switch routing table according to it.
				asiotap::osi::const_helper<asiotap::osi::ethernet_frame> ethernet_helper(data);

				m_switch.update_entry(ethernet_helper.sender(), sender);
			}
			catch (std::length_error&)
			{
			}
		}

		m_tap_adapter.write(data);
	}

	void core::tap_adapter_read_done(asiotap::tap_adapter& _tap_adapter, const boost::system::error_code& ec, size_t cnt)
	{
		if (!ec)
		{
			boost::asio::const_buffer data = boost::asio::buffer(m_tap_adapter_buffer, cnt);

			bool handled = false;

			if (m_arp_proxy || m_dhcp_proxy)
			{
				m_ethernet_filter.parse(data);

				if (m_arp_proxy && m_arp_filter.get_last_helper())
				{
					handled = true;
					m_arp_filter.clear_last_helper();
				}

				if (m_dhcp_proxy && m_dhcp_filter.get_last_helper())
				{
					handled = true;
					m_dhcp_filter.clear_last_helper();
				}
			}

			if (!handled)
			{
				if (m_configuration.routing_method == configuration::RM_SWITCH)
				{
					ep_type endpoint;

					try
					{
						// We read the destination ethernet address and send to the targetted host only.
						asiotap::osi::const_helper<asiotap::osi::ethernet_frame> ethernet_helper(data);

						if (m_switch.get_entry(ethernet_helper.target(), endpoint))
						{
							m_server.async_send_data(endpoint, data);

							handled = true;
						}
					}
					catch (std::length_error&)
					{
					}
				}
				
				if (!handled)
				{
					m_server.async_send_data_to_all(data);
				}
			}

			// Start another read
			_tap_adapter.async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(_tap_adapter), _1, _2));
		}
		else
		{
			// If the core is currently stopping, this kind of error is expected.
			if (m_running)
			{
				m_logger(LOG_ERROR) << "Read failed on " << _tap_adapter.name() << ". Error: " << ec << endl;

				close();
			}
		}
	}

	void core::do_greet(const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator it, const freelan::configuration::ep_type& ep)
	{
		if (!ec)
		{
			if (!m_server.has_session(*it))
			{
				m_logger(LOG_DEBUG) << "Sending HELLO_REQUEST to " << ep_type(*it) << "..." << endl;

				async_greet(*it);
			}
		} else
		{
			m_logger(LOG_WARNING) << "Failed to resolve " << *ep << "." << endl;
		}
	}

	void core::do_contact()
	{
		BOOST_FOREACH(const freelan::configuration::ep_type& ep, m_configuration.contact_list)
		{
			typedef boost::asio::ip::udp::resolver::query query;

			ep->async_resolve(m_resolver, m_configuration.hostname_resolution_protocol, query::address_configured, DEFAULT_SERVICE, boost::bind(&core::do_greet, this, _1, _2, ep));
		}
	}

	void core::do_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			do_contact();

			m_contact_timer.expires_from_now(CONTACT_PERIOD);
			m_contact_timer.async_wait(boost::bind(&core::do_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::on_proxy_data(boost::asio::const_buffer data)
	{
		m_tap_adapter.write(data);
	}

	bool core::on_arp_request(const boost::asio::ip::address_v4& logical_address, ethernet_address_type& ethernet_address)
	{
		if (m_configuration.tap_adapter_ipv4_address_prefix_length)
		{
			if (logical_address != m_configuration.tap_adapter_ipv4_address_prefix_length->address)
			{
				ethernet_address = m_configuration.arp_proxy_fake_ethernet_address;

				return true;
			}
		}

		return false;
	}
}

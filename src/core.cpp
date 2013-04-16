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

#include <sstream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "os.hpp"
#include "client.hpp"
#include "tap_adapter_switch_port.hpp"
#include "endpoint_switch_port.hpp"
#include "logger_stream.hpp"

namespace freelan
{
	namespace
	{
		static const switch_::group_type TAP_ADAPTERS_GROUP = 0;
		static const switch_::group_type ENDPOINTS_GROUP = 1;
		static const boost::posix_time::time_duration CERTIFICATE_RENEWAL_DELAY = boost::posix_time::hours(6);

		enum ConfigurationItems
		{
			CI_GET_AUTHORITY_CERTIFICATE = 0x01,
			CI_JOIN_NETWORK = 0x02,
			CI_SIGN = 0x04,
			CI_ALL = CI_GET_AUTHORITY_CERTIFICATE | CI_JOIN_NETWORK | CI_SIGN
		};

		cryptoplus::x509::certificate_request generate_certificate_request(const freelan::configuration& configuration, const cryptoplus::pkey::rsa_key& private_key)
		{
			using namespace cryptoplus;

			x509::certificate_request csr = x509::certificate_request::create();

			csr.set_version(2);

			csr.set_public_key(pkey::pkey::from_rsa_key(private_key));

			csr.subject().push_back("CN", MBSTRING_ASC, configuration.server.username.c_str(), configuration.server.username.size());

			csr.sign(pkey::pkey::from_rsa_key(private_key), hash::message_digest_algorithm(NID_sha1));

			return csr;
		}
	}

	// Has to be put first, as static variables definition order matters
	const int core::ex_data_index = cryptoplus::x509::store_context::register_index();

	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);
	const boost::posix_time::time_duration core::DYNAMIC_CONTACT_PERIOD = boost::posix_time::seconds(45);

	const std::string core::DEFAULT_SERVICE = "12000";

	core::core(boost::asio::io_service& io_service, const freelan::configuration& _configuration, const freelan::logger& _logger) :
		m_io_service(io_service),
		m_running(false),
		m_configuration(_configuration),
		m_logger(_logger),
		m_server(),
		m_resolver(m_io_service),
		m_contact_timer(m_io_service, CONTACT_PERIOD),
		m_dynamic_contact_timer(m_io_service, DYNAMIC_CONTACT_PERIOD),
		m_configuration_update_callback(),
		m_open_callback(),
		m_close_callback(),
		m_session_failed_callback(),
		m_session_established_callback(),
		m_session_lost_callback(),
		m_arp_filter(m_ethernet_filter),
		m_ipv4_filter(m_ethernet_filter),
		m_udp_filter(m_ipv4_filter),
		m_bootp_filter(m_udp_filter),
		m_dhcp_filter(m_bootp_filter),
		m_switch(m_configuration.switch_),
		m_check_configuration_timer(m_io_service)
	{
	}

	void core::open()
	{
		typedef boost::asio::ip::udp::resolver::query query;

		m_listen_endpoint = boost::apply_visitor(endpoint_resolve_visitor(m_resolver, to_protocol(m_configuration.fscp.hostname_resolution_protocol), query::address_configured | query::passive, DEFAULT_SERVICE), m_configuration.fscp.listen_on);

		m_logger(LL_DEBUG) << "Core opening on " << *m_listen_endpoint << "...";

		if (m_configuration.server.enabled)
		{
			m_logger(LL_INFORMATION) << "Server mode enabled.";

			update_server_configuration(CI_ALL);
		}

		if (m_configuration_update_callback)
		{
			m_configuration_update_callback(m_configuration);
		}

		if (!m_configuration.security.identity)
		{
			throw std::runtime_error("No user certificate or private key set. Unable to continue.");
		}

		create_server();
		create_tap_adapter();

		// FSCP
		m_server->open(*m_listen_endpoint);

		if (m_configuration.security.certificate_validation_method == security_configuration::CVM_DEFAULT)
		{
			m_ca_store = cryptoplus::x509::store::create();

			BOOST_FOREACH(const cert_type& cert, m_configuration.security.certificate_authority_list)
			{
				m_ca_store.add_certificate(cert);
			}

			BOOST_FOREACH(const crl_type& crl, m_configuration.security.certificate_revocation_list_list)
			{
				m_ca_store.add_certificate_revocation_list(crl);
			}

			switch (m_configuration.security.certificate_revocation_validation_method)
			{
				case security_configuration::CRVM_LAST:
					{
						m_ca_store.set_verification_flags(X509_V_FLAG_CRL_CHECK);
						break;
					}
				case security_configuration::CRVM_ALL:
					{
						m_ca_store.set_verification_flags(X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
						break;
					}
				case security_configuration::CRVM_NONE:
					{
						break;
					}
			}
		}

		// We start the contact loop
		do_contact();
		m_contact_timer.async_wait(boost::bind(&core::do_periodic_contact, this, boost::asio::placeholders::error));
		m_dynamic_contact_timer.async_wait(boost::bind(&core::do_periodic_dynamic_contact, this, boost::asio::placeholders::error));

		// Tap adapter
		if (m_tap_adapter)
		{
			m_logger(LL_DEBUG) << "Opening tap adapter \"" << m_configuration.tap_adapter.name << "\" with a desired MTU set to: " << m_configuration.tap_adapter.mtu;

			m_tap_adapter->open(m_configuration.tap_adapter.name, compute_mtu(m_configuration.tap_adapter.mtu, get_auto_mtu_value()));

			m_logger(LL_INFORMATION) << "Tap adapter \"" << m_tap_adapter->name() << "\" opened with a MTU set to: " << m_tap_adapter->mtu();

			// IPv4 address
			if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
			{
				try
				{
#ifdef WINDOWS
					// Quick fix for Windows:
					// Directly setting the IPv4 address/prefix length doesn't work like it should on Windows.
					// We disable direct setting if DHCP is enabled.

					if (!m_configuration.tap_adapter.dhcp_proxy_enabled)
					{
						m_tap_adapter->add_ip_address_v4(
						    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
						    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
						);
					}
#else
					m_tap_adapter->add_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
					);
#endif
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot set IPv4 address: " << ex.what();
				}
			}

			// IPv6 address
			if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->add_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv6_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot set IPv6 address: " << ex.what();
				}
			}

			m_tap_adapter->set_connected_state(true);

			m_tap_adapter->async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(*m_tap_adapter), _1, _2));

			// The ARP proxy
			if (m_configuration.tap_adapter.arp_proxy_enabled)
			{
				m_arp_proxy.reset(new arp_proxy_type(boost::asio::buffer(m_proxy_buffer), boost::bind(&core::on_proxy_data, this, _1), m_arp_filter));
				m_arp_proxy->set_arp_request_callback(boost::bind(&core::on_arp_request, this, _1, _2));
			}
			else
			{
				m_arp_proxy.reset();
			}

			// The DHCP proxy
			if (m_configuration.tap_adapter.dhcp_proxy_enabled)
			{
				m_dhcp_proxy.reset(new dhcp_proxy_type(boost::asio::buffer(m_proxy_buffer), boost::bind(&core::on_proxy_data, this, _1), m_dhcp_filter));
				m_dhcp_proxy->set_hardware_address(m_tap_adapter->ethernet_address());

				if (!m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.is_null())
				{
					m_dhcp_proxy->set_software_address(m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.address());
				}

				if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
				{
					m_dhcp_proxy->add_entry(
					    m_tap_adapter->ethernet_address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
					);
				}
			}
			else
			{
				m_dhcp_proxy.reset();
			}

			if (m_configuration.tap_adapter.up_callback)
			{
				m_configuration.tap_adapter.up_callback(*this, *m_tap_adapter);
			}
		}

		m_logger(LL_DEBUG) << "Core opened.";

		if (m_open_callback)
		{
			m_io_service.post(m_open_callback);
		}

		m_running = true;
	}

	void core::close()
	{
		if (m_running)
		{
			m_running = false;

			if (m_close_callback)
			{
				m_io_service.post(m_close_callback);
			}

			m_io_service.post(boost::bind(&core::do_close, this));
		}
	}

	void core::log(freelan::log_level level, const std::string& msg)
	{
		m_io_service.post(boost::bind(&logger::log, boost::ref(m_logger), level, msg));
	}

	void core::do_close()
	{
		m_logger(LL_DEBUG) << "Core closing...";

		m_dhcp_proxy.reset();
		m_arp_proxy.reset();

		if (m_tap_adapter)
		{
			if (m_configuration.tap_adapter.down_callback)
			{
				m_configuration.tap_adapter.down_callback(*this, *m_tap_adapter);
			}

			m_tap_adapter->cancel();
			m_tap_adapter->set_connected_state(false);

			// IPv6 address
			if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->remove_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv6_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv6 address: " << ex.what();
				}
			}

			// IPv4 address
			if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
			{
				try
				{
					m_tap_adapter->remove_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv4 address: " << ex.what();
				}
			}

			m_tap_adapter->close();
		}

		m_check_configuration_timer.cancel();
		m_contact_timer.cancel();
		m_dynamic_contact_timer.cancel();

		m_server->close();
		m_listen_endpoint = boost::none;

		m_logger(LL_DEBUG) << "Core closed.";
	}

	void core::async_greet(const ep_type& target)
	{
		m_server->async_greet(target, boost::bind(&core::on_hello_response, this, _1, _2, _3), m_configuration.fscp.hello_timeout);
	}

	bool core::on_hello_request(const ep_type& sender, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received HELLO_REQUEST from " << sender << ".";

		if (default_accept)
		{
			m_server->async_introduce_to(sender);

			return true;
		}

		return false;
	}

	void core::on_hello_response(const ep_type& sender, const boost::posix_time::time_duration& time_duration, bool success)
	{
		if (success)
		{
			m_logger(LL_DEBUG) << "Received HELLO_RESPONSE from " << sender << ". Latency: " << time_duration << ".";

			m_server->async_introduce_to(sender);
		}
		else
		{
			m_logger(LL_DEBUG) << "Received no HELLO_RESPONSE from " << sender << ". Timeout: " << time_duration << ".";
		}
	}

	bool core::on_presentation(const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool is_new)
	{
		if (m_logger.level() <= LL_DEBUG)
		{
			m_logger(LL_DEBUG) << "Received PRESENTATION from " << sender << ". Signature: " << sig_cert.subject().oneline() << ". Cipherment: " << enc_cert.subject().oneline() << ". New presentation: " << is_new << ".";
		}

		if (certificate_is_valid(sig_cert) && certificate_is_valid(enc_cert))
		{
			m_server->async_request_session(sender);
			return true;
		}

		return false;
	}

	bool core::on_session_request(const ep_type& sender, const fscp::cipher_algorithm_list_type& calg_cap, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received SESSION_REQUEST from " << sender << ".";

		if (m_logger.level() <= LL_DEBUG)
		{
			std::ostringstream oss;

			BOOST_FOREACH(const fscp::cipher_algorithm_type& calg, calg_cap)
			{
				oss << " " << calg;
			}

			m_logger(LL_DEBUG) << "Cipher algorithm capabilities:" << oss.str();
		}

		if (default_accept)
		{
			return true;
		}

		return false;
	}

	void core::on_session_failed(const ep_type& sender, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
	{
		cert_type sig_cert = m_server->get_presentation(sender).signature_certificate();

		if (is_new)
		{
			m_logger(LL_WARNING) << "Session establishment with " << sender << " (" << sig_cert.subject().oneline() << ") failed.";
		}
		else
		{
			m_logger(LL_WARNING) << "Session renewal with " << sender << " (" << sig_cert.subject().oneline() << ") failed.";
		}

		m_logger(LL_WARNING) << "Local algorithms: " << local;
		m_logger(LL_WARNING) << "Remote algorithms: " << remote;

		if (m_session_failed_callback)
		{
			m_session_failed_callback(sender, is_new, local, remote);
		}
	}

	void core::on_session_established(const ep_type& sender, bool is_new, const fscp::algorithm_info_type& local, const fscp::algorithm_info_type& remote)
	{
		cert_type sig_cert = m_server->get_presentation(sender).signature_certificate();

		if (is_new)
		{
			m_logger(LL_INFORMATION) << "Session established with " << sender << " (" << sig_cert.subject().oneline() << ").";
		}
		else
		{
			m_logger(LL_INFORMATION) << "Session renewed with " << sender << " (" << sig_cert.subject().oneline() << ").";
		}

		m_logger(LL_INFORMATION) << "Local algorithms: " << local;
		m_logger(LL_INFORMATION) << "Remote algorithms: " << remote;

		if (is_new)
		{
			const switch_::port_type port = boost::make_shared<endpoint_switch_port>(sender, boost::bind(&fscp::server::async_send_data, &*m_server, _1, fscp::CHANNEL_NUMBER_0, _2));

			m_endpoint_switch_port_map[sender] = port;
			m_switch.register_port(port, ENDPOINTS_GROUP);
		}

		if (m_session_established_callback)
		{
			m_session_established_callback(sender, is_new, local, remote);
		}
	}

	void core::on_session_lost(const ep_type& sender)
	{
		cert_type sig_cert = m_server->get_presentation(sender).signature_certificate();

		m_logger(LL_INFORMATION) << "Session with " << sender << " lost (" << sig_cert.subject().oneline() << ").";

		if (m_session_lost_callback)
		{
			m_session_lost_callback(sender);
		}

		const switch_::port_type port = m_endpoint_switch_port_map[sender];

		if (port)
		{
			m_switch.unregister_port(port);
			m_endpoint_switch_port_map.erase(sender);
		}
	}

	void core::on_data(const ep_type& sender, fscp::channel_number_type channel_number, boost::asio::const_buffer data)
	{
		switch (channel_number)
		{
			case fscp::CHANNEL_NUMBER_0:
				on_ethernet_data(sender, data);
				break;
			default:
				m_logger(LL_WARNING) << "Received unhandled " << boost::asio::buffer_size(data) << " byte(s) of data on FSCP channel #" << static_cast<int>(channel_number);
				break;
		}
	}

	bool core::on_contact_request(const ep_type& sender, cert_type cert, const ep_type& target)
	{
		if (m_configuration.fscp.accept_contact_requests)
		{
			m_logger(LL_INFORMATION) << "Received contact request from " << sender << " for " << cert.subject().oneline() << " (" << target << ")";

			return true;
		}
		else
		{
			return false;
		}
	}

	void core::on_contact(const ep_type& sender, cert_type cert, const ep_type& target)
	{
		if (m_configuration.fscp.accept_contacts)
		{
			// We check if the contact is one of our forbidden network list.
			if (has_address(m_configuration.fscp.never_contact_list.begin(), m_configuration.fscp.never_contact_list.end(), target.address()))
			{
				m_logger(LL_WARNING) << "Received forbidden contact from " << sender << ": " << cert.subject().oneline() << " is at " << target << " but won't be contacted.";
			}
			else
			{
				m_logger(LL_INFORMATION) << "Received contact from " << sender << ": " << cert.subject().oneline() << " is at " << target;

				do_greet(target);
			}
		}
	}

	void core::on_ethernet_data(const ep_type& sender, boost::asio::const_buffer data)
	{
		const switch_::port_type port = m_endpoint_switch_port_map[sender];

		if (port)
		{
			m_switch.receive_data(port, data);
		}
	}

	void core::on_network_error(const ep_type& target, const boost::system::error_code& ec)
	{
		m_logger(LL_WARNING) << "Error while sending message to" << target << ": " << ec;
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
				m_switch.receive_data(m_tap_adapter_switch_port, data);
			}

			// Start another read
			_tap_adapter.async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(_tap_adapter), _1, _2));
		}
		else
		{
			// If the core is currently stopping, this kind of error is expected.
			if (m_running)
			{
				m_logger(LL_ERROR) << "Read failed on " << _tap_adapter.name() << ". Error: " << ec;

				close();
			}
		}
	}

	void core::do_greet(const ep_type& ep)
	{
		if (!m_server->has_session(ep))
		{
			m_logger(LL_DEBUG) << "Sending HELLO_REQUEST to " << ep << "...";

			async_greet(ep);
		}
	}

	void core::do_greet(const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator it, const freelan::fscp_configuration::endpoint& ep)
	{
		if (!ec)
		{
			do_greet(*it);
		}
		else
		{
			m_logger(LL_WARNING) << "Failed to resolve " << ep << ": " << ec;
		}
	}

	void core::do_contact()
	{
		std::for_each(m_configuration.fscp.contact_list.begin(), m_configuration.fscp.contact_list.end(), boost::bind(&core::do_contact, this, _1));
	}

	void core::do_contact(const fscp_configuration::endpoint& ep)
	{
		typedef boost::asio::ip::udp::resolver::query query;

		boost::apply_visitor(
		    endpoint_async_resolve_visitor(
		        m_resolver,
		        to_protocol(m_configuration.fscp.hostname_resolution_protocol),
		        query::address_configured,
		        DEFAULT_SERVICE,
		        boost::bind(&core::do_greet, this, _1, _2, ep)
		    ),
		    ep
		);
	}

	void core::do_periodic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			do_contact();

			m_contact_timer.expires_from_now(CONTACT_PERIOD);
			m_contact_timer.async_wait(boost::bind(&core::do_periodic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_dynamic_contact()
	{
		std::for_each(m_configuration.fscp.dynamic_contact_list.begin(), m_configuration.fscp.dynamic_contact_list.end(), boost::bind(&core::do_dynamic_contact, this, _1));
	}

	void core::do_dynamic_contact(cert_type cert)
	{
		m_server->async_send_contact_request_to_all(cert);
	}

	void core::do_periodic_dynamic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			do_dynamic_contact();

			m_dynamic_contact_timer.expires_from_now(DYNAMIC_CONTACT_PERIOD);
			m_dynamic_contact_timer.async_wait(boost::bind(&core::do_periodic_dynamic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_check_configuration(const boost::system::error_code& ec)
	{
		using namespace cryptoplus::pkey;
		using namespace cryptoplus::x509;

		if (ec != boost::asio::error::operation_aborted)
		{
			m_logger(LL_DEBUG) << "Checking certificate expiration...";

			const certificate sig_cert = m_configuration.security.identity->signature_certificate();
			const boost::posix_time::ptime not_after = sig_cert.not_after().to_ptime();
			const boost::posix_time::time_duration time_left = not_after - boost::posix_time::second_clock::universal_time();

			if (time_left <= CERTIFICATE_RENEWAL_DELAY)
			{
				m_logger(LL_INFORMATION) << "Certificate expires in " << time_left << ". Renewing...";

				async_update_server_configuration(CI_SIGN);
			}
			else
			{
				m_logger(LL_DEBUG) << "Certificate doesn't expire yet. Checking again at " << boost::posix_time::to_simple_string(not_after - CERTIFICATE_RENEWAL_DELAY) << ".";

				m_check_configuration_timer.expires_at(not_after - CERTIFICATE_RENEWAL_DELAY);
				m_check_configuration_timer.async_wait(boost::bind(&core::do_check_configuration, this, boost::asio::placeholders::error));
			}
		}
	}

	void core::create_server()
	{
		assert(m_configuration.security.identity);

		m_server.reset(new fscp::server(m_io_service, *m_configuration.security.identity));

		m_server->set_cipher_capabilities(m_configuration.fscp.cipher_capabilities);

		m_server->set_hello_message_callback(boost::bind(&core::on_hello_request, this, _1, _2));
		m_server->set_presentation_message_callback(boost::bind(&core::on_presentation, this, _1, _2, _3, _4));
		m_server->set_session_request_message_callback(boost::bind(&core::on_session_request, this, _1, _2, _3));
		m_server->set_session_failed_callback(boost::bind(&core::on_session_failed, this, _1, _2, _3, _4));
		m_server->set_session_established_callback(boost::bind(&core::on_session_established, this, _1, _2, _3, _4));
		m_server->set_session_lost_callback(boost::bind(&core::on_session_lost, this, _1));
		m_server->set_data_message_callback(boost::bind(&core::on_data, this, _1, _2, _3));
		m_server->set_contact_request_message_callback(boost::bind(&core::on_contact_request, this, _1, _2, _3));
		m_server->set_contact_message_callback(boost::bind(&core::on_contact, this, _1, _2, _3));
		m_server->set_network_error_callback(boost::bind(&core::on_network_error, this, _1, _2));
	}

	void core::create_tap_adapter()
	{
		if (m_configuration.tap_adapter.enabled)
		{
			m_tap_adapter.reset(new asiotap::tap_adapter(m_io_service));

			m_tap_adapter_switch_port = boost::make_shared<tap_adapter_switch_port>(boost::ref(*m_tap_adapter));
			m_switch.register_port(m_tap_adapter_switch_port, TAP_ADAPTERS_GROUP);
		}
	}

	unsigned int core::get_auto_mtu_value() const
	{
		assert(m_server);

		const unsigned int default_mtu_value = 1500;
		const size_t static_payload_size = 20 + 8 + 4 + 22; // IP + UDP + FSCP HEADER + FSCP DATA HEADER

		return default_mtu_value - static_payload_size;
	}

	void core::on_proxy_data(boost::asio::const_buffer data)
	{
		if (m_tap_adapter)
		{
			m_tap_adapter->write(data);
		}
	}

	bool core::on_arp_request(const boost::asio::ip::address_v4& logical_address, ethernet_address_type& ethernet_address)
	{
		if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
		{
			if (logical_address != m_configuration.tap_adapter.ipv4_address_prefix_length.address())
			{
				ethernet_address = m_configuration.tap_adapter.arp_proxy_fake_ethernet_address;

				return true;
			}
		}

		return false;
	}

	int core::certificate_validation_callback(int ok, X509_STORE_CTX* ctx)
	{
		cryptoplus::x509::store_context store_context(ctx);

		core* _this = static_cast<core*>(store_context.get_external_data(core::ex_data_index));

		return (_this->certificate_validation_method(ok != 0, store_context)) ? 1 : 0;
	}

	bool core::certificate_validation_method(bool ok, cryptoplus::x509::store_context store_context)
	{
		(void)store_context;

		cert_type cert = store_context.get_current_certificate();

		if (m_logger.level() <= LL_DEBUG)
		{
			m_logger(LL_DEBUG) << "Validating " << cert.subject().oneline() << ": " << (ok ? "OK" : "Error");
		}

		if (!ok)
		{
			m_logger(LL_WARNING) << "Error when validating " << cert.subject().oneline() << ": " << store_context.get_error_string() << " (depth: " << store_context.get_error_depth() << ")";
		}

		return ok;
	}

	bool core::certificate_is_valid(cert_type cert)
	{
		switch (m_configuration.security.certificate_validation_method)
		{
			case security_configuration::CVM_DEFAULT:
				{
					using namespace cryptoplus;

					// Create a store context to proceed to verification
					x509::store_context store_context = x509::store_context::create();

					store_context.initialize(m_ca_store, cert, NULL);

					// Ensure to set the verification callback *AFTER* you called initialize or it will be ignored.
					store_context.set_verification_callback(&core::certificate_validation_callback);

					// Add a reference to the current instance into the store context.
					store_context.set_external_data(core::ex_data_index, this);

					if (!store_context.verify())
					{
						return false;
					}

					break;
				}
			case security_configuration::CVM_NONE:
				{
					break;
				}
		}

		if (m_configuration.security.certificate_validation_callback)
		{
			return m_configuration.security.certificate_validation_callback(*this, cert);
		}

		return true;
	}

	void core::async_update_server_configuration(int items)
	{
		boost::thread thread(boost::bind(&core::update_server_configuration, this, items, true));

		thread.detach();
	}

	void core::update_server_configuration(int items, bool delayed)
	{
		// Warning !
		// This function may be called in another thread than the io_service thread.
		//
		// That is, modifiying values should *NOT* be done here.

		using namespace cryptoplus::pkey;
		using namespace cryptoplus::x509;

		freelan::logger delayed_logger(boost::bind(&core::log, this, _1, _2), m_logger.level());

		client _client(m_configuration, delayed ? delayed_logger : m_logger);

		_client.authenticate();

		if (CI_GET_AUTHORITY_CERTIFICATE & items)
		{
			certificate ca_cert = _client.get_authority_certificate();

			if (delayed)
			{
				m_io_service.post(boost::bind(&core::set_ca_certificate, this, ca_cert));
			}
			else
			{
				set_ca_certificate(ca_cert);
			}
		}

		if (CI_JOIN_NETWORK & items)
		{
			server_configuration::endpoint_list public_endpoint_list(m_configuration.server.public_endpoint_list.size());

			const uint16_t default_port = m_server ? m_server->socket().local_endpoint().port() : m_listen_endpoint->port();

			std::transform(
					m_configuration.server.public_endpoint_list.begin(),
					m_configuration.server.public_endpoint_list.end(),
					public_endpoint_list.begin(),
					boost::bind(get_default_port_endpoint, _1, default_port)
					);

			const network_info ninfo = _client.join_network(m_configuration.server.network, public_endpoint_list);

			if (delayed)
			{
				m_io_service.post(boost::bind(&core::set_network_information, this, ninfo));
			}
			else
			{
				set_network_information(ninfo);
			}
		}

		if (CI_SIGN & items)
		{
			pkey rsa_key = pkey::from_rsa_key(cryptoplus::pkey::rsa_key::generate_private_key(2048, 17, NULL, NULL, false));

			certificate_request csr = generate_certificate_request(m_configuration, rsa_key.get_rsa_key());

			certificate cert = _client.renew_certificate(csr);

			if (delayed)
			{
				m_io_service.post(boost::bind(&core::set_identity, this, fscp::identity_store(cert, rsa_key)));
			}
			else
			{
				set_identity(fscp::identity_store(cert, rsa_key));
			}
		}
	}

	void core::set_ca_certificate(cert_type ca_cert)
	{
		m_logger(LL_INFORMATION) << "Adding authority certificate to the trusted certificate list.";

		m_configuration.security.certificate_authority_list.push_back(ca_cert);

		if (m_ca_store)
		{
			m_ca_store.add_certificate(ca_cert);
		}
	}

	void core::set_network_information(const network_info& ninfo)
	{
		if (!ninfo.ipv4_address_prefix_length.is_null())
		{
			m_configuration.tap_adapter.ipv4_address_prefix_length = ninfo.ipv4_address_prefix_length;
			m_logger(LL_INFORMATION) << "IPv4 address set to " << m_configuration.tap_adapter.ipv4_address_prefix_length;
		}
		else
		{
			m_logger(LL_INFORMATION) << "No IPv4 address set.";
		}

		if (!ninfo.ipv6_address_prefix_length.is_null())
		{
			m_configuration.tap_adapter.ipv6_address_prefix_length = ninfo.ipv6_address_prefix_length;
			m_logger(LL_INFORMATION) << "IPv6 address set to " << m_configuration.tap_adapter.ipv6_address_prefix_length;
		}
		else
		{
			m_logger(LL_INFORMATION) << "No IPv6 address set.";
		}

		// This eases writting
		cert_list_type& dcl = m_configuration.fscp.dynamic_contact_list;

		BOOST_FOREACH(cert_type& user_cert, m_last_dynamic_contact_list_from_server)
		{
			const cert_list_type::iterator it = std::remove(dcl.begin(), dcl.end(), user_cert);

			if (it != dcl.end())
			{
				m_logger(LL_INFORMATION) << "Removing " << user_cert.subject().oneline() << " from the dynamic contact list.";

				dcl.erase(it, dcl.end());
			}
		}

		m_last_dynamic_contact_list_from_server = ninfo.users_certificates;

		BOOST_FOREACH(cert_type& user_cert, m_last_dynamic_contact_list_from_server)
		{
			m_logger(LL_INFORMATION) << "Adding " << user_cert.subject().oneline() << " to the dynamic contact list.";

			dcl.push_back(user_cert);
		}

		// This eases writting
		endpoint_list& el = m_configuration.fscp.contact_list;

		BOOST_FOREACH(endpoint& ep, m_last_contact_list_from_server)
		{
			const endpoint_list::iterator it = std::remove(el.begin(), el.end(), ep);

			if (it != el.end())
			{
				m_logger(LL_INFORMATION) << "Removing " << ep << " from the contact list.";

				el.erase(it, el.end());
			}
		}

		m_last_contact_list_from_server = ninfo.users_endpoints;

		BOOST_FOREACH(endpoint& ep, m_last_contact_list_from_server)
		{
			m_logger(LL_INFORMATION) << "Adding " << ep << " to the contact list.";

			el.push_back(ep);
		}
	}

	void core::set_identity(identity_store _identity)
	{
		m_configuration.security.identity.reset(_identity);

		if (m_server)
		{
			m_server->set_identity(_identity);
		}

		m_logger(LL_INFORMATION) << "Local client identity was updated.";

		using namespace cryptoplus::pkey;
		using namespace cryptoplus::x509;

		const certificate sig_cert = _identity.signature_certificate();
		const boost::posix_time::ptime not_after = sig_cert.not_after().to_ptime();
		const boost::posix_time::ptime renewal_date = not_after - CERTIFICATE_RENEWAL_DELAY;

		m_check_configuration_timer.expires_at(renewal_date);
		m_check_configuration_timer.async_wait(boost::bind(&core::do_check_configuration, this, boost::asio::placeholders::error));

		m_logger(LL_INFORMATION) << "Checking again configuration on " << boost::posix_time::to_simple_string(renewal_date) << ".";
	}
}

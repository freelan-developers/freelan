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

#include "os.hpp"
#include "tap_adapter_switch_port.hpp"
#include "endpoint_switch_port.hpp"
#include "logger_stream.hpp"

namespace freelan
{
	namespace
	{
		static const switch_::group_type TAP_ADAPTERS_GROUP = 0;
		static const switch_::group_type ENDPOINTS_GROUP = 1;
	}

	// Has to be put first, as static variables definition order matters
	const int core::ex_data_index = cryptoplus::x509::store_context::register_index();

	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);

	const std::string core::DEFAULT_SERVICE = "12000";

	core::core(boost::asio::io_service& io_service, const freelan::configuration& _configuration, const freelan::logger& _logger) :
		m_running(false),
		m_configuration(_configuration),
		m_logger(_logger),
		m_server(io_service, *m_configuration.security.identity),
		m_resolver(io_service),
		m_contact_timer(io_service, CONTACT_PERIOD),
		m_open_callback(),
		m_close_callback(),
		m_session_established_callback(),
		m_session_lost_callback(),
		m_arp_filter(m_ethernet_filter),
		m_ipv4_filter(m_ethernet_filter),
		m_udp_filter(m_ipv4_filter),
		m_bootp_filter(m_udp_filter),
		m_dhcp_filter(m_bootp_filter),
		m_switch(m_configuration.switch_)
	{
		m_server.set_hello_message_callback(boost::bind(&core::on_hello_request, this, _1, _2));
		m_server.set_presentation_message_callback(boost::bind(&core::on_presentation, this, _1, _2, _3, _4));
		m_server.set_session_request_message_callback(boost::bind(&core::on_session_request, this, _1, _2));
		m_server.set_session_established_callback(boost::bind(&core::on_session_established, this, _1));
		m_server.set_session_lost_callback(boost::bind(&core::on_session_lost, this, _1));
		m_server.set_data_message_callback(boost::bind(&core::on_data, this, _1, _2));

		if (m_configuration.tap_adapter.enabled)
		{
			m_tap_adapter.reset(new asiotap::tap_adapter(io_service));

			m_tap_adapter_switch_port = boost::make_shared<tap_adapter_switch_port>(boost::ref(*m_tap_adapter));
			m_switch.register_port(m_tap_adapter_switch_port, TAP_ADAPTERS_GROUP);
		}
	}

	void core::open()
	{
		typedef boost::asio::ip::udp::resolver::query query;

		m_logger(LL_DEBUG) << "Core opening...";

		// FSCP
		m_server.open(boost::apply_visitor(endpoint_resolve_visitor(m_resolver, to_protocol(m_configuration.fscp.hostname_resolution_protocol), query::address_configured | query::passive, DEFAULT_SERVICE), m_configuration.fscp.listen_on));

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

		do_contact();
		m_contact_timer.async_wait(boost::bind(&core::do_contact, this, boost::asio::placeholders::error));

		// Tap adapter
		if (m_tap_adapter)
		{
			m_tap_adapter->open();

			// IPv4 address
			if (m_configuration.tap_adapter.ipv4_address_prefix_length)
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
						    m_configuration.tap_adapter.ipv4_address_prefix_length->address,
						    m_configuration.tap_adapter.ipv4_address_prefix_length->prefix_length
						);
					}
#else
					m_tap_adapter->add_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length->address,
					    m_configuration.tap_adapter.ipv4_address_prefix_length->prefix_length
					);
#endif
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot set IPv4 address: " << ex.what();
				}
			}

			// IPv6 address
			if (m_configuration.tap_adapter.ipv6_address_prefix_length)
			{
				try
				{
					m_tap_adapter->add_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length->address,
					    m_configuration.tap_adapter.ipv6_address_prefix_length->prefix_length
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

				if (m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length)
				{
					m_dhcp_proxy->set_software_address(m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length->address);
				}

				if (m_configuration.tap_adapter.ipv4_address_prefix_length)
				{
					m_dhcp_proxy->add_entry(
					    m_tap_adapter->ethernet_address(),
					    m_configuration.tap_adapter.ipv4_address_prefix_length->address,
					    m_configuration.tap_adapter.ipv4_address_prefix_length->prefix_length
					);
				}
			}
			else
			{
				m_dhcp_proxy.reset();
			}

			if (m_configuration.tap_adapter.up_callback)
			{
				m_configuration.tap_adapter.up_callback(*m_tap_adapter);
			}
		}

		m_logger(LL_DEBUG) << "Core opened.";

		if (m_open_callback)
		{
			m_server.get_io_service().post(m_open_callback);
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
				m_server.get_io_service().post(m_close_callback);
			}

			m_server.get_io_service().post(boost::bind(&core::do_close, this));
		}
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
				m_configuration.tap_adapter.down_callback(*m_tap_adapter);
			}

			m_tap_adapter->cancel();
			m_tap_adapter->set_connected_state(false);

			// IPv6 address
			if (m_configuration.tap_adapter.ipv6_address_prefix_length)
			{
				try
				{
					m_tap_adapter->remove_ip_address_v6(
					    m_configuration.tap_adapter.ipv6_address_prefix_length->address,
					    m_configuration.tap_adapter.ipv6_address_prefix_length->prefix_length
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv6 address: " << ex.what();
				}
			}

			// IPv4 address
			if (m_configuration.tap_adapter.ipv4_address_prefix_length)
			{
				try
				{
					m_tap_adapter->remove_ip_address_v4(
					    m_configuration.tap_adapter.ipv4_address_prefix_length->address,
					    m_configuration.tap_adapter.ipv4_address_prefix_length->prefix_length
					);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(LL_WARNING) << "Cannot unset IPv4 address: " << ex.what();
				}
			}

			m_tap_adapter->close();
		}

		m_contact_timer.cancel();

		m_server.close();

		m_logger(LL_DEBUG) << "Core closed.";
	}

	void core::async_greet(const ep_type& target)
	{
		m_server.async_greet(target, boost::bind(&core::on_hello_response, this, _1, _2, _3), m_configuration.fscp.hello_timeout);
	}

	bool core::on_hello_request(const ep_type& sender, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received HELLO_REQUEST from " << sender << ".";

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
			m_logger(LL_DEBUG) << "Received HELLO_RESPONSE from " << sender << ". Latency: " << time_duration << ".";

			m_server.async_introduce_to(sender);
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
			m_server.async_request_session(sender);
			return true;
		}

		return false;
	}

	bool core::on_session_request(const ep_type& sender, bool default_accept)
	{
		m_logger(LL_DEBUG) << "Received SESSION_REQUEST from " << sender << ".";

		if (default_accept)
		{
			return true;
		}

		return false;
	}

	void core::on_session_established(const ep_type& sender)
	{
		m_logger(LL_INFORMATION) << "Session established with " << sender << ".";

		const switch_::port_type port = boost::make_shared<endpoint_switch_port>(boost::ref(m_server), sender);
		m_endpoint_switch_port_map[sender] = port;
		m_switch.register_port(port, ENDPOINTS_GROUP);

		if (m_session_established_callback)
		{
			m_session_established_callback(sender);
		}
	}

	void core::on_session_lost(const ep_type& sender)
	{
		m_logger(LL_INFORMATION) << "Session with " << sender << " lost.";

		const switch_::port_type port = m_endpoint_switch_port_map[sender];

		if (port)
		{
			m_switch.unregister_port(port);
			m_endpoint_switch_port_map.erase(sender);
		}

		if (m_session_lost_callback)
		{
			m_session_lost_callback(sender);
		}
	}

	void core::on_data(const ep_type& sender, boost::asio::const_buffer data)
	{
		const switch_::port_type port = m_endpoint_switch_port_map[sender];

		if (port)
		{
			m_switch.receive_data(port, data);
		}
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

	void core::do_greet(const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator it, const freelan::fscp_configuration::endpoint& ep)
	{
		if (!ec)
		{
			if (!m_server.has_session(*it))
			{
				m_logger(LL_DEBUG) << "Sending HELLO_REQUEST to " << ep_type(*it) << "...";

				async_greet(*it);
			}
		}
		else
		{
			m_logger(LL_WARNING) << "Failed to resolve " << ep << ".";
		}
	}

	void core::do_contact()
	{
		BOOST_FOREACH(const freelan::fscp_configuration::endpoint& ep, m_configuration.fscp.contact_list)
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
		if (m_tap_adapter)
		{
			m_tap_adapter->write(data);
		}
	}

	bool core::on_arp_request(const boost::asio::ip::address_v4& logical_address, ethernet_address_type& ethernet_address)
	{
		if (m_configuration.tap_adapter.ipv4_address_prefix_length)
		{
			if (logical_address != m_configuration.tap_adapter.ipv4_address_prefix_length->address)
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
}

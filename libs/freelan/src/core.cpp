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
//Use the behavior of Boost from bevor 1.63
#define BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX

#include "core.hpp"

#include "tools.hpp"
#include "client.hpp"
#include "routes_request_message.hpp"
#include "routes_message.hpp"

#include "server.hpp"
#include "client.hpp"

#include <fscp/server_error.hpp>

#include <asiotap/types/ip_network_address.hpp>

#ifdef WINDOWS
#include <executeplus/windows_system.hpp>
#else
#include <executeplus/posix_system.hpp>
#endif

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/future.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <cassert>

namespace freelan
{
	using fscp::SharedBuffer;

	typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adjustor;

	using boost::asio::buffer;
	using boost::asio::buffer_cast;
	using boost::asio::buffer_size;

	namespace
	{
		void null_simple_write_handler(const boost::system::error_code&)
		{
		}

		void null_switch_write_handler(const switch_::multi_write_result_type&)
		{
		}

		void null_router_write_handler(const boost::system::error_code&)
		{
		}

		asiotap::endpoint to_endpoint(const core::ep_type& host)
		{
			if (host.address().is_v4())
			{
				return asiotap::ipv4_endpoint(host.address().to_v4(), host.port());
			}
			else
			{
				return asiotap::ipv6_endpoint(host.address().to_v6(), host.port());
			}
		}

		template <typename Handler, typename CausalHandler>
		class causal_handler
		{
			private:

				class automatic_caller : public boost::noncopyable
				{
					public:

						automatic_caller(CausalHandler& _handler) :
							m_auto_handler(_handler)
						{
						}

						~automatic_caller()
						{
							m_auto_handler();
						}

					private:

						CausalHandler& m_auto_handler;
				};

			public:

				typedef void result_type;

				causal_handler(Handler _handler, CausalHandler _causal_handler) :
					m_handler(_handler),
					m_causal_handler(_causal_handler)
				{}

				result_type operator()()
				{
					automatic_caller ac(m_causal_handler);

					m_handler();
				}

				template <typename Arg1>
				result_type operator()(Arg1 arg1)
				{
					automatic_caller ac(m_causal_handler);

					m_handler(arg1);
				}

				template <typename Arg1, typename Arg2>
				result_type operator()(Arg1 arg1, Arg2 arg2)
				{
					automatic_caller ac(m_causal_handler);

					m_handler(arg1, arg2);
				}

			private:

				Handler m_handler;
				CausalHandler m_causal_handler;
		};

		template <typename Handler, typename CausalHandler>
		inline causal_handler<Handler, CausalHandler> make_causal_handler(Handler _handler, CausalHandler _causal_handler)
		{
			return causal_handler<Handler, CausalHandler>(_handler, _causal_handler);
		}

		unsigned int get_auto_mtu_value()
		{
			const unsigned int default_mtu_value = 1500;
			const size_t static_payload_size = 20 + 8 + 4 + 22; // IP + UDP + FSCP HEADER + FSCP DATA HEADER

			return default_mtu_value - static_payload_size;
		}

		size_t get_auto_mss_value(size_t mtu)
		{
			// This somehow is the magic number.
			const size_t static_payload_size = 20 + 8 + 4 + 22; // IP + UDP + FSCP HEADER + FSCP DATA HEADER
			return mtu - static_payload_size;
		}

		static const unsigned int TAP_ADAPTERS_GROUP = 0;
		static const unsigned int ENDPOINTS_GROUP = 1;

		asiotap::ip_route_set filter_routes(const asiotap::ip_route_set& routes, router_configuration::internal_route_scope_type scope, unsigned int limit, const asiotap::ip_network_address_list& network_addresses)
		{
			asiotap::ip_route_set result;
			auto ipv4_limit = limit;
			auto ipv6_limit = limit;

			auto check_limit = [limit, &ipv4_limit, &ipv6_limit](const asiotap::ip_route& route) {

				if (limit == 0)
				{
					return true;
				}

				const bool is_ipv4 = get_network_address(network_address(route)).is_v4();

				if (is_ipv4 ? ipv4_limit : ipv6_limit > 0)
				{
					(is_ipv4 ? ipv4_limit : ipv6_limit)--;

					return true;
				}

				return false;
			};

			switch (scope)
			{
				case router_configuration::internal_route_scope_type::none:
					break;
				case router_configuration::internal_route_scope_type::unicast_in_network:
				{
					for (auto&& ina: network_addresses)
					{
						for (auto&& route : routes)
						{
							if (is_unicast(route) && has_network(ina, network_address(route)))
							{
								if (check_limit(route))
								{
									result.insert(route);
								}
							}
						}
					}

					break;
				}
				case router_configuration::internal_route_scope_type::unicast:
				{
					for (auto&& route : routes)
					{
						if (is_unicast(route))
						{
							if (check_limit(route))
							{
								result.insert(route);
							}
						}
					}

					break;
				}
				case router_configuration::internal_route_scope_type::subnet:
				{
					for (auto&& ina : network_addresses)
					{
						for (auto&& route : routes)
						{
							if (has_network(ina, network_address(route)))
							{
								if (check_limit(route))
								{
									result.insert(route);
								}
							}
						}
					}

					break;
				}
				case router_configuration::internal_route_scope_type::any:
				{
					for (auto&& route : routes)
					{
						if (check_limit(route))
						{
							result.insert(route);
						}
					}

					break;
				}
			}

			return result;
		}

		asiotap::ip_route_set filter_routes(const asiotap::ip_route_set& routes, router_configuration::system_route_scope_type scope, unsigned int limit)
		{
			asiotap::ip_route_set result;
			auto ipv4_limit = limit;
			auto ipv6_limit = limit;

			auto check_limit = [limit, &ipv4_limit, &ipv6_limit](const asiotap::ip_route& route) {

				if (limit == 0)
				{
					return true;
				}

				const bool is_ipv4 = get_network_address(network_address(route)).is_v4();

				if (is_ipv4 ? ipv4_limit : ipv6_limit > 0)
				{
					(is_ipv4 ? ipv4_limit : ipv6_limit)--;

					return true;
				}

				return false;
			};

			switch (scope)
			{
				case router_configuration::system_route_scope_type::none:
					break;
				case router_configuration::system_route_scope_type::unicast:
				case router_configuration::system_route_scope_type::unicast_with_gateway:
				{
					for (auto&& route : routes)
					{
						if (is_unicast(route))
						{
							if ((scope == router_configuration::system_route_scope_type::unicast_with_gateway) || !has_gateway(route))
							{
								if (check_limit(route))
								{
									result.insert(route);
								}
							}
						}
					}

					break;
				}
				case router_configuration::system_route_scope_type::any:
				case router_configuration::system_route_scope_type::any_with_gateway:
				{
					for (auto&& route : routes)
					{
						if ((scope == router_configuration::system_route_scope_type::any_with_gateway) || !has_gateway(route))
						{
							if (check_limit(route))
							{
								result.insert(route);
							}
						}
					}

					break;
				}
			}

			return result;
		}

		asiotap::ip_address_set filter_dns_servers(const asiotap::ip_address_set& dns_servers, router_configuration::dns_servers_scope_type scope, const asiotap::ip_network_address_list& network_addresses)
		{
			asiotap::ip_address_set result;

			switch (scope)
			{
				case router_configuration::dns_servers_scope_type::none:
					break;
				case router_configuration::dns_servers_scope_type::in_network:
				{
					for (auto&& ina : network_addresses)
					{
						for (auto&& dns_server : dns_servers)
						{
							if (has_address(ina, dns_server.value()))
							{
								result.insert(dns_server);
							}
						}
					}

					break;
				}
				case router_configuration::dns_servers_scope_type::any:
				{
					result = dns_servers;
					break;
				}
			}

			return result;
		}

		void exponential_backoff_value(boost::posix_time::time_duration& value, const boost::posix_time::time_duration& min, const boost::posix_time::time_duration& max, float min_factor = 1.5f, float max_factor = 2.5f) {
			const auto factor = (static_cast<float>(::rand()) / static_cast<float>(RAND_MAX)) * (max_factor - min_factor) + min_factor;
			value = boost::posix_time::milliseconds(static_cast<int>(value.total_milliseconds() * factor));

			if (value < min) {
				value = min;
			} else if (value > max) {
				value = max;
			}
		}
	}

	typedef boost::asio::ip::udp::resolver::query resolver_query;

	// Has to be put first, as static variables definition order matters.
	const int core::ex_data_index = cryptoplus::x509::store_context::register_index();

	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);
	const boost::posix_time::time_duration core::DYNAMIC_CONTACT_PERIOD = boost::posix_time::seconds(45);
	const boost::posix_time::time_duration core::ROUTES_REQUEST_PERIOD = boost::posix_time::seconds(180);
	const boost::posix_time::time_duration core::RENEW_CERTIFICATE_WARNING_PERIOD = boost::posix_time::hours(6);
	const boost::posix_time::time_duration core::REGISTRATION_WARNING_PERIOD = boost::posix_time::minutes(5);
	const boost::posix_time::time_duration core::GET_CONTACT_INFORMATION_UPDATE_PERIOD = boost::posix_time::minutes(5);

	const std::string core::DEFAULT_SERVICE = "12000";

	core::core(boost::asio::io_service& io_service, const freelan::configuration& _configuration) :
		m_io_service(io_service),
		m_configuration(_configuration),
		m_logger_strand(m_io_service),
		m_logger(m_logger_strand.wrap(boost::bind(&core::do_handle_log, this, _1, _2, _3))),
		m_log_callback(),
		m_core_opened_callback(),
		m_core_closed_callback(),
		m_session_failed_callback(),
		m_session_error_callback(),
		m_session_established_callback(),
		m_session_lost_callback(),
		m_authentication_callback(),
		m_certificate_validation_callback(),
		m_tap_adapter_up_callback(),
		m_tap_adapter_down_callback(),
		m_dns_callback(),
		m_fscp_server(),
		m_contact_timer(m_io_service, CONTACT_PERIOD),
		m_dynamic_contact_timer(m_io_service, DYNAMIC_CONTACT_PERIOD),
		m_routes_request_timer(m_io_service, ROUTES_REQUEST_PERIOD),
		m_tap_adapter_io_service(),
		m_tap_adapter_thread(),
		m_arp_filter(m_ethernet_filter),
		m_ipv4_filter(m_ethernet_filter),
		m_ipv6_filter(m_ethernet_filter),
		m_udp_filter(m_ipv4_filter),
		m_tcpv4_filter(m_ipv4_filter),
		m_tcpv6_filter(m_ipv6_filter),
		m_bootp_filter(m_udp_filter),
		m_dhcp_filter(m_bootp_filter),
		m_tun_ipv4_filter(),
		m_tun_ipv6_filter(),
		m_tun_tcpv4_filter(m_tun_ipv4_filter),
		m_tun_tcpv6_filter(m_tun_ipv6_filter),
		m_tun_icmpv6_filter(m_tun_ipv6_filter),
		m_router_strand(m_io_service),
		m_switch(m_configuration.switch_),
		m_router(m_configuration.router),
		m_route_manager(m_io_service),
		m_dns_servers_manager(m_io_service),
		m_request_certificate(m_io_service, boost::posix_time::seconds(5), boost::posix_time::seconds(90)),
		m_request_ca_certificate(m_io_service, boost::posix_time::seconds(5), boost::posix_time::seconds(90)),
		m_renew_certificate_timer(m_io_service),
		m_registration_retry(m_io_service, boost::posix_time::seconds(5), boost::posix_time::seconds(90)),
		m_set_contact_information_retry(m_io_service, boost::posix_time::seconds(5), boost::posix_time::seconds(35)),
		m_get_contact_information_retry(m_io_service, boost::posix_time::seconds(5), boost::posix_time::seconds(35))
	{
		m_arp_filter.add_handler(boost::bind(&core::do_handle_arp_frame, this, _1));
		m_dhcp_filter.add_handler(boost::bind(&core::do_handle_dhcp_frame, this, _1));
		m_tun_icmpv6_filter.add_handler(boost::bind(&core::do_handle_icmpv6_frame, this, _1));
		m_tcpv4_filter.add_handler([this](asiotap::osi::mutable_helper<asiotap::osi::tcp_frame> tcp_helper){
			if (m_tcp_mss_morpher) {
				m_tcp_mss_morpher->handle(*m_tcpv4_filter.parent().get_last_helper(), tcp_helper);
			}
		});
		m_tcpv6_filter.add_handler([this](asiotap::osi::mutable_helper<asiotap::osi::tcp_frame> tcp_helper){
			if (m_tcp_mss_morpher) {
				m_tcp_mss_morpher->handle(*m_tcpv6_filter.parent().get_last_helper(), tcp_helper);
			}
		});
		m_tun_tcpv4_filter.add_handler([this](asiotap::osi::mutable_helper<asiotap::osi::tcp_frame> tcp_helper){
			if (m_tcp_mss_morpher) {
				m_tcp_mss_morpher->handle(*m_tun_tcpv4_filter.parent().get_last_helper(), tcp_helper);
			}
		});
		m_tun_tcpv6_filter.add_handler([this](asiotap::osi::mutable_helper<asiotap::osi::tcp_frame> tcp_helper){
			if (m_tcp_mss_morpher) {
				m_tcp_mss_morpher->handle(*m_tun_tcpv6_filter.parent().get_last_helper(), tcp_helper);
			}
		});

		// Setup the route manager.
		m_route_manager.set_route_registration_success_handler([this](const asiotap::route_manager::route_type& route){
			m_logger(fscp::log_level::information) << "Added system route: " << route;
		});
		m_route_manager.set_route_registration_failure_handler([this](const asiotap::route_manager::route_type& route, const boost::system::system_error& ex){
			m_logger(fscp::log_level::warning) << "Unable to add system route (" << route << "): " << ex.what();
		});
		m_route_manager.set_route_unregistration_success_handler([this](const asiotap::route_manager::route_type& route){
			m_logger(fscp::log_level::information) << "Removed system route: " << route;
		});
		m_route_manager.set_route_unregistration_failure_handler([this](const asiotap::route_manager::route_type& route, const boost::system::system_error& ex){
			m_logger(fscp::log_level::warning) << "Unable to remove system route (" << route << "): " << ex.what();
		});

		// Setup the DNS servers manager.
		m_dns_servers_manager.set_dns_server_add_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server) -> bool {
			if (m_dns_callback) {
				if (!m_dns_callback(dns_server.interface_name, DnsAction::Add, dns_server.dns_server_address.value())) {
					throw boost::system::system_error(make_error_code(asiotap::asiotap_error::external_process_execution_failed));
				}

				return true;
			} else {
#ifndef WINDOWS
				throw boost::system::system_error(make_error_code(asiotap::asiotap_error::no_dns_script_provided));
#endif
			}

			return false;
		});
		m_dns_servers_manager.set_dns_server_remove_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server) -> bool {
			if (m_dns_callback) {
				if (!m_dns_callback(dns_server.interface_name, DnsAction::Remove, dns_server.dns_server_address.value())) {
					throw boost::system::system_error(make_error_code(asiotap::asiotap_error::external_process_execution_failed));
				}

				return true;
			}
			else {
#ifndef WINDOWS
				m_logger(fscp::log_level::warning) << "Should have added a DNS server but no DNS script was configured.";
				throw boost::system::system_error(make_error_code(asiotap::asiotap_error::no_dns_script_provided));
#endif
			}

			return false;
		});
		m_dns_servers_manager.set_dns_server_registration_success_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server){
			m_logger(fscp::log_level::information) << "Added DNS server: " << dns_server;
		});
		m_dns_servers_manager.set_dns_server_registration_failure_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server, const boost::system::system_error& ex){
			m_logger(fscp::log_level::warning) << "Unable to add DNS server (" << dns_server << "): " << ex.what();
		});
		m_dns_servers_manager.set_dns_server_unregistration_success_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server){
			m_logger(fscp::log_level::information) << "Removed DNS server: " << dns_server;
		});
		m_dns_servers_manager.set_dns_server_unregistration_failure_handler([this](const asiotap::dns_servers_manager::dns_server_type& dns_server, const boost::system::system_error& ex){
			m_logger(fscp::log_level::warning) << "Unable to remove DNS server (" << dns_server << "): " << ex.what();
		});
	}

	void core::open()
	{
		m_logger(fscp::log_level::debug) << "Opening core...";

		open_web_client();

		if (m_configuration.security.identity || !m_configuration.client.enabled)
		{
			open_fscp_server();
		}

		open_tap_adapter();
		open_web_server();

		m_logger(fscp::log_level::debug) << "Core opened.";
	}

	void core::close()
	{
		m_logger(fscp::log_level::debug) << "Closing core...";

		close_web_server();
		close_tap_adapter();
		close_fscp_server();
		close_web_client();

		m_logger(fscp::log_level::debug) << "Core closed.";
	}

	// Private methods

	void core::do_handle_log(fscp::log_level level, const std::string& msg, const boost::posix_time::ptime& timestamp)
	{
		// All do_handle_log() calls are done within the same strand, so the user does not need to protect his callback with a mutex that might slow things down.
		if (m_log_callback)
		{
			m_log_callback(level, msg, timestamp);
		}
	}

	bool core::is_banned(const boost::asio::ip::address& address) const
	{
		return has_address(m_configuration.fscp.never_contact_list.begin(), m_configuration.fscp.never_contact_list.end(), address);
	}

	void core::open_fscp_server()
	{
		if (!m_configuration.security.identity)
		{
			m_logger(fscp::log_level::warning) << "No user certificate/private key or pre-shared key set. Generating a temporary certificate/private key pair...";

			const auto private_key = generate_private_key();
			const auto certificate = generate_self_signed_certificate(private_key);

			m_configuration.security.identity = fscp::identity_store(certificate, private_key);

			m_logger(fscp::log_level::warning) << "Using a generated temporary certificate (" << certificate.subject() << ") prevents reliable authentication ! Generate and specify a static certificate/key pair for use in production.";
		}
		else
		{
			if (!!m_configuration.security.identity->signature_certificate())
			{
				m_logger(fscp::log_level::information) << "Enabling certificate-based authentication.";
			}

			if (!m_configuration.security.identity->pre_shared_key().empty())
			{
				m_logger(fscp::log_level::information) << "Enabling pre-shared key authentication.";
			}
		}

		m_logger(fscp::log_level::information) << "Starting FSCP server...";

		m_fscp_server = boost::make_shared<fscp::server>(boost::ref(m_io_service), boost::ref(m_logger), boost::cref(*m_configuration.security.identity));

		try
		{
			m_fscp_server->set_cipher_suites(m_configuration.fscp.cipher_suite_capabilities);
			m_fscp_server->set_elliptic_curves(m_configuration.fscp.elliptic_curve_capabilities);
			m_fscp_server->set_hello_max_per_second(m_configuration.fscp.max_unauthenticated_messages_per_second);
			m_fscp_server->set_presentation_max_per_second(m_configuration.fscp.max_unauthenticated_messages_per_second);

			m_fscp_server->set_hello_message_received_callback(boost::bind(&core::do_handle_hello_received, this, _1, _2));
			m_fscp_server->set_contact_request_received_callback(boost::bind(&core::do_handle_contact_request_received, this, _1, _2, _3, _4));
			m_fscp_server->set_contact_received_callback(boost::bind(&core::do_handle_contact_received, this, _1, _2, _3));
			m_fscp_server->set_presentation_message_received_callback(boost::bind(&core::do_handle_presentation_received, this, _1, _2, _3, _4));
			m_fscp_server->set_session_request_message_received_callback(boost::bind(&core::do_handle_session_request_received, this, _1, _2, _3, _4));
			m_fscp_server->set_session_message_received_callback(boost::bind(&core::do_handle_session_received, this, _1, _2, _3, _4));
			m_fscp_server->set_session_failed_callback(boost::bind(&core::do_handle_session_failed, this, _1, _2));
			m_fscp_server->set_session_error_callback(boost::bind(&core::do_handle_session_error, this, _1, _2, _3));
			m_fscp_server->set_session_established_callback(boost::bind(&core::do_handle_session_established, this, _1, _2, _3, _4));
			m_fscp_server->set_session_lost_callback(boost::bind(&core::do_handle_session_lost, this, _1, _2));
			m_fscp_server->set_data_received_callback(boost::bind(&core::do_handle_data_received, this, _1, _2, _3, _4));

			resolver_type resolver(m_io_service);

			const ep_type listen_endpoint = boost::apply_visitor(
				asiotap::endpoint_resolve_visitor(
					resolver,
					to_protocol(m_configuration.fscp.hostname_resolution_protocol),
					resolver_query::address_configured | resolver_query::passive, DEFAULT_SERVICE
				),
				m_configuration.fscp.listen_on
			);

			m_logger(fscp::log_level::important) << "Core set to listen on: " << listen_endpoint;

			if (m_configuration.security.certificate_validation_method == security_configuration::CVM_DEFAULT)
			{
				build_ca_store(build_ca_store_when::it_doesnt_exist);
			}

			for(auto&& network_address : m_configuration.fscp.never_contact_list)
			{
				m_logger(fscp::log_level::information) << "Configured not to accept requests from: " << network_address;
			}

			// Let's open the server.
			m_fscp_server->open(listen_endpoint);

			if (m_configuration.fscp.upnp_enabled)
			{
#ifdef USE_UPNP
				m_fscp_server->upnp_punch_hole(listen_endpoint.port());
#else
				m_logger(fscp::log_level::error) << "UPnP support is not compiled in this version";
#endif
			}

#ifdef LINUX
			if (!m_configuration.fscp.listen_on_device.empty())
			{
				const auto socket_fd = m_fscp_server->get_socket().native_handle();
				const std::string device_name = m_configuration.fscp.listen_on_device;

				if (::setsockopt(socket_fd, SOL_SOCKET, SO_BINDTODEVICE, device_name.c_str(), device_name.size()) == 0)
				{
					m_logger(fscp::log_level::important) << "Restricting VPN traffic on: " << device_name;
				}
				else
				{
					m_logger(fscp::log_level::warning) << "Unable to restrict traffic on: " << device_name << ". Error was: " << boost::system::error_code(errno, boost::system::system_category()).message();
				}
			}
#endif

			// We start the contact loop.
			async_contact_all();

			m_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_contact, this, boost::asio::placeholders::error));
			m_dynamic_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_dynamic_contact, this, boost::asio::placeholders::error));
			m_routes_request_timer.async_wait(boost::bind(&core::do_handle_periodic_routes_request, this, boost::asio::placeholders::error));

			m_logger(fscp::log_level::information) << "FSCP server started.";
		}
		catch (std::exception& ex)
		{
			m_logger(fscp::log_level::error) << "Unable to start the FSCP server: " << ex.what();

			close_fscp_server();

			throw;
		}
	}

	void core::close_fscp_server()
	{
		if (m_fscp_server)
		{
			m_logger(fscp::log_level::information) << "Closing FSCP server...";

			// Stop the contact loop timers.
			m_routes_request_timer.cancel();
			m_dynamic_contact_timer.cancel();
			m_contact_timer.cancel();

			m_fscp_server->close();

			m_logger(fscp::log_level::information) << "FSCP server closed.";
		}
	}

	void core::async_contact(const endpoint& target, duration_handler_type handler)
	{
		m_logger(fscp::log_level::debug) << "Resolving " << target << " for potential contact...";

		// This is a ugly workaround for a bug in Boost::Variant (<1.55)
		endpoint target1 = target;

		const auto resolve_handler = [this, handler, target1] (const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator it)
		{
			if (!ec)
			{
				const ep_type host = *it;

				// This is a ugly workaround for a bug in Boost::Variant (<1.55)
				endpoint target2 = target1;

				// The host was resolved: we first make sure no session exist with that host before doing anything else.
				m_fscp_server->async_has_session_with_endpoint(
					host,
					[this, handler, host, target2] (bool has_session)
					{
						if (!has_session)
						{
							m_logger(fscp::log_level::debug) << "No session exists with " << target2 << " (at " << host << "). Contacting...";

							do_contact(host, handler);
						}
						else
						{
							m_logger(fscp::log_level::debug) << "A session already exists with " << target2 << " (at " << host << "). Not contacting again.";
						}
					}
				);
			}
			else
			{
				handler(ep_type(), ec, boost::posix_time::time_duration());
			}
		};

		boost::apply_visitor(
			asiotap::endpoint_async_resolve_visitor(
				boost::make_shared<resolver_type>(m_io_service),
				to_protocol(m_configuration.fscp.hostname_resolution_protocol),
				resolver_query::address_configured,
				DEFAULT_SERVICE,
				resolve_handler
			),
			target
		);
	}

	void core::async_contact(const endpoint& target)
	{
		async_contact(target, boost::bind(&core::do_handle_contact, this, target, _1, _2, _3));
	}

	void core::async_contact_all()
	{
		for (auto&& contact : m_configuration.fscp.contact_list)
		{
			async_contact(contact);
		}
	}

	void core::async_dynamic_contact_all()
	{
		using boost::make_transform_iterator;

		hash_type (*func)(cert_type) = fscp::get_certificate_hash;

		const hash_list_type hash_list(make_transform_iterator(m_configuration.fscp.dynamic_contact_list.begin(), func), make_transform_iterator(m_configuration.fscp.dynamic_contact_list.end(), func));

		async_send_contact_request_to_all(hash_list);
	}

	void core::async_send_contact_request_to_all(const hash_list_type& hash_list, multiple_endpoints_handler_type handler)
	{
		m_fscp_server->async_send_contact_request_to_all(hash_list, handler);
	}

	void core::async_send_contact_request_to_all(const hash_list_type& hash_list)
	{
		async_send_contact_request_to_all(hash_list, boost::bind(&core::do_handle_send_contact_request_to_all, this, _1));
	}

	void core::async_introduce_to(const ep_type& target, simple_handler_type handler)
	{
		assert(m_fscp_server);

		m_fscp_server->async_introduce_to(target, handler);
	}

	void core::async_introduce_to(const ep_type& target)
	{
		async_introduce_to(target, boost::bind(&core::do_handle_introduce_to, this, target, _1));
	}

	void core::async_request_session(const ep_type& target, simple_handler_type handler)
	{
		assert(m_fscp_server);

		m_logger(fscp::log_level::debug) << "Sending SESSION_REQUEST to " << target << ".";

		m_fscp_server->async_request_session(target, handler);
	}

	void core::async_request_session(const ep_type& target)
	{
		async_request_session(target, boost::bind(&core::do_handle_request_session, this, target, _1));
	}

	void core::async_handle_routes_request(const ep_type& sender, const routes_request_message& msg)
	{
		// The routes request message does not contain any meaningful information.
		static_cast<void>(msg);

		m_router_strand.post(
			boost::bind(
				&core::do_handle_routes_request,
				this,
				sender
			)
		);
	}

	void core::async_handle_routes(const ep_type& sender, const routes_message& msg)
	{
		const auto version = msg.version();
		const auto routes = msg.routes();
		const auto dns_servers = msg.dns_servers();

		async_get_tap_addresses([this, sender, version, routes, dns_servers](const asiotap::ip_network_address_list& ip_addresses){
			m_router_strand.post(
				boost::bind(
					&core::do_handle_routes,
					this,
					ip_addresses,
					sender,
					version,
					routes,
					dns_servers
				)
			);
		});
	}

	void core::async_send_routes_request(const ep_type& target, simple_handler_type handler)
	{
		assert(m_fscp_server);

		m_logger(fscp::log_level::debug) << "Sending routes request to " << target << ".";

		const auto data_buffer = SharedBuffer(2048);
		const size_t size = routes_request_message::write(
			buffer_cast<uint8_t*>(data_buffer),
			buffer_size(data_buffer)
		);

		m_fscp_server->async_send_data(
			target,
			fscp::CHANNEL_NUMBER_1,
			buffer(data_buffer, size),
			make_shared_buffer_handler(
				data_buffer,
				handler
			)
		);
	}

	void core::async_send_routes_request(const ep_type& target)
	{
		async_send_routes_request(target, boost::bind(&core::do_handle_send_routes_request, this, target, _1));
	}

	void core::async_send_routes_request_to_all(multiple_endpoints_handler_type handler)
	{
		assert(m_fscp_server);

		m_logger(fscp::log_level::debug) << "Sending routes request to all hosts.";

		const auto data_buffer = SharedBuffer(2048);
		const size_t size = routes_request_message::write(
			buffer_cast<uint8_t*>(data_buffer),
			buffer_size(data_buffer)
		);

		m_fscp_server->async_send_data_to_all(
			fscp::CHANNEL_NUMBER_1,
			buffer(data_buffer, size),
			make_shared_buffer_handler(
				data_buffer,
				handler
			)
		);
	}

	void core::async_send_routes_request_to_all()
	{
		async_send_routes_request_to_all(boost::bind(&core::do_handle_send_routes_request_to_all, this, _1));
	}

	void core::async_send_routes(const ep_type& target, routes_message::version_type version, const asiotap::ip_route_set& routes, const asiotap::ip_address_set& dns_servers, simple_handler_type handler)
	{
		assert(m_fscp_server);

		m_logger(fscp::log_level::debug) << "Sending routes and DNS servers to " << target << ": version " << version << " (" << routes << "), (" << dns_servers << ").";

		const auto data_buffer = SharedBuffer(8192);
		const size_t size = routes_message::write(
			buffer_cast<uint8_t*>(data_buffer),
			buffer_size(data_buffer),
			version,
			routes,
			dns_servers
		);

		m_fscp_server->async_send_data(
			target,
			fscp::CHANNEL_NUMBER_1,
			buffer(data_buffer, size),
			make_shared_buffer_handler(
				data_buffer,
				handler
			)
		);
	}

	void core::do_contact(const ep_type& address, duration_handler_type handler)
	{
		assert(m_fscp_server);

		m_logger(fscp::log_level::debug) << "Sending HELLO to " << address;

		m_fscp_server->async_greet(address, boost::bind(handler, address, _1, _2));
	}

	void core::do_handle_contact(const endpoint& host, const ep_type& address, const boost::system::error_code& ec, const boost::posix_time::time_duration& duration)
	{
		if (!ec)
		{
			m_logger(fscp::log_level::debug) << "Received HELLO_RESPONSE from " << host << " at " << address << ". Latency: " << duration << "";

			async_introduce_to(address);
		}
		else
		{
			if (ec == fscp::server_error::hello_request_timed_out)
			{
				m_logger(fscp::log_level::debug) << "Received no HELLO_RESPONSE from " << host << " at " << address << ": " << ec.message() << " (timeout: " << duration << ")";
			}
			else
			{
				m_logger(fscp::log_level::debug) << "Unable to send HELLO to " << host << ": " << ec.message();
			}
		}
	}

	void core::do_handle_periodic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			async_contact_all();

			m_contact_timer.expires_from_now(CONTACT_PERIOD);
			m_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_handle_periodic_dynamic_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			async_dynamic_contact_all();

			m_dynamic_contact_timer.expires_from_now(DYNAMIC_CONTACT_PERIOD);
			m_dynamic_contact_timer.async_wait(boost::bind(&core::do_handle_periodic_dynamic_contact, this, boost::asio::placeholders::error));
		}
	}

	void core::do_handle_periodic_routes_request(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			async_send_routes_request_to_all();

			m_routes_request_timer.expires_from_now(ROUTES_REQUEST_PERIOD);
			m_routes_request_timer.async_wait(boost::bind(&core::do_handle_periodic_routes_request, this, boost::asio::placeholders::error));
		}
	}

	void core::do_handle_send_contact_request(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(fscp::log_level::warning) << "Error sending contact request to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_send_contact_request_to_all(const std::map<ep_type, boost::system::error_code>& results)
	{
		for (std::map<ep_type, boost::system::error_code>::const_iterator result = results.begin(); result != results.end(); ++result)
		{
			do_handle_send_contact_request(result->first, result->second);
		}
	}

	void core::do_handle_introduce_to(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(fscp::log_level::warning) << "Error sending introduction message to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_request_session(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(fscp::log_level::warning) << "Error requesting session to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_send_routes_request(const ep_type& target, const boost::system::error_code& ec)
	{
		if (ec)
		{
			m_logger(fscp::log_level::warning) << "Error sending routes request to " << target << ": " << ec.message();
		}
	}

	void core::do_handle_send_routes_request_to_all(const std::map<ep_type, boost::system::error_code>& results)
	{
		for (std::map<ep_type, boost::system::error_code>::const_iterator result = results.begin(); result != results.end(); ++result)
		{
			do_handle_send_routes_request(result->first, result->second);
		}
	}

	bool core::do_handle_hello_received(const ep_type& sender, bool default_accept)
	{
		m_logger(fscp::log_level::debug) << "Received HELLO_REQUEST from " << sender << ".";

		if (is_banned(sender.address()))
		{
			m_logger(fscp::log_level::warning) << "Ignoring HELLO_REQUEST from " << sender << " as it is a banned host.";

			default_accept = false;
		}

		if (default_accept)
		{
			async_introduce_to(sender);
		}

		return default_accept;
	}

	bool core::do_handle_contact_request_received(const ep_type& sender, cert_type cert, hash_type hash, const ep_type& answer)
	{
		if (m_configuration.fscp.accept_contact_requests)
		{
			m_logger(fscp::log_level::information) << "Received contact request from " << sender << " for " << cert.subject() << " (" << hash << "). Host is at: " << answer;

			return true;
		}
		else
		{
			return false;
		}
	}

	void core::do_handle_contact_received(const ep_type& sender, hash_type hash, const ep_type& answer)
	{
		if (m_configuration.fscp.accept_contacts)
		{
			// We check if the contact belongs to the forbidden network list.
			if (is_banned(answer.address()))
			{
				m_logger(fscp::log_level::warning) << "Received forbidden contact from " << sender << ": " << hash << " is at " << answer << " but won't be contacted.";
			}
			else
			{
				m_logger(fscp::log_level::information) << "Received contact from " << sender << ": " << hash << " is at: " << answer;

				async_contact(to_endpoint(answer));
			}
		}
	}

	bool core::do_handle_presentation_received(const ep_type& sender, cert_type sig_cert, fscp::server::presentation_status_type status, bool has_session)
	{
		if (m_logger.level() <= fscp::log_level::debug)
		{
			if (!!sig_cert)
			{
				m_logger(fscp::log_level::debug) << "Received PRESENTATION from " << sender << ": " << sig_cert.subject() << ".";
			}
			else
			{
				m_logger(fscp::log_level::debug) << "Received PRESENTATION from " << sender << " using pre-shared key authentication.";
			}
		}

		if (is_banned(sender.address()))
		{
			m_logger(fscp::log_level::warning) << "Ignoring PRESENTATION from " << sender << " as it is a banned host.";

			return false;
		}

		if (has_session)
		{
			m_logger(fscp::log_level::warning) << "Ignoring PRESENTATION from " << sender << " as an active session currently exists with this host.";

			return false;
		}

		if (!!sig_cert)
		{
			if (!certificate_is_valid(sig_cert))
			{
				m_logger(fscp::log_level::warning) << "Ignoring PRESENTATION from " << sender << " as the signature certificate is invalid.";

				return false;
			}

			m_logger(fscp::log_level::information) << "Accepting PRESENTATION from " << sender << " (" << sig_cert.subject() << "): " << status << ".";
		}
		else
		{
			m_logger(fscp::log_level::information) << "Accepting PRESENTATION from " << sender << " for pre-shared key authentication: " << status << ".";
		}

		async_request_session(sender);

		return true;
	}

	bool core::do_handle_session_request_received(const ep_type& sender, const fscp::cipher_suite_list_type& cscap, const fscp::elliptic_curve_list_type& eccap, bool default_accept)
	{
		m_logger(fscp::log_level::debug) << "Received SESSION_REQUEST from " << sender << " (default: " << (default_accept ? std::string("accept") : std::string("deny")) << ").";

		if (m_logger.level() <= fscp::log_level::debug)
		{
			std::ostringstream oss;

			for (auto&& cs : cscap)
			{
				oss << " " << cs;
			}

			m_logger(fscp::log_level::debug) << "Cipher suites capabilities:" << oss.str();

			oss.str("");

			for (auto&& ec : eccap)
			{
				oss << " " << ec;
			}

			m_logger(fscp::log_level::debug) << "Elliptic curve capabilities:" << oss.str();
		}

		return default_accept;
	}

	bool core::do_handle_session_received(const ep_type& sender, fscp::cipher_suite_type cs, fscp::elliptic_curve_type ec, bool default_accept)
	{
		m_logger(fscp::log_level::debug) << "Received SESSION from " << sender << " (default: " << (default_accept ? std::string("accept") : std::string("deny")) << ").";
		m_logger(fscp::log_level::debug) << "Cipher suite: " << cs;
		m_logger(fscp::log_level::debug) << "Elliptic curve: " << ec;

		return default_accept;
	}

	void core::do_handle_session_failed(const ep_type& host, bool is_new)
	{
		if (is_new)
		{
			m_logger(fscp::log_level::warning) << "Session establishment with " << host << " failed.";
		}
		else
		{
			m_logger(fscp::log_level::warning) << "Session renewal with " << host << " failed.";
		}

		if (m_session_failed_callback)
		{
			m_session_failed_callback(host, is_new);
		}
	}

	void core::do_handle_session_error(const ep_type& host, bool is_new, const std::exception& error)
	{
		if (is_new)
		{
			m_logger(fscp::log_level::warning) << "Session establishment with " << host << " encountered an error: " << error.what();
		}
		else
		{
			m_logger(fscp::log_level::warning) << "Session renewal with " << host << " encountered an error: " << error.what();
		}

		if (m_session_error_callback)
		{
			m_session_error_callback(host, is_new, error);
		}
	}

	void core::do_handle_session_established(const ep_type& host, bool is_new, const fscp::cipher_suite_type& cs, const fscp::elliptic_curve_type& ec)
	{
		if (is_new)
		{
			m_logger(fscp::log_level::important) << "Session established with " << host << ".";
		}
		else
		{
			m_logger(fscp::log_level::information) << "Session renewed with " << host << ".";
		}

		m_logger(fscp::log_level::information) << "Cipher suite: " << cs;
		m_logger(fscp::log_level::information) << "Elliptic curve: " << ec;

		if (is_new)
		{
			if (m_configuration.tap_adapter.type == tap_adapter_configuration::tap_adapter_type::tap)
			{
				async_register_switch_port(host, boost::bind(&core::async_send_routes_request, this, host));
			}
			else
			{
				// We register the router port without any routes, at first.
				async_register_router_port(host, boost::bind(&core::async_send_routes_request, this, host));
			}

			const auto route = m_route_manager.get_route_for(host.address());
			async_save_system_route(host, route, void_handler_type());
		}

		if (m_session_established_callback)
		{
			m_session_established_callback(host, is_new, cs, ec);
		}
	}

	void core::do_handle_session_lost(const ep_type& host, fscp::server::session_loss_reason reason)
	{
		m_logger(fscp::log_level::important) << "Session with " << host << " lost (" << reason << ").";

		if (m_session_lost_callback)
		{
			m_session_lost_callback(host, reason);
		}

		if (m_configuration.tap_adapter.type == tap_adapter_configuration::tap_adapter_type::tap)
		{
			async_unregister_switch_port(host, void_handler_type());
		}
		else
		{
			async_unregister_router_port(host, void_handler_type());
		}

		async_clear_client_router_info(host, void_handler_type());
	}

	void core::do_handle_data_received(const ep_type& sender, fscp::channel_number_type channel_number, fscp::SharedBuffer buffer, boost::asio::const_buffer data)
	{
		switch (channel_number)
		{
			// Channel 0 contains ethernet/ip frames
			case fscp::CHANNEL_NUMBER_0:
				if (m_configuration.tap_adapter.type == tap_adapter_configuration::tap_adapter_type::tap)
				{
					async_write_switch(
						make_port_index(sender),
						data,
						make_shared_buffer_handler(
							buffer,
							&null_switch_write_handler
						)
					);
				}
				else
				{
					async_write_router(
						make_port_index(sender),
						data,
						make_shared_buffer_handler(
							buffer,
							&null_router_write_handler
						)
					);
				}

				break;
			// Channel 1 contains messages
			case fscp::CHANNEL_NUMBER_1:
				try
				{
					const message msg(buffer_cast<const uint8_t*>(data), buffer_size(data));

					do_handle_message(sender, buffer, msg);
				}
				catch (std::runtime_error& ex)
				{
					m_logger(fscp::log_level::warning) << "Received incorrectly formatted message from " << sender << ". Error was: " << ex.what();
				}

				break;
			default:
				m_logger(fscp::log_level::warning) << "Received unhandled " << buffer_size(data) << " byte(s) of data on FSCP channel #" << static_cast<int>(channel_number);
				break;
		}
	}

	void core::do_handle_message(const ep_type& sender, fscp::SharedBuffer, const message& msg)
	{
		switch (msg.type())
		{
			case message::MT_ROUTES_REQUEST:
				{
					routes_request_message rr_msg(msg);

					async_handle_routes_request(sender, rr_msg);

					break;
				}

			case message::MT_ROUTES:
				{
					routes_message r_msg(msg);

					async_handle_routes(sender, r_msg);

					break;
				}

			default:
				m_logger(fscp::log_level::warning) << "Received unhandled message of type " << static_cast<int>(msg.type()) << " on the message channel";
				break;
		}
	}

	void core::do_handle_routes_request(const ep_type& sender)
	{
		// All calls to do_handle_routes_request() are done within the m_router_strand, so the following is safe.
		if (!m_configuration.router.accept_routes_requests)
		{
			m_logger(fscp::log_level::debug) << "Received routes request from " << sender << " but ignoring as specified in the configuration";
		}
		else
		{
			if (m_tap_adapter && (m_tap_adapter->layer() == asiotap::tap_adapter_layer::ip))
			{
				const auto local_port = m_router.get_port(make_port_index(m_tap_adapter));

				if (m_local_routes_version.is_initialized())
				{
					const auto routes = local_port->local_routes();
					const auto dns_servers = local_port->local_dns_servers();

					m_logger(fscp::log_level::debug) << "Received routes request from " << sender << ". Replying with version " << *m_local_routes_version << ": " << routes << ". DNS: " << dns_servers;

					async_send_routes(sender, *m_local_routes_version, routes, dns_servers, &null_simple_write_handler);
				}
				else
				{
					m_logger(fscp::log_level::debug) << "Received routes request from " << sender << " but no local routes are set. Not sending anything.";
				}
			}
			else
			{
				const auto routes = translate_ip_routes(m_configuration.router.local_ip_routes);
				const auto dns_servers = m_configuration.router.local_dns_servers;
				const auto version = 0;

				m_logger(fscp::log_level::debug) << "Received routes request from " << sender << ". Replying with version " << version << ": " << routes;

				async_send_routes(sender, version, routes, dns_servers, &null_simple_write_handler);
			}
		}
	}

	void core::do_handle_routes(const asiotap::ip_network_address_list& tap_addresses, const ep_type& sender, routes_message::version_type version, const asiotap::ip_route_set& routes, const asiotap::ip_address_set& dns_servers)
	{
		// All calls to do_handle_routes() are done within the m_router_strand, so the following is safe.

		client_router_info_type& client_router_info = m_client_router_info_map[sender];

		if (!client_router_info.is_older_than(version))
		{
			m_logger(fscp::log_level::debug) << "Ignoring old routes message with version " << version << " as current version is " << *client_router_info.version;

			return;
		}

		if (!m_tap_adapter)
		{
			m_logger(fscp::log_level::information) << "Ignoring routes message as no tap adapter is currently associated.";

			return;
		}

		asiotap::ip_address_set filtered_dns_servers;

		if (m_configuration.router.dns_servers_acceptance_policy == router_configuration::dns_servers_scope_type::none)
		{
			if (!dns_servers.empty()) {
				m_logger(fscp::log_level::warning) << "Received DNS servers from " << sender << " (version " << version << ") will be ignored, as the configuration requires: " << dns_servers;
			}
		}
		else
		{
			filtered_dns_servers = filter_dns_servers(dns_servers, m_configuration.router.dns_servers_acceptance_policy, tap_addresses);

			if (filtered_dns_servers != dns_servers)
			{
				if (filtered_dns_servers.empty())
				{
					m_logger(fscp::log_level::warning) << "Received DNS servers from " << sender << " (version " << version << ") but none matched the DNS servers acceptance policy (" << m_configuration.router.dns_servers_acceptance_policy << "): " << dns_servers;
				}
				else
				{
					asiotap::ip_address_set excluded_dns_servers;
					std::set_difference(dns_servers.begin(), dns_servers.end(), filtered_dns_servers.begin(), filtered_dns_servers.end(), std::inserter(excluded_dns_servers, excluded_dns_servers.end()));

					m_logger(fscp::log_level::warning) << "Received DNS servers from " << sender << " (version " << version << ") but some did not match the DNS servers acceptance policy (" << m_configuration.router.dns_servers_acceptance_policy << "): " << excluded_dns_servers;
				}
			}

			if (!filtered_dns_servers.empty())
			{
				if ((m_tap_adapter->layer() == asiotap::tap_adapter_layer::ip))
				{
					const auto port = m_router.get_port(make_port_index(sender));

					if (port)
					{
						port->set_local_dns_servers(filtered_dns_servers);

						m_logger(fscp::log_level::information) << "Received DNS servers from " << sender << " (version " << version << ") were saved: " << filtered_dns_servers;
					}
					else
					{
						m_logger(fscp::log_level::debug) << "Received DNS servers from " << sender << " but unable to get the associated router port. Doing nothing";
					}
				}
			}
		}

		asiotap::ip_route_set filtered_routes;

		if (m_tap_adapter->layer() == asiotap::tap_adapter_layer::ip)
		{
			if (m_configuration.router.internal_route_acceptance_policy == router_configuration::internal_route_scope_type::none)
			{
				if (!routes.empty()) {
					m_logger(fscp::log_level::warning) << "Received routes from " << sender << " (version " << version << ") will be ignored, as the configuration requires: " << routes;
				}
			}
			else
			{
				filtered_routes = filter_routes(routes, m_configuration.router.internal_route_acceptance_policy, m_configuration.router.maximum_routes_limit, tap_addresses);

				if (filtered_routes != routes)
				{
					if (filtered_routes.empty())
					{
						m_logger(fscp::log_level::warning) << "Received routes from " << sender << " (version " << version << ") but none matched the internal route acceptance policy (" << m_configuration.router.internal_route_acceptance_policy << ", limit " << m_configuration.router.maximum_routes_limit << "): " << routes;
					}
					else
					{
						asiotap::ip_route_set excluded_routes;
						std::set_difference(routes.begin(), routes.end(), filtered_routes.begin(), filtered_routes.end(), std::inserter(excluded_routes, excluded_routes.end()));

						m_logger(fscp::log_level::warning) << "Received routes from " << sender << " (version " << version << ") but some did not match the internal route acceptance policy (" << m_configuration.router.internal_route_acceptance_policy << ", limit " << m_configuration.router.maximum_routes_limit << "): " << excluded_routes;
					}
				}

				if (!filtered_routes.empty())
				{
					const auto port = m_router.get_port(make_port_index(sender));

					if (port)
					{
						port->set_local_routes(filtered_routes);

						m_logger(fscp::log_level::information) << "Received routes from " << sender << " (version " << version << ") were applied: " << filtered_routes;
					}
					else
					{
						m_logger(fscp::log_level::debug) << "Received routes from " << sender << " but unable to get the associated router port. Doing nothing";
					}
				}
			}
		}
		else
		{
			if (m_configuration.router.system_route_acceptance_policy == router_configuration::system_route_scope_type::none)
			{
				if (!routes.empty()) {
					m_logger(fscp::log_level::warning) << "Received routes from " << sender << " (version " << version << ") will be ignored, as the configuration requires: " << routes;
				}
			}
			else
			{
				filtered_routes = routes;
			}
		}

		// Silently filter out routes that are already covered by the default interface routing table entries (aka. routes that belong to the interface's network).
		asiotap::ip_route_set filtered_system_routes;

		for (auto&& ina: tap_addresses)
		{
			for (auto&& route : filtered_routes)
			{
				if (!has_network(ina, network_address(route)))
				{
					filtered_system_routes.insert(route);
				}
			}
		}

		const auto system_routes = filter_routes(filtered_system_routes, m_configuration.router.system_route_acceptance_policy, m_configuration.router.maximum_routes_limit);

		if (system_routes != filtered_system_routes)
		{
			if (system_routes.empty() && !filtered_system_routes.empty())
			{
				m_logger(fscp::log_level::warning) << "Received system routes from " << sender << " (version " << version << ") but none matched the system route acceptance policy (" << m_configuration.router.system_route_acceptance_policy << ", limit " << m_configuration.router.maximum_routes_limit << "): " << filtered_system_routes;
			}
			else
			{
				asiotap::ip_route_set excluded_routes;
				std::set_difference(filtered_system_routes.begin(), filtered_system_routes.end(), system_routes.begin(), system_routes.end(), std::inserter(excluded_routes, excluded_routes.end()));

				m_logger(fscp::log_level::warning) << "Received system routes from " << sender << " (version " << version << ") but some did not match the system route acceptance policy (" << m_configuration.router.system_route_acceptance_policy << ", limit " << m_configuration.router.maximum_routes_limit << "): " << excluded_routes;
			}
		}

		client_router_info_type new_client_router_info;
		new_client_router_info.saved_system_route = client_router_info.saved_system_route;
		new_client_router_info.version = client_router_info.version;

		for (auto&& route : filtered_system_routes)
		{
			// Mac OSX doesn't support duplicate default gateways.
#ifdef MACINTOSH
			const auto address = to_ip_address(network_address(route));
			const auto prefix_length = to_prefix_length(network_address(route));

			if ((address == boost::asio::ip::address_v4::any()) && (prefix_length == 0)) {
				m_logger(fscp::log_level::warning) << "Received a default IPv4 route: splitting it as Mac OS X doesn't support duplicate default routes.";

				const auto gateway = asiotap::gateway(route);

				boost::optional<boost::asio::ip::address_v4> ipv4_gateway;

				if (gateway && gateway->is_v4()) {
					ipv4_gateway = gateway->to_v4();
				}

				const auto route1 = asiotap::ipv4_route(asiotap::ipv4_network_address(boost::asio::ip::address_v4::from_string("0.0.0.0"), 1), ipv4_gateway);
				const auto route2 = asiotap::ipv4_route(asiotap::ipv4_network_address(boost::asio::ip::address_v4::from_string("128.0.0.0"), 1), ipv4_gateway);

				new_client_router_info.system_route_entries.push_back(m_route_manager.get_route_entry(m_tap_adapter->get_route(route1)));
				new_client_router_info.system_route_entries.push_back(m_route_manager.get_route_entry(m_tap_adapter->get_route(route2)));
			} else {
				new_client_router_info.system_route_entries.push_back(m_route_manager.get_route_entry(m_tap_adapter->get_route(route)));
			}
#else
			new_client_router_info.system_route_entries.push_back(m_route_manager.get_route_entry(m_tap_adapter->get_route(route)));
#endif
		}
		for (auto&& dns_server : filtered_dns_servers)
		{
			new_client_router_info.dns_servers_entries.push_back(m_dns_servers_manager.get_dns_server_entry(m_tap_adapter->get_dns_server(dns_server)));
		}

		client_router_info = new_client_router_info;
	}

	int core::certificate_validation_callback(int ok, X509_STORE_CTX* ctx)
	{
		cryptoplus::x509::store_context store_context(ctx);

		core* _this = static_cast<core*>(store_context.get_external_data(core::ex_data_index));

		return (_this->certificate_validation_method(ok != 0, store_context)) ? 1 : 0;
	}

	void core::build_ca_store(build_ca_store_when condition)
	{
		// We can't easily ensure m_ca_store is used only in one strand, so we protect it with a mutex instead.
		boost::mutex::scoped_lock lock(m_ca_store_mutex);

		if (!!m_ca_store)
		{
			if (condition == build_ca_store_when::it_doesnt_exist)
			{
				return;
			}

			m_logger(fscp::log_level::information) << "Rebuilding CA store...";
		}
		else
		{
			m_logger(fscp::log_level::information) << "Building CA store...";
		}

		m_ca_store = cryptoplus::x509::store::create();

		for (const cert_type& cert : m_configuration.security.certificate_authority_list)
		{
			m_ca_store.add_certificate(cert);
		}

		for (const cert_type& cert : m_client_certificate_authority_list)
		{
			m_ca_store.add_certificate(cert);
		}

		for (const crl_type& crl : m_configuration.security.certificate_revocation_list_list)
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
	bool core::certificate_validation_method(bool ok, cryptoplus::x509::store_context store_context)
	{
		cert_type cert = store_context.get_current_certificate();

		if (!ok)
		{
			m_logger(fscp::log_level::warning) << "Error when validating " << cert.subject() << ": " << store_context.get_error_string() << " (depth: " << store_context.get_error_depth() << ")";
		}
		else
		{
			m_logger(fscp::log_level::information) << cert.subject() << " is valid.";
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

					// We can't easily ensure m_ca_store is used only in one strand, so we protect it with a mutex instead.
					boost::mutex::scoped_lock lock(m_ca_store_mutex);

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

		if (m_certificate_validation_callback)
		{
			return m_certificate_validation_callback(cert);
		}

		return true;
	}

	void core::open_tap_adapter()
	{
		if (m_configuration.tap_adapter.enabled)
		{
			const asiotap::tap_adapter_layer tap_adapter_type = (m_configuration.tap_adapter.type == tap_adapter_configuration::tap_adapter_type::tap) ? asiotap::tap_adapter_layer::ethernet : asiotap::tap_adapter_layer::ip;

			m_tap_adapter = boost::make_shared<asiotap::tap_adapter>(boost::ref(m_tap_adapter_io_service), tap_adapter_type);

			const auto write_func = [this] (boost::asio::const_buffer data, simple_handler_type handler) {
				async_write_tap(buffer(data), m_io_service.wrap(handler));
			};

			m_tap_adapter->open(m_configuration.tap_adapter.name);

			asiotap::tap_adapter_configuration tap_config;

			// The device MTU.
			tap_config.mtu = compute_mtu(m_configuration.tap_adapter.mtu, get_auto_mtu_value());

			m_logger(fscp::log_level::important) << "Tap adapter \"" << *m_tap_adapter << "\" opened in mode " << m_configuration.tap_adapter.type << " with a MTU set to: " << tap_config.mtu;

			// The MSS override.
			const size_t max_mss = compute_mss(m_configuration.tap_adapter.mss_override, get_auto_mss_value(tap_config.mtu));

			if (max_mss > 0) {
				m_tcp_mss_morpher.reset(new asiotap::osi::tcp_mss_morpher(max_mss));
				m_logger(fscp::log_level::important) << "MSS override enabled with a value of: " << max_mss;
			} else {
				m_tcp_mss_morpher.reset();
				m_logger(fscp::log_level::warning) << "MSS override disabled. You may experience IP fragmentation for encapsulated TCP connections.";
			}

			// IPv4 address
			if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
			{
				m_logger(fscp::log_level::information) << "IPv4 address: " << m_configuration.tap_adapter.ipv4_address_prefix_length;

				asiotap::ipv4_network_address address = { m_configuration.tap_adapter.ipv4_address_prefix_length.address(), m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length() };
				tap_config.ipv4.network_address = address;
			}
			else
			{
				m_logger(fscp::log_level::information) << "No IPv4 address configured.";
			}

			// IPv4 DHCP (for Windows)
			tap_config.ipv4.dhcp = m_configuration.tap_adapter.ipv4_dhcp;

			// IPv6 address
			if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
			{
				m_logger(fscp::log_level::information) << "IPv6 address: " << m_configuration.tap_adapter.ipv6_address_prefix_length;

				asiotap::ipv6_network_address address = { m_configuration.tap_adapter.ipv6_address_prefix_length.address(), m_configuration.tap_adapter.ipv6_address_prefix_length.prefix_length() };
				tap_config.ipv6.network_address = address;
			}
			else
			{
				m_logger(fscp::log_level::information) << "No IPv6 address configured.";
			}

			// If we are running in tun mode, we need at least one
			if (m_configuration.tap_adapter.type == tap_adapter_configuration::tap_adapter_type::tun)
			{
				if (m_configuration.tap_adapter.ipv4_address_prefix_length.is_null() && m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
				{
					throw std::runtime_error("Running in tun mode, but no IPv4 or IPv6 address was provided. Please configure at least one IPv4 or IPv6 address.");
				}

				if (m_configuration.tap_adapter.remote_ipv4_address)
				{
					m_logger(fscp::log_level::information) << "IPv4 remote address: " << m_configuration.tap_adapter.remote_ipv4_address->to_string();

					tap_config.ipv4.remote_address = *m_configuration.tap_adapter.remote_ipv4_address;
				}
				else
				{
					const boost::asio::ip::address_v4 remote_ipv4_address = m_configuration.tap_adapter.ipv4_address_prefix_length.get_network_address();

					m_logger(fscp::log_level::information) << "No IPv4 remote address configured. Using a default of: " << remote_ipv4_address.to_string();

					tap_config.ipv4.remote_address = remote_ipv4_address;
				}
			}

			m_tap_adapter->configure(tap_config);

#ifdef WINDOWS
			const auto metric_value = get_metric_value(m_configuration.tap_adapter.metric);

			if (metric_value)
			{
				m_logger(fscp::log_level::information) << "Setting interface metric to: " << *metric_value;

				m_tap_adapter->set_metric(*metric_value);
			}
#endif

			m_logger(fscp::log_level::information) << "Putting interface into the connected state.";
			m_tap_adapter->set_connected_state(true);

			auto local_routes = translate_ip_routes(m_configuration.router.local_ip_routes);
			auto local_dns_servers = m_configuration.router.local_dns_servers;

			if (tap_adapter_type == asiotap::tap_adapter_layer::ethernet)
			{
				// Registers the switch port.
				m_switch.register_port(make_port_index(m_tap_adapter), switch_::port_type(write_func, TAP_ADAPTERS_GROUP));

				// The ARP proxy
				if (m_configuration.tap_adapter.arp_proxy_enabled)
				{
					m_logger(fscp::log_level::warning) << "The ARP proxy is enabled and this is NOT recommended ! You will face IPv4 connectivity issues !";

					m_arp_proxy.reset(new arp_proxy_type());
					m_arp_proxy->set_arp_request_callback(boost::bind(&core::do_handle_arp_request, this, _1, _2));
				}
				else
				{
					m_arp_proxy.reset();
				}

				// The DHCP proxy
				if (m_configuration.tap_adapter.dhcp_proxy_enabled)
				{
					m_logger(fscp::log_level::information) << "The DHCP proxy is enabled.";

					m_dhcp_proxy.reset(new dhcp_proxy_type());
					m_dhcp_proxy->set_hardware_address(m_tap_adapter->ethernet_address().data());

					if (!m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.is_null())
					{
						m_dhcp_proxy->set_software_address(m_configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length.address());
					}

					if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null())
					{
						m_dhcp_proxy->add_entry(
								m_tap_adapter->ethernet_address().data(),
								m_configuration.tap_adapter.ipv4_address_prefix_length.address(),
								m_configuration.tap_adapter.ipv4_address_prefix_length.prefix_length()
						);
					}
				}
				else
				{
					m_dhcp_proxy.reset();
				}

				// We don't need those proxies in TAP mode.
				m_icmpv6_proxy.reset();
			}
			else
			{
				// Registers the router port.
				m_router.register_port(make_port_index(m_tap_adapter), router::port_type(write_func, TAP_ADAPTERS_GROUP));

				// Add the routes from the TAP adapter.
				const auto tap_ip_addresses = m_tap_adapter->get_ip_addresses();

				for (auto&& ip_address : tap_ip_addresses)
				{
					local_routes.insert(asiotap::to_network_address(asiotap::to_ip_address(ip_address)));
				}

				m_local_routes_version = routes_message::version_type();
				m_router.get_port(make_port_index(m_tap_adapter))->set_local_routes(local_routes);
				m_router.get_port(make_port_index(m_tap_adapter))->set_local_dns_servers(local_dns_servers);

				// Handle ICMPv6 neighbor solicitations. This is required for Windows.
				m_icmpv6_proxy.reset(new icmpv6_proxy_type());
				m_icmpv6_proxy->set_neighbor_solicitation_callback(boost::bind(&core::do_handle_icmpv6_neighbor_solicitation, this, _1, _2));

				// We don't need those proxies in TUN mode.
				m_arp_proxy.reset();
				m_dhcp_proxy.reset();
			}

			if (local_routes.empty())
			{
				m_logger(fscp::log_level::information) << "Not advertising any routes.";
			}
			else
			{
				m_logger(fscp::log_level::information) << "Advertising the following routes: " << local_routes;
			}

			if (local_dns_servers.empty())
			{
				m_logger(fscp::log_level::information) << "Not advertising any DNS servers.";
			}
			else
			{
				m_logger(fscp::log_level::information) << "Advertising the following DNS servers: " << local_dns_servers;
			}

			if (m_tap_adapter_up_callback)
			{
				m_tap_adapter_up_callback(*m_tap_adapter);
			}

			async_read_tap();

			m_tap_adapter_thread = boost::thread([this](){
				m_logger(fscp::log_level::information) << "Starting tap adapter's thread...";
				m_tap_adapter_io_service.run();
				m_logger(fscp::log_level::information) << "Tap adapter's thread is now stopped.";
			});
		}
		else
		{
			m_tap_adapter.reset();
		}
	}

	void core::close_tap_adapter()
	{
		// Clear the endpoint routes, if any.
		m_router_strand.post([this](){
			m_client_router_info_map.clear();
		});

		m_dhcp_proxy.reset();
		m_arp_proxy.reset();
		m_icmpv6_proxy.reset();

		m_tcp_mss_morpher.reset();

		if (m_tap_adapter)
		{
			if (m_tap_adapter_down_callback)
			{
				m_tap_adapter_down_callback(*m_tap_adapter);
			}

			m_router_strand.post([this](){
				m_switch.unregister_port(make_port_index(m_tap_adapter));
				m_router.unregister_port(make_port_index(m_tap_adapter));
			});

			m_tap_adapter->cancel();
			m_tap_adapter->set_connected_state(false);

			m_tap_adapter->close();
			m_tap_adapter_io_service.stop();

			m_tap_adapter_thread.join();
		}
	}

	void core::async_get_tap_addresses(ip_network_address_list_handler_type handler)
	{
		if (m_tap_adapter)
		{
			m_tap_adapter_io_service.post([this, handler](){
			handler(m_tap_adapter->get_ip_addresses());
			});
		}
		else
		{
			handler(asiotap::ip_network_address_list {});
		}
	}

	void core::async_read_tap()
	{
		m_tap_adapter_io_service.post(boost::bind(&core::do_read_tap, this));
	}

	template <typename ConstBufferSequence>
	void core::push_tap_write(const ConstBufferSequence& data, simple_handler_type handler)
	{
		// All push_tap_write() calls are done in the m_tap_adapter_io_service so the following is thread-safe.
		const auto write_call = [this, data, handler] () {
			m_tap_adapter->async_write(data, [this, handler] (const boost::system::error_code& ec, size_t) {
				pop_tap_write();

				handler(ec);
			});
		};

		if (m_tap_write_queue.empty())
		{
			// Nothing is being written, lets start the write immediately.
			write_call();
		}

		// We need to push it always, even if it was called immediately as it
		// will be popped-out when the write ends and also serves as a marker
		// that a write is in progress.
		m_tap_write_queue.push(write_call);
	}

	void core::pop_tap_write()
	{
		// All pop_tap_write() calls are done in the m_tap_adapter_io_service so the following is thread-safe.
		m_tap_write_queue.pop();

		if (!m_tap_write_queue.empty())
		{
			m_tap_write_queue.front()();
		}
	}

	void core::do_read_tap()
	{
		// All calls to do_read_tap() are done within the m_tap_adapter_io_service, so the following is safe.
		assert(m_tap_adapter);

		// Get either a new buffer or an old, recycled one if possible.
		const SharedBuffer receive_buffer = m_tap_adapter_buffers.empty() ? SharedBuffer(65536) : [this] () {
			const auto result = m_tap_adapter_buffers.front();
			m_tap_adapter_buffers.pop_front();

			return result;
		}();

		// disable buffer recycling from now (possible issue with SharedBuffer
		// which references itself due to lambda capture)
		m_tap_adapter->async_read(
			buffer(receive_buffer),
			boost::bind(
				&core::do_handle_tap_adapter_read,
				this,
				/*
				SharedBuffer(receive_buffer, [this](const SharedBuffer& buffer) {
					m_tap_adapter_io_service.post([this, buffer] () {
						m_tap_adapter_buffers.push_back(buffer);
					});
				}),
				*/
				receive_buffer,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	void core::do_handle_tap_adapter_read(SharedBuffer receive_buffer, const boost::system::error_code& ec, size_t count)
	{
		// All calls to do_handle_tap_adapter_read() are done within the m_tap_adapter_io_service, so the following is safe.
		if (ec != boost::asio::error::operation_aborted)
		{
			// We try to read again, as soon as possible.
			do_read_tap();
		}

		if (!ec)
		{
			const boost::asio::mutable_buffer data = buffer(receive_buffer, count);

#ifdef FREELAN_DEBUG
			std::cerr << "Read " << buffer_size(data) << " byte(s) on " << *m_tap_adapter << std::endl;
#endif

			bool handled = false;

			if (m_tap_adapter->layer() == asiotap::tap_adapter_layer::ethernet)
			{
				// This line will eventually call the filters callbacks and the mss morpher.
				m_ethernet_filter.parse(data);

				if (m_arp_proxy || m_dhcp_proxy)
				{
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
					async_write_switch(
						make_port_index(m_tap_adapter),
						data,
						make_shared_buffer_handler(
							receive_buffer,
							&null_switch_write_handler
						)
					);
				}
			}
			else
			{
				// This line will eventually call the filters callbacks and the mss override.
				m_tun_ipv6_filter.parse(data);

				if (m_icmpv6_proxy)
				{
					if (m_tun_icmpv6_filter.get_last_helper())
					{
						// We don't want to catch ICMP echo requests or other stuff yet.
						handled = m_tun_icmpv6_filter.get_last_helper()->type() == asiotap::osi::ICMPV6_NEIGHBOR_SOLICITATION;
						m_tun_icmpv6_filter.clear_last_helper();
					}
				}

				if (!handled)
				{
					// This is a TUN interface. We receive either IPv4 or IPv6 frames.
					async_write_router(
						make_port_index(m_tap_adapter),
						data,
						make_shared_buffer_handler(
							receive_buffer,
							&null_router_write_handler
						)
					);
				}
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			m_logger(fscp::log_level::error) << "Read failed on " << m_tap_adapter->name() << ". Error: " << ec.message();
		}
	}

	void core::do_handle_tap_adapter_write(const boost::system::error_code& ec)
	{
		if (ec)
		{
			if (ec != boost::asio::error::operation_aborted)
			{
				m_logger(fscp::log_level::warning) << "Write failed on " << m_tap_adapter->name() << ". Error: " << ec.message();
			}
		}
	}

	void core::do_handle_arp_frame(const arp_helper_type& helper)
	{
		if (m_arp_proxy)
		{
			const auto response_buffer = SharedBuffer(2048);
			const boost::optional<boost::asio::const_buffer> data = m_arp_proxy->process_frame(
				*m_arp_filter.parent().get_last_helper(),
				helper,
				buffer(response_buffer)
			);

			if (data)
			{
				async_write_tap(
					buffer(*data),
					make_shared_buffer_handler(
						response_buffer,
						boost::bind(
							&core::do_handle_tap_adapter_write,
							this,
							boost::asio::placeholders::error
						)
					)
				);
			}
		}
	}

	void core::do_handle_dhcp_frame(const dhcp_helper_type& helper)
	{
		if (m_dhcp_proxy)
		{
			const auto response_buffer = SharedBuffer(2048);
			const boost::optional<boost::asio::const_buffer> data = m_dhcp_proxy->process_frame(
				*m_dhcp_filter.parent().parent().parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().parent().get_last_helper(),
				*m_dhcp_filter.parent().get_last_helper(),
				helper,
				buffer(response_buffer)
			);

			if (data)
			{
				async_write_tap(
					buffer(*data),
					make_shared_buffer_handler(
						response_buffer,
						boost::bind(
							&core::do_handle_tap_adapter_write,
							this,
							boost::asio::placeholders::error
						)
					)
				);
			}
		}
	}

	void core::do_handle_icmpv6_frame(const icmpv6_helper_type& helper)
	{
		if (m_icmpv6_proxy)
		{
			const auto response_buffer = SharedBuffer(2048);
			const boost::optional<boost::asio::const_buffer> data = m_icmpv6_proxy->process_frame(
				*m_tun_icmpv6_filter.parent().get_last_helper(),
				helper,
				buffer(response_buffer)
			);

			if (data)
			{
				async_write_tap(
					buffer(*data),
					make_shared_buffer_handler(
					response_buffer,
					boost::bind(
						&core::do_handle_tap_adapter_write,
						this,
						boost::asio::placeholders::error
					)
					)
				);
			}
		}
	}

	bool core::do_handle_arp_request(const boost::asio::ip::address_v4& logical_address, ethernet_address_type& ethernet_address)
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

	bool core::do_handle_icmpv6_neighbor_solicitation(const boost::asio::ip::address_v6& logical_address, ethernet_address_type& ethernet_address)
	{
		if (!m_configuration.tap_adapter.ipv6_address_prefix_length.is_null())
		{
			if (logical_address != m_configuration.tap_adapter.ipv6_address_prefix_length.address())
			{
				ethernet_address = m_configuration.tap_adapter.arp_proxy_fake_ethernet_address;

				return true;
			}
		}

		return false;
	}

	void core::do_register_switch_port(const ep_type& host, void_handler_type handler)
	{
		// All calls to do_register_switch_port() are done within the m_router_strand, so the following is safe.
		m_switch.register_port(make_port_index(host), switch_::port_type(boost::bind(&fscp::server::async_send_data, m_fscp_server, host, fscp::CHANNEL_NUMBER_0, _1, _2), ENDPOINTS_GROUP));

		if (handler)
		{
			handler();
		}
	}

	void core::do_unregister_switch_port(const ep_type& host, void_handler_type handler)
	{
		// All calls to do_unregister_switch_port() are done within the m_router_strand, so the following is safe.
		m_switch.unregister_port(make_port_index(host));

		if (handler)
		{
			handler();
		}
	}

	void core::do_register_router_port(const ep_type& host, void_handler_type handler)
	{
		// All calls to do_register_router_port() are done within the m_router_strand, so the following is safe.
		m_router.register_port(make_port_index(host), router::port_type(boost::bind(&fscp::server::async_send_data, m_fscp_server, host, fscp::CHANNEL_NUMBER_0, _1, _2), ENDPOINTS_GROUP));

		if (handler)
		{
			handler();
		}
	}

	void core::do_unregister_router_port(const ep_type& host, void_handler_type handler)
	{
		// All calls to do_unregister_router_port() are done within the m_router_strand, so the following is safe.
		m_router.unregister_port(make_port_index(host));

		if (handler)
		{
			handler();
		}
	}

	void core::do_save_system_route(const ep_type& host, const route_type& route, void_handler_type handler)
	{
		// All calls to do_save_system_route() are done within the m_router_strand, so the following is safe.
		client_router_info_type& client_router_info = m_client_router_info_map[host];
		client_router_info.saved_system_route = m_route_manager.get_route_entry(route);

		if (handler)
		{
			handler();
		}
	}

	void core::do_clear_client_router_info(const ep_type& host, void_handler_type handler)
	{
		// All calls to do_clear_client_router_info() are done within the m_router_strand, so the following is safe.

		// This clears the routes, if any.
		m_client_router_info_map.erase(host);

		if (handler)
		{
			handler();
		}
	}

	void core::do_write_switch(const port_index_type& index, boost::asio::const_buffer data, switch_::multi_write_handler_type handler)
	{
		// All calls to do_write_switch() are done within the m_router_strand, so the following is safe.
		m_switch.async_write(index, data, handler);
	}

	void core::do_write_router(const port_index_type& index, boost::asio::const_buffer data, router::port_type::write_handler_type handler)
	{
		// All calls to do_write_router() are done within the m_router_strand, so the following is safe.
		m_router.async_write(index, data, handler);
	}

	void core::open_web_server()
	{
		if (m_configuration.server.enabled)
		{
#ifndef USE_MONGOOSE
			// Due to GPL licensing issues, we can't include that in the build without violating the GPLv3 license.
			// If you still want that support, adds USE_MONGOOSE define to the build options.
			m_logger(fscp::log_level::warning) << "Web server support is not compiled in this version.";
#else
			if (m_configuration.server.protocol == server_configuration::server_protocol_type::https)
			{
				bool generated = false;

				if (!m_configuration.server.server_private_key)
				{
					m_logger(fscp::log_level::warning) << "No private key set for the web server. Generating a temporary one...";

					m_configuration.server.server_private_key = generate_private_key();
					generated = true;
				}

				if (!m_configuration.server.server_certificate)
				{
					m_logger(fscp::log_level::warning) << "No certificate set for the web server. Generating a temporary one...";

					m_configuration.server.server_certificate = generate_self_signed_certificate(m_configuration.server.server_private_key);
					generated = true;
				}

				if (generated)
				{
					m_logger(fscp::log_level::warning) << "Using a dynamically generated certificate/private key for the web server will force web clients to disable peer verification. Is this what you really want ?";
				}
			}

			// CA certificate check.
			{
				bool generated = false;

				if (!m_configuration.server.certification_authority_private_key)
				{
					m_logger(fscp::log_level::warning) << "No private key set for the web server's CA. Generating a temporary one...";

					m_configuration.server.certification_authority_private_key = generate_private_key();
					generated = true;
				}

				if (!m_configuration.server.certification_authority_certificate)
				{
					m_logger(fscp::log_level::warning) << "No certificate set for the web server's CA. Generating a temporary one...";

					m_configuration.server.certification_authority_certificate = generate_self_signed_certificate(m_configuration.server.certification_authority_private_key);
					generated = true;
				}

				if (generated)
				{
					m_logger(fscp::log_level::warning) << "Using a dynamically generated certificate/private key for the web server's CA will cause the session lifecycle to be tied to the one of the server.";
				}
			}

			m_web_server = boost::make_shared<web_server>(m_logger, m_configuration.server, m_authentication_callback);

			m_logger(fscp::log_level::information) << "Starting " << m_configuration.server.protocol << " web server on " << m_configuration.server.listen_on << "...";

			m_web_server_thread = boost::thread([this](){ m_web_server->run(); });

			m_logger(fscp::log_level::information) << "Web server started.";
#endif
		}
	}

	void core::close_web_server()
	{
#ifdef USE_MONGOOSE
		if (m_web_server)
		{
			m_logger(fscp::log_level::information) << "Closing web server...";

			m_web_server->stop();
			m_web_server_thread.join();
			m_web_server.reset();

			m_logger(fscp::log_level::information) << "Web server closed.";
		}
#endif
	}

	void core::open_web_client()
	{
		if (m_configuration.client.enabled)
		{
			m_logger(fscp::log_level::information) << "Starting web client to contact web server at " << m_configuration.client.protocol << "://" << m_configuration.client.server_endpoint << "...";

			m_web_client = web_client::create(m_io_service, m_logger, m_configuration.client);

			m_logger(fscp::log_level::information) << "Web client started.";

			if (!m_configuration.security.identity)
			{
				m_logger(fscp::log_level::information) << "No user or private key set. Requesting one from web server...";

				request_certificate();
			}
			else
			{
				m_logger(fscp::log_level::information) << "Registering onto the server...";

				register_();
			}

			request_ca_certificate();
		}
	}

	void core::close_web_client()
	{
		if (m_web_client)
		{
			m_logger(fscp::log_level::information) << "Closing web client...";

			unregister();
			m_request_certificate.timer.cancel();
			m_request_ca_certificate.timer.cancel();
			m_renew_certificate_timer.cancel();
			m_registration_retry.timer.cancel();
			m_set_contact_information_retry.timer.cancel();
			m_get_contact_information_retry.timer.cancel();
			m_web_client.reset();

			m_logger(fscp::log_level::information) << "Web client closed.";
		}
	}

	void core::request_certificate()
	{
		if (m_web_client) {
			m_logger(fscp::log_level::information) << "Requesting a client certificate from the web server...";

			const auto private_key = generate_private_key();
			const auto certificate_request = generate_certificate_request(private_key);

			m_web_client->request_certificate(certificate_request, [this, private_key](const boost::system::error_code& ec, cryptoplus::x509::certificate certificate) {
				if (ec)
				{
					if (m_web_client) {
						m_logger(fscp::log_level::error) << "The certificate request to the web server failed: " << ec.message() << " (" << ec << "). Retrying in " << m_request_certificate.period << "...";

						m_request_certificate.exponential_backoff();
						m_request_certificate.timer.async_wait([this](const boost::system::error_code& ec2) {
							if (ec2 != boost::asio::error::operation_aborted)
							{
								request_certificate();
							}
						});
					}
					else {
						m_logger(fscp::log_level::error) << "The certificate request to the web server failed: " << ec.message() << " (" << ec << "). Not retrying as the web client was shut down.";
					}
				}
				else
				{
					m_request_certificate.reset();
					m_logger(fscp::log_level::information) << "Received certificate from server: " << certificate.subject();

					m_configuration.security.identity = fscp::identity_store(certificate, private_key);

					if (m_fscp_server)
					{
						m_fscp_server->async_set_identity(*m_configuration.security.identity, [this]() {
							m_logger(fscp::log_level::important) << "Renewed identity. Existing connections will be reset.";
						});
					}
					else
					{
						open_fscp_server();
					}

					if(!m_web_client)
					{
						m_logger(fscp::log_level::information) << "FSCP closing as the web client was shut down.";
						// closing core when FSCP was not setup so close it now!
						close_fscp_server();
						return;
					}

					const auto renew_timestamp = certificate.not_after().to_ptime() - RENEW_CERTIFICATE_WARNING_PERIOD;

					m_logger(fscp::log_level::information) << "Certificate expires on " << local_adjustor::utc_to_local(certificate.not_after().to_ptime()) << ". Renewing on " << local_adjustor::utc_to_local(renew_timestamp) << ".";

					m_renew_certificate_timer.expires_at(renew_timestamp);
					m_renew_certificate_timer.async_wait([this](const boost::system::error_code& ec2) {
						if (ec2 != boost::asio::error::operation_aborted)
						{
							request_certificate();
						}
					});

					m_logger(fscp::log_level::information) << "Registering to the server...";
					register_();
				}
			});
		} else {
			m_logger(fscp::log_level::information) << "Not requesting a client certificate from the web server as the web client was shut down.";
		}
	}

	void core::request_ca_certificate()
	{
		if (m_web_client) {
			m_logger(fscp::log_level::information) << "Requesting the CA certificate from the web server...";

			m_web_client->request_ca_certificate([this](const boost::system::error_code& ec, cryptoplus::x509::certificate certificate) {
				if (ec)
				{
					if (m_web_client) {
						m_logger(fscp::log_level::error) << "The CA certificate request to the web server failed: " << ec.message() << " (" << ec << "). Retrying in " << m_request_ca_certificate.period << "...";

						m_request_ca_certificate.exponential_backoff();
						m_request_ca_certificate.timer.async_wait([this](const boost::system::error_code& ec2) {
							if (ec2 != boost::asio::error::operation_aborted)
							{
								request_ca_certificate();
							}
						});
					}
					else {
						m_logger(fscp::log_level::error) << "The CA certificate request to the web server failed: " << ec.message() << " (" << ec << "). Not retrying as the web client was shut down.";
					}
				}
				else
				{
					m_request_ca_certificate.reset();
					m_logger(fscp::log_level::information) << "Received CA certificate from server: " << certificate.subject();

					m_client_certificate_authority_list.clear();
					m_client_certificate_authority_list.push_back(certificate);

					build_ca_store(build_ca_store_when::always);
				}
			});
		} else {
			m_logger(fscp::log_level::information) << "Not requesting the CA certificate from the web server as the web client was shut down.";
		}
	}

	void core::register_()
	{
		if (m_web_client) {
			if (!m_configuration.security.identity)
			{
				if (m_web_client) {
					m_logger(fscp::log_level::warning) << "Cannot register onto the web server right now as no identity is currently set. Retrying in " << m_registration_retry.period << "...";
					m_registration_retry.exponential_backoff();
					m_registration_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
						if (ec2 != boost::asio::error::operation_aborted)
						{
							register_();
						}
					});
				} else {
					m_logger(fscp::log_level::warning) << "Cannot register onto the web server right now as no identity is currently set. Won't retry as the web client was shut down.";
				}
			}
			else
			{
				m_logger(fscp::log_level::information) << "Registering at the web server...";

				m_web_client->register_(m_configuration.security.identity->signature_certificate(), [this](const boost::system::error_code& ec, const boost::posix_time::ptime& expiration_timestamp) {
					if (ec)
					{
						if (m_web_client) {
							m_logger(fscp::log_level::error) << "The registration onto the web server failed: " << ec.message() << " (" << ec << "). Retrying in " << m_registration_retry.period << "...";
							m_registration_retry.exponential_backoff();
							m_registration_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
								if (ec2 != boost::asio::error::operation_aborted)
								{
									register_();
								}
							});
						}
						else {
							m_logger(fscp::log_level::error) << "The registration onto the web server failed: " << ec.message() << " (" << ec << "). Not retrying as the web client was shut down.";
						}
					}
					else
					{
						const auto local_expiration_timestamp = local_adjustor::utc_to_local(expiration_timestamp);

						if (m_web_client) {
							m_registration_retry.reset();

							const auto registration_update_timestamp = expiration_timestamp - REGISTRATION_WARNING_PERIOD;
							const auto local_registration_update_timestamp = local_adjustor::utc_to_local(registration_update_timestamp);

							m_logger(fscp::log_level::information) << "Registered onto the web server until " << local_expiration_timestamp << ". Registration update planned at " << local_registration_update_timestamp << ".";
							m_registration_retry.timer.expires_at(registration_update_timestamp);
							m_registration_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
								if (ec2 != boost::asio::error::operation_aborted)
								{
									register_();
								}
							});

							set_contact_information();
							get_contact_information();
						}
						else {
							m_logger(fscp::log_level::information) << "Registered onto the web server until " << local_expiration_timestamp << ". However, no registration update was planned as the web client was shut down already.";
						}
					}
				});
			}
		} else {
			m_logger(fscp::log_level::information) << "Not registering at the web server as the web client was shut down.";
		}
	}

	void core::unregister()
	{
		if (m_web_client) {
			m_logger(fscp::log_level::information) << "Unregistering from the web server...";

			m_web_client->unregister([this](const boost::system::error_code& ec) {
				if (ec)
				{
					m_logger(fscp::log_level::error) << "The unregistration from the web server failed: " << ec.message() << " (" << ec << "). Not retrying to avoid delaying shutdown.";
				}
				else
				{
					m_logger(fscp::log_level::information) << "Unregistered from the web server.";
				}
			});
		} else {
			m_logger(fscp::log_level::information) << "Not unregistering from the web server as the web client was shut down.";
		}
	}

	void core::set_contact_information()
	{
		if (m_web_client) {
			m_logger(fscp::log_level::information) << "Setting contact information on the web server...";

			if (!m_fscp_server)
			{
				m_logger(fscp::log_level::warning) << "Cannot set contact information right now as the FSCP server is not started yet.";

				m_set_contact_information_retry.exponential_backoff();
				m_set_contact_information_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
					if (ec2 != boost::asio::error::operation_aborted)
					{
						set_contact_information();
					}
				});
			}
			else
			{
				const auto local_port = m_fscp_server->get_socket().local_endpoint().port();
				std::set<asiotap::endpoint> public_endpoints;

				for (auto&& public_endpoint : m_configuration.client.public_endpoint_list)
				{
					public_endpoints.insert(asiotap::get_default_port_endpoint(public_endpoint, local_port));
				}

				if (public_endpoints.empty())
				{
					m_logger(fscp::log_level::information) << "Setting contact information on the web server with no public endpoints...";
				}
				else
				{
					std::ostringstream oss;

					for (auto&& ep : public_endpoints)
					{
						if (!oss.str().empty())
						{
							oss << ", ";
						}

						oss << ep;
					}

					m_logger(fscp::log_level::information) << "Setting contact information on the web server with " << public_endpoints.size() << " public endpoint(s) (" << oss.str() << ")...";
				}

				m_web_client->set_contact_information(public_endpoints, [this](const boost::system::error_code& ec, const std::set<asiotap::endpoint>& accepted_endpoints, const std::set<asiotap::endpoint>& rejected_endpoints) {
					if (ec)
					{
						m_logger(fscp::log_level::error) << "Failed to set contact information on the web server: " << ec.message() << " (" << ec << ").";

						m_set_contact_information_retry.exponential_backoff();
						m_set_contact_information_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
							if (ec2 != boost::asio::error::operation_aborted)
							{
								set_contact_information();
							}
						});
					}
					else
					{
						m_set_contact_information_retry.reset();
						m_logger(fscp::log_level::information) << "The web server acknowledged our contact information.";

						if (accepted_endpoints.empty())
						{
							m_logger(fscp::log_level::information) << "No public endpoints will be advertised.";
						}
						else
						{
							std::ostringstream oss;

							for (auto&& ep : accepted_endpoints)
							{
								if (!oss.str().empty())
								{
									oss << ", ";
								}

								oss << ep;
							}

							m_logger(fscp::log_level::information) << "Server will advertise the following endpoints: " << oss.str();
						}

						if (!rejected_endpoints.empty())
						{
							std::ostringstream oss;

							for (auto&& ep : rejected_endpoints)
							{
								if (!oss.str().empty())
								{
									oss << ", ";
								}

								oss << ep;
							}

							m_logger(fscp::log_level::warning) << "Server refused to advertise the following endpoints: " << oss.str();
						}
					}
				});
			}
		} else {
			m_logger(fscp::log_level::information) << "Not setting contact information on the web server as the web client was shut down.";
		}
	}

	void core::get_contact_information()
	{
		if (m_web_client) {
			if (!m_fscp_server)
			{
				m_logger(fscp::log_level::warning) << "Cannot get contact information right now as the FSCP server is not started yet.";

				m_get_contact_information_retry.exponential_backoff();
				m_get_contact_information_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
					if (ec2 != boost::asio::error::operation_aborted)
					{
						get_contact_information();
					}
				});
			}
			else
			{
				m_logger(fscp::log_level::information) << "Getting contact information from the web server...";

				// The requested contacts list is empty, meaning we want them all.
				std::set<fscp::hash_type> requested_contacts;

				m_web_client->get_contact_information(requested_contacts, [this](const boost::system::error_code& ec, const std::map<fscp::hash_type, std::set<asiotap::endpoint>>& contacts) {
					if (ec)
					{
						m_logger(fscp::log_level::error) << "Failed to get contact information from the web server: " << ec.message() << " (" << ec << ").";

						m_get_contact_information_retry.exponential_backoff();
						m_get_contact_information_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
							if (ec2 != boost::asio::error::operation_aborted)
							{
								get_contact_information();
							}
						});
					}
					else
					{
						// We got contact information lets trigger an update later.
						m_get_contact_information_retry.reset();
						m_get_contact_information_retry.timer.expires_from_now(GET_CONTACT_INFORMATION_UPDATE_PERIOD);
						m_get_contact_information_retry.timer.async_wait([this](const boost::system::error_code& ec2) {
							if (ec2 != boost::asio::error::operation_aborted)
							{
								get_contact_information();
							}
						});

						m_logger(fscp::log_level::information) << "The web server replied to our contact information request.";

						if (contacts.empty())
						{
							m_logger(fscp::log_level::information) << "No contacts were provided.";
						}
						else
						{
							for (auto&& contact : contacts)
							{
								std::ostringstream oss;

								for (auto&& ep : contact.second)
								{
									async_contact(ep);

									if (!oss.str().empty())
									{
										oss << ", ";
									}

									oss << ep;
								}

								m_logger(fscp::log_level::information) << "Contact information for " << contact.first << ": " << oss.str();
							}
						}
					}
				});
			}
		} else {
			m_logger(fscp::log_level::information) << "Not getting contact information from the web server as the web client was shut down.";
		}
	}

	void freelan::core::timer_period::exponential_backoff() {
		timer.expires_from_now(period);
		exponential_backoff_value(period, min, max);
	}

	asiotap::ip_route_set core::translate_ip_routes(const std::set<ip_route>& routes) const
	{
		boost::optional<boost::asio::ip::address_v4> ipv4_gateway;
		boost::optional<boost::asio::ip::address_v6> ipv6_gateway;

		if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null()) {
			ipv4_gateway = m_configuration.tap_adapter.ipv4_address_prefix_length.address();
		}
		if (!m_configuration.tap_adapter.ipv4_address_prefix_length.is_null()) {
			ipv6_gateway = m_configuration.tap_adapter.ipv6_address_prefix_length.address();
		}

		const auto ipv4_default_route = asiotap::ipv4_route(asiotap::ipv4_network_address::any(), ipv4_gateway);
		const auto ipv6_default_route = asiotap::ipv6_route(asiotap::ipv6_network_address::any(), ipv6_gateway);

		return to_ip_routes(routes, ipv4_default_route, ipv6_default_route);
	}
}

/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file configuration_helper.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A configuration helper.
 */

#include "configuration_helper.hpp"

#include <vector>

#include <boost/asio.hpp>
#include <boost/foreach.hpp>

#include "configuration_types.hpp"
#include "version.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace fl = freelan;

namespace
{
	cryptoplus::file load_file(const fs::path& filename)
	{
		if (filename.empty())
		{
			throw std::runtime_error("Cannot load file: filename is empty");
		}

		if (!is_regular_file(filename))
		{
			throw std::runtime_error("No such file: " + filename.string());
		}

		return cryptoplus::file::open(filename.native());
	}

	fl::security_configuration::cert_type load_certificate(const fs::path& filename)
	{
		return fl::security_configuration::cert_type::from_certificate(load_file(filename));
	}

	cryptoplus::pkey::pkey load_private_key(const fs::path& filename)
	{
		return cryptoplus::pkey::pkey::from_private_key(load_file(filename));
	}

	fl::security_configuration::cert_type load_trusted_certificate(const fs::path& filename)
	{
		return fl::security_configuration::cert_type::from_trusted_certificate(load_file(filename));
	}

	fl::security_configuration::crl_type load_crl(const fs::path& filename)
	{
		return fl::security_configuration::crl_type::from_certificate_revocation_list(load_file(filename));
	}
}

po::options_description get_server_options()
{
	po::options_description result("FreeLAN Server options");

	result.add_options()
	("server.enabled", po::value<bool>()->default_value(false, "no"), "Whether to enable the server mechanism.")
	("server.host", po::value<asiotap::endpoint>(), "The server host.")
	("server.https_proxy", po::value<asiotap::endpoint>(), "The HTTP proxy host.")
	("server.username", po::value<std::string>(), "The username.")
	("server.password", po::value<std::string>(), "The password. If no password is specified, it will be taken from the FREELAN_SERVER_PASSWORD environment variable.")
	("server.network", po::value<std::string>(), "The network. If no network is specified, it will be taken from the FREELAN_SERVER_NETWORK environment variable.")
	("server.public_endpoint", po::value<std::vector<asiotap::endpoint> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::endpoint>(), ""), "A public endpoint to publish to others hosts.")
	("server.user_agent", po::value<std::string>(), "The user agent. If no user agent is specified, \"" FREELAN_USER_AGENT "\" will be used.")
	("server.protocol", po::value<fl::server_configuration::server_protocol_type>()->default_value(fl::server_configuration::SP_HTTPS), "The protocol to use to contact the server.")
	("server.ca_info_file", po::value<fs::path>()->default_value(""), "The CA info file.")
	("server.disable_peer_verification", po::value<bool>()->default_value(false, "no"), "Whether to disable peer verification.")
	("server.disable_host_verification", po::value<bool>()->default_value(false, "no"), "Whether to disable host verification.")
	;

	return result;
}

po::options_description get_fscp_options()
{
	po::options_description result("FreeLAN Secure Channel Protocol (FSCP) options");

	result.add_options()
	("fscp.hostname_resolution_protocol", po::value<fl::fscp_configuration::hostname_resolution_protocol_type>()->default_value(fl::fscp_configuration::HRP_IPV4), "The hostname resolution protocol to use.")
	("fscp.listen_on", po::value<asiotap::endpoint>()->default_value(asiotap::ipv4_endpoint(boost::asio::ip::address_v4::any(), 12000)), "The endpoint to listen on.")
	("fscp.hello_timeout", po::value<millisecond_duration>()->default_value(3000), "The default timeout for HELLO messages, in milliseconds.")
	("fscp.contact", po::value<std::vector<asiotap::endpoint> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::endpoint>(), ""), "The address of an host to contact.")
	("fscp.accept_contact_requests", po::value<bool>()->default_value(true, "yes"), "Whether to accept CONTACT-REQUEST messages.")
	("fscp.accept_contacts", po::value<bool>()->default_value(true, "yes"), "Whether to accept CONTACT messages.")
	("fscp.dynamic_contact_file", po::value<std::vector<std::string> >()->multitoken()->zero_tokens()->default_value(std::vector<std::string>(), ""), "The certificate of an host to dynamically contact.")
	("fscp.never_contact", po::value<std::vector<asiotap::ip_network_address> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::ip_network_address>(), ""), "A network address to avoid when dynamically contacting hosts.")
	("fscp.cipher_suite_capability", po::value<std::vector<fscp::cipher_suite_type> >()->multitoken()->zero_tokens()->default_value(fscp::get_default_cipher_suites(), ""), "A cipher suite to allow.")
	;

	return result;
}

po::options_description get_security_options()
{
	po::options_description result("Security options");

	result.add_options()
	("security.signature_certificate_file", po::value<fs::path>(), "The certificate file to use for signing.")
	("security.signature_private_key_file", po::value<fs::path>(), "The private key file to use for signing.")
	("security.certificate_validation_method", po::value<fl::security_configuration::certificate_validation_method_type>()->default_value(fl::security_configuration::CVM_DEFAULT), "The certificate validation method.")
	("security.certificate_validation_script", po::value<fs::path>()->default_value(""), "The certificate validation script to use.")
	("security.authority_certificate_file", po::value<std::vector<std::string> >()->multitoken()->zero_tokens()->default_value(std::vector<std::string>(), ""), "An authority certificate file to use.")
	("security.certificate_revocation_validation_method", po::value<fl::security_configuration::certificate_revocation_validation_method_type>()->default_value(fl::security_configuration::CRVM_NONE), "The certificate revocation validation method.")
	("security.certificate_revocation_list_file", po::value<std::vector<std::string> >()->multitoken()->zero_tokens()->default_value(std::vector<std::string>(), ""), "A certificate revocation list file to use.")
	;

	return result;
}

po::options_description get_tap_adapter_options()
{
	po::options_description result("Tap adapter options");

	const asiotap::ipv4_network_address default_ipv4_network_address(boost::asio::ip::address_v4::from_string("9.0.0.1"), 24);
	const asiotap::ipv6_network_address default_ipv6_network_address(boost::asio::ip::address_v6::from_string("fe80::1"), 10);
	const asiotap::ipv4_network_address default_dhcp_ipv4_network_address(boost::asio::ip::address_v4::from_string("9.0.0.0"), 24);
	const asiotap::ipv6_network_address default_dhcp_ipv6_network_address(boost::asio::ip::address_v6::from_string("fe80::"), 10);

	result.add_options()
	("tap_adapter.type", po::value<fl::tap_adapter_configuration::tap_adapter_type>()->default_value(fl::tap_adapter_configuration::tap_adapter_type::tap, "tap"), "The TAP adapter type.")
	("tap_adapter.enabled", po::value<bool>()->default_value(true, "yes"), "Whether to enable the tap adapter.")
	("tap_adapter.name", po::value<std::string>(), "The name of the tap adapter to use or create.")
	("tap_adapter.mtu", po::value<fl::mtu_type>()->default_value(fl::auto_mtu_type()), "The MTU of the tap adapter.")
	("tap_adapter.ipv4_address_prefix_length", po::value<asiotap::ipv4_network_address>()->default_value(default_ipv4_network_address), "The tap adapter IPv4 address and prefix length.")
	("tap_adapter.ipv6_address_prefix_length", po::value<asiotap::ipv6_network_address>()->default_value(default_ipv6_network_address), "The tap adapter IPv6 address and prefix length.")
	("tap_adapter.remote_ipv4_address", po::value<asiotap::ipv4_network_address>(), "The tap adapter IPv4 remote address.")
	("tap_adapter.arp_proxy_enabled", po::value<bool>()->default_value(false), "Whether to enable the ARP proxy.")
	("tap_adapter.arp_proxy_fake_ethernet_address", po::value<fl::tap_adapter_configuration::ethernet_address_type>()->default_value(boost::lexical_cast<fl::tap_adapter_configuration::ethernet_address_type>("00:aa:bb:cc:dd:ee")), "The ARP proxy fake ethernet address.")
	("tap_adapter.dhcp_proxy_enabled", po::value<bool>()->default_value(true), "Whether to enable the DHCP proxy.")
	("tap_adapter.dhcp_server_ipv4_address_prefix_length", po::value<asiotap::ipv4_network_address>()->default_value(default_dhcp_ipv4_network_address), "The DHCP proxy server IPv4 address and prefix length.")
	("tap_adapter.dhcp_server_ipv6_address_prefix_length", po::value<asiotap::ipv6_network_address>()->default_value(default_dhcp_ipv6_network_address), "The DHCP proxy server IPv6 address and prefix length.")
	("tap_adapter.up_script", po::value<fs::path>()->default_value(""), "The tap adapter up script.")
	("tap_adapter.down_script", po::value<fs::path>()->default_value(""), "The tap adapter down script.")
	;

	return result;
}

po::options_description get_switch_options()
{
	po::options_description result("Switch options");

	result.add_options()
	("switch.routing_method", po::value<fl::switch_configuration::routing_method_type>()->default_value(fl::switch_configuration::RM_SWITCH), "The routing method for messages.")
	("switch.relay_mode_enabled", po::value<bool>()->default_value(false, "no"), "Whether to enable the relay mode.")
	;

	return result;
}

po::options_description get_router_options()
{
	po::options_description result("Router options");

	result.add_options()
	("router.local_ip_route", po::value<std::vector<asiotap::ip_network_address> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::ip_network_address>(), ""), "A route to advertise to the other peers.")
	("router.client_routing_enabled", po::value<bool>()->default_value(true, "yes"), "Whether to enable client routing.")
	("router.accept_routes_requests", po::value<bool>()->default_value(true, "yes"), "Whether to accept routes requests.")
	("router.internal_route_acceptance_policy", po::value<fl::router_configuration::internal_route_scope_type>()->default_value(fl::router_configuration::internal_route_scope_type::unicast_in_network), "The internal route acceptance policy.")
	("router.system_route_acceptance_policy", po::value<fl::router_configuration::system_route_scope_type>()->default_value(fl::router_configuration::system_route_scope_type::none), "The system route acceptance policy.")
	("router.maximum_routes_limit", po::value<unsigned int>()->default_value(1), "The maximum count of routes to accept for a given host.")
	;

	return result;
}

void setup_configuration(fl::configuration& configuration, const boost::filesystem::path& root, const po::variables_map& vm)
{
	typedef fl::security_configuration::cert_type cert_type;
	typedef cryptoplus::pkey::pkey pkey;

	// Server options
	configuration.server.enabled = vm["server.enabled"].as<bool>();

	if (vm.count("server.host"))
	{
		configuration.server.host = vm["server.host"].as<asiotap::endpoint>();
	}

	if (vm.count("server.https_proxy"))
	{
		configuration.server.https_proxy = vm["server.https_proxy"].as<asiotap::endpoint>();
	}

	if (vm.count("server.username"))
	{
		configuration.server.username = vm["server.username"].as<std::string>();
	}

	if (vm.count("server.password"))
	{
		configuration.server.password = vm["server.password"].as<std::string>();
	}
	else
	{
#ifdef _MSC_VER
		std::string value(256, '\0');

		DWORD value_size = GetEnvironmentVariableA("FREELAN_SERVER_PASSWORD", &value[0], static_cast<DWORD>(value.size()));

		const char* default_password = NULL;

		if (value_size > 0)
		{
			value.resize(value_size);
			default_password = value.c_str();
		}
#else
		const char* default_password = getenv("FREELAN_SERVER_PASSWORD");
#endif

		if (default_password)
		{
			configuration.server.password = default_password;
		}
	}

	if (vm.count("server.network"))
	{
		configuration.server.network= vm["server.network"].as<std::string>();
	}
	else
	{
#ifdef _MSC_VER
		std::string value(256, '\0');

		DWORD value_size = GetEnvironmentVariableA("FREELAN_SERVER_NETWORK", &value[0], static_cast<DWORD>(value.size()));

		const char* default_network = NULL;

		if (value_size > 0)
		{
			value.resize(value_size);
			default_network = value.c_str();
		}
#else
		const char* default_network = getenv("FREELAN_SERVER_NETWORK");
#endif

		if (default_network)
		{
			configuration.server.network = default_network;
		}
	}

	const std::vector<asiotap::endpoint> public_endpoint_list = vm["server.public_endpoint"].as<std::vector<asiotap::endpoint> >();
	configuration.server.public_endpoint_list.insert(public_endpoint_list.begin(), public_endpoint_list.end());

	if (vm.count("server.user_agent"))
	{
		configuration.server.user_agent = vm["server.user_agent"].as<std::string>();
	}
	else
	{
		configuration.server.user_agent = FREELAN_USER_AGENT;
	}

	configuration.server.protocol = vm["server.protocol"].as<fl::server_configuration::server_protocol_type>();
	configuration.server.ca_info = vm["server.ca_info_file"].as<fs::path>().empty() ? fs::path() : fs::absolute(vm["server.ca_info_file"].as<fs::path>(), root);

	configuration.server.disable_peer_verification = vm["server.disable_peer_verification"].as<bool>();
	configuration.server.disable_host_verification = vm["server.disable_host_verification"].as<bool>();

	// FSCP options
	configuration.fscp.hostname_resolution_protocol = vm["fscp.hostname_resolution_protocol"].as<fl::fscp_configuration::hostname_resolution_protocol_type>();
	configuration.fscp.listen_on = vm["fscp.listen_on"].as<asiotap::endpoint>();
	configuration.fscp.hello_timeout = vm["fscp.hello_timeout"].as<millisecond_duration>().to_time_duration();

	const std::vector<asiotap::endpoint> contact = vm["fscp.contact"].as<std::vector<asiotap::endpoint> >();
	configuration.fscp.contact_list.insert(contact.begin(), contact.end());

	configuration.fscp.accept_contact_requests = vm["fscp.accept_contact_requests"].as<bool>();
	configuration.fscp.accept_contacts = vm["fscp.accept_contacts"].as<bool>();
	const std::vector<std::string> dynamic_contact_file_list = vm["fscp.dynamic_contact_file"].as<std::vector<std::string> >();

	configuration.fscp.dynamic_contact_list.clear();

	BOOST_FOREACH(const fs::path& dynamic_contact_file, dynamic_contact_file_list)
	{
		configuration.fscp.dynamic_contact_list.push_back(load_certificate(fs::absolute(dynamic_contact_file, root)));
	}

	configuration.fscp.never_contact_list = vm["fscp.never_contact"].as<std::vector<asiotap::ip_network_address>>();
	configuration.fscp.cipher_suite_capabilities = vm["fscp.cipher_suite_capability"].as<std::vector<fscp::cipher_suite_type>>();

	// Security options
	cert_type signature_certificate;
	pkey signature_private_key;

	if (vm.count("security.signature_certificate_file"))
	{
		signature_certificate = load_certificate(fs::absolute(vm["security.signature_certificate_file"].as<fs::path>(), root));
	}

	if (vm.count("security.signature_private_key_file"))
	{
		signature_private_key = load_private_key(fs::absolute(vm["security.signature_private_key_file"].as<fs::path>(), root));
	}

	if (!!signature_certificate && !!signature_private_key)
	{
		configuration.security.identity = fscp::identity_store(signature_certificate, signature_private_key);
	}

	configuration.security.certificate_validation_method = vm["security.certificate_validation_method"].as<fl::security_configuration::certificate_validation_method_type>();

	const std::vector<std::string> authority_certificate_file_list = vm["security.authority_certificate_file"].as<std::vector<std::string> >();

	configuration.security.certificate_authority_list.clear();

	BOOST_FOREACH(const std::string& authority_certificate_file, authority_certificate_file_list)
	{
		configuration.security.certificate_authority_list.push_back(load_trusted_certificate(fs::absolute(authority_certificate_file, root)));
	}

	configuration.security.certificate_revocation_validation_method = vm["security.certificate_revocation_validation_method"].as<fl::security_configuration::certificate_revocation_validation_method_type>();

	const std::vector<std::string> crl_file_list = vm["security.certificate_revocation_list_file"].as<std::vector<std::string> >();

	configuration.security.certificate_revocation_list_list.clear();

	BOOST_FOREACH(const std::string& crl_file, crl_file_list)
	{
		configuration.security.certificate_revocation_list_list.push_back(load_crl(fs::absolute(crl_file, root)));
	}

	// Tap adapter options
	configuration.tap_adapter.type = vm["tap_adapter.type"].as<fl::tap_adapter_configuration::tap_adapter_type>();
	configuration.tap_adapter.enabled = vm["tap_adapter.enabled"].as<bool>();

	if (vm.count("tap_adapter.name"))
	{
		configuration.tap_adapter.name = vm["tap_adapter.name"].as<std::string>();
	}

	configuration.tap_adapter.mtu = vm["tap_adapter.mtu"].as<fl::mtu_type>();
	configuration.tap_adapter.ipv4_address_prefix_length = vm["tap_adapter.ipv4_address_prefix_length"].as<asiotap::ipv4_network_address>();
	configuration.tap_adapter.ipv6_address_prefix_length = vm["tap_adapter.ipv6_address_prefix_length"].as<asiotap::ipv6_network_address>();

	if (vm.count("tap_adapter.remote_ipv4_address"))
	{
		configuration.tap_adapter.remote_ipv4_address = vm["tap_adapter.remote_ipv4_address"].as<asiotap::ipv4_network_address>().address();
	}

	configuration.tap_adapter.arp_proxy_enabled = vm["tap_adapter.arp_proxy_enabled"].as<bool>();
	configuration.tap_adapter.arp_proxy_fake_ethernet_address = vm["tap_adapter.arp_proxy_fake_ethernet_address"].as<fl::tap_adapter_configuration::ethernet_address_type>();
	configuration.tap_adapter.dhcp_proxy_enabled = vm["tap_adapter.dhcp_proxy_enabled"].as<bool>();
	configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length = vm["tap_adapter.dhcp_server_ipv4_address_prefix_length"].as<asiotap::ipv4_network_address>();
	configuration.tap_adapter.dhcp_server_ipv6_address_prefix_length = vm["tap_adapter.dhcp_server_ipv6_address_prefix_length"].as<asiotap::ipv6_network_address>();

	// Switch options
	configuration.switch_.routing_method = vm["switch.routing_method"].as<fl::switch_configuration::routing_method_type>();
	configuration.switch_.relay_mode_enabled = vm["switch.relay_mode_enabled"].as<bool>();

	// Router
	const auto local_ip_routes = vm["router.local_ip_route"].as<std::vector<asiotap::ip_network_address> >();
	configuration.router.local_ip_routes.insert(local_ip_routes.begin(), local_ip_routes.end());

	configuration.router.client_routing_enabled = vm["router.client_routing_enabled"].as<bool>();
	configuration.router.accept_routes_requests = vm["router.accept_routes_requests"].as<bool>();
	configuration.router.internal_route_acceptance_policy = vm["router.internal_route_acceptance_policy"].as<fl::router_configuration::internal_route_scope_type>();
	configuration.router.system_route_acceptance_policy = vm["router.system_route_acceptance_policy"].as<fl::router_configuration::system_route_scope_type>();
	configuration.router.maximum_routes_limit = vm["router.maximum_routes_limit"].as<unsigned int>();
}

boost::filesystem::path get_tap_adapter_up_script(const boost::filesystem::path& root, const boost::program_options::variables_map& vm)
{
	fs::path tap_adapter_up_script_file = vm["tap_adapter.up_script"].as<fs::path>();

	return tap_adapter_up_script_file.empty() ? tap_adapter_up_script_file : fs::absolute(tap_adapter_up_script_file, root);
}

boost::filesystem::path get_tap_adapter_down_script(const boost::filesystem::path& root, const boost::program_options::variables_map& vm)
{
	fs::path tap_adapter_down_script_file = vm["tap_adapter.down_script"].as<fs::path>();

	return tap_adapter_down_script_file.empty() ? tap_adapter_down_script_file : fs::absolute(tap_adapter_down_script_file, root);
}

boost::filesystem::path get_certificate_validation_script(const boost::filesystem::path& root, const boost::program_options::variables_map& vm)
{
	fs::path certificate_validation_script_file = vm["security.certificate_validation_script"].as<fs::path>();

	return certificate_validation_script_file.empty() ? certificate_validation_script_file : fs::absolute(certificate_validation_script_file, root);
}

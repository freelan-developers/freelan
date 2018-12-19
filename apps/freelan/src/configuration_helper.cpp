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

#include <cryptoplus/hash/pbkdf2.hpp>
#include <cryptoplus/hash/message_digest_algorithm.hpp>

// This file is generated locally.
#include <defines.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace fl = freelan;

namespace
{
	struct trusted_cert_type
	{
		typedef fl::security_configuration::cert_type cert_type;

		trusted_cert_type() {}
		trusted_cert_type(cert_type& c) : cert(c) {}
		operator cert_type() const { return cert; }

		cert_type cert;
	};

	void from_file(fl::security_configuration::cert_type& value, const cryptoplus::file& file)
	{
		value = fl::security_configuration::cert_type::from_certificate(file);
	}

	void from_file(cryptoplus::pkey::pkey& value, const cryptoplus::file& file)
	{
		value = cryptoplus::pkey::pkey::from_private_key(file);
	}

	void from_file(trusted_cert_type& value, const cryptoplus::file& file)
	{
		value.cert = fl::security_configuration::cert_type::from_trusted_certificate(file);
	}

	void from_file(fl::security_configuration::crl_type& value, const cryptoplus::file& file)
	{
		value = fl::security_configuration::crl_type::from_certificate_revocation_list(file);
	}

	template <typename ValueType>
	bool load_file(const std::string& file_type, ValueType& value, const std::string& name, const fs::path& filename)
	{
		try
		{
			const cryptoplus::file file = cryptoplus::file::open(filename.native());
			from_file(value, file);
		}
		catch (const std::exception& ex)
		{
			po::error_with_option_name error("in %canonical_option%: unable to load %file_type% at \"%filename%\" (%error%)", name, filename.string());
			error.set_substitute("file_type", file_type);
			error.set_substitute("filename", filename.string());
			error.set_substitute("error", ex.what());

			throw error;
		}

		return true;
	}

	template <typename ValueType>
	bool load_file(const std::string& file_type, ValueType& value, const std::string& name, const po::variables_map& vm)
	{
		if (!vm.count(name))
		{
			return false;
		}

		const fs::path path = vm[name].as<fs::path>();

		if (path.native().empty())
		{
			return false;
		}

		return load_file(file_type, value, name, path);
	}

	template <typename ValueType>
	bool load_file_list(const std::string& file_type, std::vector<ValueType>& values, const std::string& name, const po::variables_map& vm)
	{
		values.clear();

		const auto paths = vm[name].as<std::vector<fs::path> >();

		for (auto&& path : paths)
		{
			if (path.native().empty())
			{
				continue;
			}

			ValueType value;

			if (load_file(file_type, value, name, path))
			{
				values.push_back(value);
			}
		}

		return !values.empty();
	}

	bool load_certificate(fl::security_configuration::cert_type& value, const std::string& name, const po::variables_map& vm)
	{
		return load_file("certificate", value, name, vm);
	}

	bool load_private_key(cryptoplus::pkey::pkey& value, const std::string& name, const po::variables_map& vm)
	{
		return load_file("private key", value, name, vm);
	}

	bool load_trusted_certificate(fl::security_configuration::cert_type& value, const std::string& name, const po::variables_map& vm)
	{
		trusted_cert_type xvalue;

		const bool result = load_file("trusted certificate", xvalue, name, vm);

		value = xvalue;

		return result;
	}

	bool load_certificate_list(std::vector<fl::security_configuration::cert_type>& value, const std::string& name, const po::variables_map& vm)
	{
		return load_file_list("certificate", value, name, vm);
	}

	bool load_trusted_certificate_list(std::vector<fl::security_configuration::cert_type>& value, const std::string& name, const po::variables_map& vm)
	{
		std::vector<trusted_cert_type> xvalue;

		const bool result = load_file_list("trusted certificate", xvalue, name, vm);

		value.assign(xvalue.begin(), xvalue.end());

		return result;
	}

	bool load_crl_list(std::vector<fl::security_configuration::crl_type>& value, const std::string& name, const po::variables_map& vm)
	{
		return load_file_list("certificate revocation list", value, name, vm);
	}

	void make_path_absolute(const std::string& name, po::variables_map& vm, const fs::path& root)
	{
		if (vm.count(name) > 0)
		{
			const auto path = vm[name].as<fs::path>();

			if (!path.empty())
			{
				vm.at(name).value() = fs::absolute(path, root);
			}
		}
	}

	void make_path_list_absolute(const std::string& name, po::variables_map& vm, const fs::path& root)
	{
		if (vm.count(name) > 0)
		{
			const auto paths = vm[name].as<std::vector<fs::path> >();
			std::vector<fs::path> new_paths;

			for (auto& path : paths)
			{
				if (path.empty())
				{
					new_paths.push_back(path);
				}
				else
				{
					new_paths.push_back(fs::absolute(path, root));
				}
			}

			vm.at(name).value() = new_paths;
		}
	}
}

po::options_description get_server_options()
{
	po::options_description result("FreeLAN Server options");

	result.add_options()
	("server.enabled", po::value<bool>()->default_value(false, "no"), "Whether to enable the server mechanism.")
	("server.listen_on", po::value<asiotap::endpoint>()->default_value(asiotap::ipv4_endpoint(boost::asio::ip::address_v4::any(), 443)), "The endpoint to listen on.")
	("server.protocol", po::value<fl::server_configuration::server_protocol_type>()->default_value(fl::server_configuration::server_protocol_type::https), "The protocol to use for clients to contact the server.")
	("server.server_certificate_file", po::value<fs::path>()->default_value(""), "The server certificate file.")
	("server.server_private_key_file", po::value<fs::path>()->default_value(""), "The server private key file.")
	("server.certification_authority_certificate_file", po::value<fs::path>()->default_value(""), "The certification authority certificate file.")
	("server.certification_authority_private_key_file", po::value<fs::path>()->default_value(""), "The certification authority private key file.")
	("server.authentication_script", po::value<fs::path>()->default_value(""), "The authentication script to use.")
	;

	return result;
}

po::options_description get_client_options()
{
	po::options_description result("FreeLAN Client options");

	result.add_options()
	("client.enabled", po::value<bool>()->default_value(false, "no"), "Whether to enable the client mechanism.")
	("client.server_endpoint", po::value<asiotap::endpoint>()->default_value(asiotap::ipv4_endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 443)), "The endpoint to connect to.")
	("client.protocol", po::value<fl::client_configuration::client_protocol_type>()->default_value(fl::client_configuration::client_protocol_type::https), "The protocol to use to contact the server.")
	("client.disable_peer_verification", po::value<bool>()->default_value(false, "no"), "Whether to disable peer verification.")
	("client.disable_host_verification", po::value<bool>()->default_value(false, "no"), "Whether to disable host verification.")
	("client.username", po::value<std::string>()->default_value(""), "The client username.")
	("client.password", po::value<std::string>()->default_value(""), "The client password.")
	("client.public_endpoint", po::value<std::vector<asiotap::endpoint> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::endpoint>(), ""), "A hostname or IP address to advertise.")
	;

	return result;
}

po::options_description get_fscp_options()
{
	po::options_description result("FreeLAN Secure Channel Protocol (FSCP) options");

	result.add_options()
	("fscp.hostname_resolution_protocol", po::value<fl::fscp_configuration::hostname_resolution_protocol_type>()->default_value(fl::fscp_configuration::HRP_IPV4), "The hostname resolution protocol to use.")
	("fscp.listen_on", po::value<asiotap::endpoint>()->default_value(asiotap::ipv4_endpoint(boost::asio::ip::address_v4::any(), 12000)), "The endpoint to listen on.")
	("fscp.listen_on_device", po::value<std::string>()->default_value(std::string()), "The endpoint to listen on.")
	("fscp.hello_timeout", po::value<millisecond_duration>()->default_value(3000), "The default timeout for HELLO messages, in milliseconds.")
	("fscp.contact", po::value<std::vector<asiotap::endpoint> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::endpoint>(), ""), "The address of an host to contact.")
	("fscp.accept_contact_requests", po::value<bool>()->default_value(true, "yes"), "Whether to accept CONTACT-REQUEST messages.")
	("fscp.accept_contacts", po::value<bool>()->default_value(true, "yes"), "Whether to accept CONTACT messages.")
	("fscp.dynamic_contact_file", po::value<std::vector<fs::path> >()->multitoken()->zero_tokens()->default_value(std::vector<fs::path>(), ""), "The certificate of an host to dynamically contact.")
	("fscp.never_contact", po::value<std::vector<asiotap::ip_network_address> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::ip_network_address>(), ""), "A network address to avoid when dynamically contacting hosts.")
	("fscp.cipher_suite_capability", po::value<std::vector<fscp::cipher_suite_type> >()->multitoken()->zero_tokens()->default_value(fscp::get_default_cipher_suites(), ""), "A cipher suite to allow.")
	("fscp.elliptic_curve_capability", po::value<std::vector<fscp::elliptic_curve_type> >()->multitoken()->zero_tokens()->default_value(fscp::get_default_elliptic_curves(), ""), "A elliptic curve to allow.")
	("fscp.upnp_enabled", po::value<bool>()->default_value(true, "yes"), "Enable UPnP.")
	("fscp.max_unauthenticated_messages_per_second", po::value<size_t>()->default_value(1, "1"), "Maximum unauthenticated messages from one host per second.")
	;

	return result;
}

po::options_description get_security_options()
{
	po::options_description result("Security options");

	result.add_options()
	("security.passphrase", po::value<std::string>()->default_value(""), "A passphrase to generate the pre - shared key from.")
	("security.passphrase_salt", po::value<std::string>()->default_value("freelan"), "The salt to use during the pre-shared key derivation.")
	("security.passphrase_iterations_count", po::value<unsigned int>()->default_value(2000, "2000"), "The number of iterations to use during the pre-shared key derivation.")
	("security.signature_certificate_file", po::value<fs::path>(), "The certificate file to use for signing.")
	("security.signature_private_key_file", po::value<fs::path>(), "The private key file to use for signing.")
	("security.certificate_validation_method", po::value<fl::security_configuration::certificate_validation_method_type>()->default_value(fl::security_configuration::CVM_DEFAULT), "The certificate validation method.")
	("security.certificate_validation_script", po::value<fs::path>()->default_value(""), "The certificate validation script to use.")
	("security.authority_certificate_file", po::value<std::vector<fs::path> >()->multitoken()->zero_tokens()->default_value(std::vector<fs::path>(), ""), "An authority certificate file to use.")
	("security.certificate_revocation_validation_method", po::value<fl::security_configuration::certificate_revocation_validation_method_type>()->default_value(fl::security_configuration::CRVM_NONE), "The certificate revocation validation method.")
	("security.certificate_revocation_list_file", po::value<std::vector<fs::path> >()->multitoken()->zero_tokens()->default_value(std::vector<fs::path>(), ""), "A certificate revocation list file to use.")
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
	("tap_adapter.mss_override", po::value<fl::mss_type>()->default_value(fl::mss_type()), "The MSS override.")
	("tap_adapter.metric", po::value<fl::metric_type>()->default_value(fl::auto_metric_type()), "The metric of the tap adapter.")
	("tap_adapter.ipv4_address_prefix_length", po::value<asiotap::ipv4_network_address>(), "The tap adapter IPv4 address and prefix length.")
	("tap_adapter.ipv4_dhcp", po::value<bool>()->default_value(false), "The tap adapter IPv4 DHCP status.")
	("tap_adapter.ipv6_address_prefix_length", po::value<asiotap::ipv6_network_address>(), "The tap adapter IPv6 address and prefix length.")
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
	("router.local_ip_route", po::value<std::vector<freelan::ip_route> >()->multitoken()->zero_tokens()->default_value(std::vector<freelan::ip_route>(), ""), "A route to advertise to the other peers.")
	("router.local_dns_server", po::value<std::vector<asiotap::ip_address> >()->multitoken()->zero_tokens()->default_value(std::vector<asiotap::ip_address>(), ""), "A DNS server to advertise to the other peers.")
	("router.client_routing_enabled", po::value<bool>()->default_value(true, "yes"), "Whether to enable client routing.")
	("router.accept_routes_requests", po::value<bool>()->default_value(true, "yes"), "Whether to accept routes requests.")
	("router.internal_route_acceptance_policy", po::value<fl::router_configuration::internal_route_scope_type>()->default_value(fl::router_configuration::internal_route_scope_type::unicast_in_network), "The internal route acceptance policy.")
	("router.system_route_acceptance_policy", po::value<fl::router_configuration::system_route_scope_type>()->default_value(fl::router_configuration::system_route_scope_type::none), "The system route acceptance policy.")
	("router.maximum_routes_limit", po::value<unsigned int>()->default_value(1), "The maximum count of routes to accept for a given host.")
	("router.dns_servers_acceptance_policy", po::value<fl::router_configuration::dns_servers_scope_type>()->default_value(fl::router_configuration::dns_servers_scope_type::in_network), "The DNS servers acceptance policy.")
	("router.dns_script", po::value<fs::path>()->default_value(""), "The DNS script.")
	;

	return result;
}

void make_paths_absolute(boost::program_options::variables_map& vm, const boost::filesystem::path& root)
{
	make_path_absolute("server.server_certificate_file", vm, root);
	make_path_absolute("server.server_private_key_file", vm, root);
	make_path_absolute("server.certification_authority_certificate_file", vm, root);
	make_path_absolute("server.certification_authority_private_key_file", vm, root);
	make_path_absolute("server.authentication_script", vm, root);
	make_path_list_absolute("fscp.dynamic_contact_file", vm, root);
	make_path_absolute("security.signature_certificate_file", vm, root);
	make_path_absolute("security.signature_private_key_file", vm, root);
	make_path_absolute("security.certificate_validation_script", vm, root);
	make_path_list_absolute("security.authority_certificate_file", vm, root);
	make_path_list_absolute("security.certificate_revocation_list_file", vm, root);
	make_path_absolute("tap_adapter.up_script", vm, root);
	make_path_absolute("tap_adapter.down_script", vm, root);
}

void setup_configuration(const fscp::logger& logger, fl::configuration& configuration, const po::variables_map& vm)
{
	typedef fl::security_configuration::cert_type cert_type;
	typedef cryptoplus::pkey::pkey pkey;

	// Server options
	configuration.server.enabled = vm["server.enabled"].as<bool>();
	configuration.server.listen_on = vm["server.listen_on"].as<asiotap::endpoint>();
	configuration.server.protocol = vm["server.protocol"].as<fl::server_configuration::server_protocol_type>();

	if (load_certificate(configuration.server.server_certificate, "server.server_certificate_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded server certificate from: " << vm["server.server_certificate_file"].as<fs::path>();
	}

	if (load_private_key(configuration.server.server_private_key, "server.server_private_key_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded server private key from: " << vm["server.server_private_key_file"].as<fs::path>();
	}

	if (load_trusted_certificate(configuration.server.certification_authority_certificate, "server.certification_authority_certificate_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded server CA certificate from: " << vm["server.certification_authority_certificate_file"].as<fs::path>();
	}

	if (load_private_key(configuration.server.certification_authority_private_key, "server.certification_authority_private_key_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded server CA private key from: " << vm["server.certification_authority_private_key_file"].as<fs::path>();
	}
	
	configuration.server.authentication_script = vm["server.authentication_script"].as<fs::path>();

	// Client options
	configuration.client.enabled = vm["client.enabled"].as<bool>();
	configuration.client.server_endpoint = vm["client.server_endpoint"].as<asiotap::endpoint>();
	configuration.client.protocol = vm["client.protocol"].as<fl::client_configuration::client_protocol_type>();
	configuration.client.disable_peer_verification = vm["client.disable_peer_verification"].as<bool>();
	configuration.client.disable_host_verification = vm["client.disable_host_verification"].as<bool>();
	configuration.client.username = vm["client.username"].as<std::string>();
	configuration.client.password = vm["client.password"].as<std::string>();

	const std::vector<asiotap::endpoint> public_endpoint = vm["client.public_endpoint"].as<std::vector<asiotap::endpoint> >();
	configuration.client.public_endpoint_list.insert(public_endpoint.begin(), public_endpoint.end());

	// FSCP options
	configuration.fscp.hostname_resolution_protocol = vm["fscp.hostname_resolution_protocol"].as<fl::fscp_configuration::hostname_resolution_protocol_type>();
	configuration.fscp.listen_on = vm["fscp.listen_on"].as<asiotap::endpoint>();
	configuration.fscp.listen_on_device = vm["fscp.listen_on_device"].as<std::string>();
	configuration.fscp.hello_timeout = vm["fscp.hello_timeout"].as<millisecond_duration>().to_time_duration();

	const std::vector<asiotap::endpoint> contact = vm["fscp.contact"].as<std::vector<asiotap::endpoint> >();
	configuration.fscp.contact_list.insert(contact.begin(), contact.end());

	configuration.fscp.accept_contact_requests = vm["fscp.accept_contact_requests"].as<bool>();
	configuration.fscp.accept_contacts = vm["fscp.accept_contacts"].as<bool>();

	if (load_certificate_list(configuration.fscp.dynamic_contact_list, "fscp.dynamic_contact_file", vm))
	{
		for (auto _file : vm["fscp.dynamic_contact_file"].as<std::vector<fs::path>>())
		{
			logger(fscp::log_level::information) << "Loaded dynamic contact certificate from: " << _file;
		}
	}

	configuration.fscp.never_contact_list = vm["fscp.never_contact"].as<std::vector<asiotap::ip_network_address>>();
	configuration.fscp.cipher_suite_capabilities = vm["fscp.cipher_suite_capability"].as<std::vector<fscp::cipher_suite_type>>();
	configuration.fscp.elliptic_curve_capabilities = vm["fscp.elliptic_curve_capability"].as<std::vector<fscp::elliptic_curve_type>>();
	configuration.fscp.upnp_enabled = vm["fscp.upnp_enabled"].as<bool>();
	configuration.fscp.max_unauthenticated_messages_per_second = vm["fscp.max_unauthenticated_messages_per_second"].as<size_t>();

	// Security options
	const std::string passphrase = vm["security.passphrase"].as<std::string>();
	const std::string passphrase_salt = vm["security.passphrase_salt"].as<std::string>();
	const unsigned int passphrase_iterations_count = vm["security.passphrase_iterations_count"].as<unsigned int>();
	cryptoplus::buffer pre_shared_key;

	if (!passphrase.empty())
	{
		logger(fscp::log_level::information) << "Deriving pre-shared key from passphrase...";

		const auto mdalg = cryptoplus::hash::message_digest_algorithm(NID_sha256);
		pre_shared_key = cryptoplus::hash::pbkdf2(&passphrase[0], passphrase.size(), &passphrase_salt[0], passphrase_salt.size(), mdalg, passphrase_iterations_count);
	}

	cert_type signature_certificate;
	pkey signature_private_key;

	if (load_certificate(signature_certificate, "security.signature_certificate_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded signature certificate from: " << vm["security.signature_certificate_file"].as<fs::path>();
	}

	if (load_private_key(signature_private_key, "security.signature_private_key_file", vm))
	{
		logger(fscp::log_level::information) << "Loaded signature private key from: " << vm["security.signature_private_key_file"].as<fs::path>();
	}

	if ((!!signature_certificate && !!signature_private_key) || !!pre_shared_key)
	{
		configuration.security.identity = fscp::identity_store(signature_certificate, signature_private_key, pre_shared_key);
	}

	configuration.security.certificate_validation_method = vm["security.certificate_validation_method"].as<fl::security_configuration::certificate_validation_method_type>();
	configuration.security.certificate_validation_script = vm["security.certificate_validation_script"].as<fs::path>();

	if (load_trusted_certificate_list(configuration.security.certificate_authority_list, "security.authority_certificate_file", vm))
	{
		for (auto _file : vm["security.authority_certificate_file"].as<std::vector<fs::path>>())
		{
			logger(fscp::log_level::information) << "Loaded authority certificate from: " << _file;
		}
	}

	configuration.security.certificate_revocation_validation_method = vm["security.certificate_revocation_validation_method"].as<fl::security_configuration::certificate_revocation_validation_method_type>();

	if (load_crl_list(configuration.security.certificate_revocation_list_list, "security.certificate_revocation_list_file", vm))
	{
		for (auto _file : vm["security.certificate_revocation_list_file"].as<std::vector<fs::path>>())
		{
			logger(fscp::log_level::information) << "Loaded certificate revocation file from: " << _file;
		}
	}

	// Tap adapter options
	configuration.tap_adapter.type = vm["tap_adapter.type"].as<fl::tap_adapter_configuration::tap_adapter_type>();
	configuration.tap_adapter.enabled = vm["tap_adapter.enabled"].as<bool>();

	if (vm.count("tap_adapter.name"))
	{
		configuration.tap_adapter.name = vm["tap_adapter.name"].as<std::string>();
	}

	configuration.tap_adapter.mtu = vm["tap_adapter.mtu"].as<fl::mtu_type>();
	configuration.tap_adapter.mss_override = vm["tap_adapter.mss_override"].as<fl::mss_type>();
	configuration.tap_adapter.metric = vm["tap_adapter.metric"].as<fl::metric_type>();

	if (vm.count("tap_adapter.ipv4_address_prefix_length"))
	{
		configuration.tap_adapter.ipv4_address_prefix_length = vm["tap_adapter.ipv4_address_prefix_length"].as<asiotap::ipv4_network_address>();
	}
	
	if (vm.count("tap_adapter.ipv4_dhcp"))
	{
		configuration.tap_adapter.ipv4_dhcp = vm["tap_adapter.ipv4_dhcp"].as<bool>();
	}

	if (vm.count("tap_adapter.ipv6_address_prefix_length"))
	{
		configuration.tap_adapter.ipv6_address_prefix_length = vm["tap_adapter.ipv6_address_prefix_length"].as<asiotap::ipv6_network_address>();
	}

	if (vm.count("tap_adapter.remote_ipv4_address"))
	{
		configuration.tap_adapter.remote_ipv4_address = vm["tap_adapter.remote_ipv4_address"].as<asiotap::ipv4_network_address>().address();
	}

	configuration.tap_adapter.arp_proxy_enabled = vm["tap_adapter.arp_proxy_enabled"].as<bool>();
	configuration.tap_adapter.arp_proxy_fake_ethernet_address = vm["tap_adapter.arp_proxy_fake_ethernet_address"].as<fl::tap_adapter_configuration::ethernet_address_type>();
	configuration.tap_adapter.dhcp_proxy_enabled = vm["tap_adapter.dhcp_proxy_enabled"].as<bool>();
	configuration.tap_adapter.dhcp_server_ipv4_address_prefix_length = vm["tap_adapter.dhcp_server_ipv4_address_prefix_length"].as<asiotap::ipv4_network_address>();
	configuration.tap_adapter.dhcp_server_ipv6_address_prefix_length = vm["tap_adapter.dhcp_server_ipv6_address_prefix_length"].as<asiotap::ipv6_network_address>();
	configuration.tap_adapter.up_script = vm["tap_adapter.up_script"].as<fs::path>();
	configuration.tap_adapter.down_script = vm["tap_adapter.down_script"].as<fs::path>();

	// Switch options
	configuration.switch_.routing_method = vm["switch.routing_method"].as<fl::switch_configuration::routing_method_type>();
	configuration.switch_.relay_mode_enabled = vm["switch.relay_mode_enabled"].as<bool>();

	// Router
	const auto local_ip_routes = vm["router.local_ip_route"].as<std::vector<freelan::ip_route> >();
	configuration.router.local_ip_routes.insert(local_ip_routes.begin(), local_ip_routes.end());
	const auto local_ip_dns_servers = vm["router.local_dns_server"].as<std::vector<asiotap::ip_address> >();
	configuration.router.local_dns_servers.insert(local_ip_dns_servers.begin(), local_ip_dns_servers.end());

	configuration.router.client_routing_enabled = vm["router.client_routing_enabled"].as<bool>();
	configuration.router.accept_routes_requests = vm["router.accept_routes_requests"].as<bool>();
	configuration.router.internal_route_acceptance_policy = vm["router.internal_route_acceptance_policy"].as<fl::router_configuration::internal_route_scope_type>();
	configuration.router.system_route_acceptance_policy = vm["router.system_route_acceptance_policy"].as<fl::router_configuration::system_route_scope_type>();
	configuration.router.maximum_routes_limit = vm["router.maximum_routes_limit"].as<unsigned int>();
	configuration.router.dns_servers_acceptance_policy = vm["router.dns_servers_acceptance_policy"].as<fl::router_configuration::dns_servers_scope_type>();
	configuration.router.dns_script = vm["router.dns_script"].as<fs::path>();
}

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
 * \file configuration.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The configuration structure.
 */

#ifndef FREELAN_CONFIGURATION_HPP
#define FREELAN_CONFIGURATION_HPP

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <cryptoplus/x509/certificate_revocation_list.hpp>

#include <fscp/server.hpp>
#include <fscp/identity_store.hpp>

#include <asiotap/tap_adapter.hpp>
#include <asiotap/osi/ethernet_address.hpp>

#include "endpoint.hpp"
#include "ip_network_address.hpp"
#include "mtu.hpp"

namespace freelan
{
	class core;

	/**
	 * \brief The server configuration.
	 */
	struct server_configuration
	{
		/**
		 * \brief The endpoint type.
		 */
		typedef freelan::endpoint endpoint;

		/**
		 * \brief The endpoint list type.
		 */
		typedef std::vector<endpoint> endpoint_list;

		/**
		 * \brief Create a new server configuration.
		 */
		server_configuration();

		/**
		 * \brief Whether the server mechanism is enabled.
		 */
		bool enabled;

		/**
		 * \brief The server host name.
		 */
		endpoint host;

		/**
		 * \brief The username.
		 */
		std::string username;

		/**
		 * \brief The password.
		 */
		std::string password;

		/**
		 * \brief The network.
		 */
		std::string network;

		/**
		 * \brief The public endpoint list.
		 */
		endpoint_list public_endpoint_list;

		/**
		 * \brief The https proxy host name.
		 */
		boost::optional<endpoint> https_proxy;

		/**
		 * \brief The user agent.
		 */
		std::string user_agent;

		/**
		 * \brief The server protocol type.
		 */
		enum server_protocol_type
		{
			SP_HTTP = 0x00, /**< \brief HTTP. */
			SP_HTTPS = 0x01 /**< \brief HTTPS. */
		};

		/**
		 * \brief The protocol to use.
		 */
		server_protocol_type protocol;

		/**
		 * \brief The CA info file.
		 */
		boost::filesystem::path ca_info;

		/**
		 * \brief Disable peer verification.
		 */
		bool disable_peer_verification;

		/**
		 * \brief Disable host verification.
		 */
		bool disable_host_verification;
	};

	/**
	 * \brief The FSCP related options type.
	 */
	struct fscp_configuration
	{
		/**
		 * \brief The endpoint type.
		 */
		typedef freelan::endpoint endpoint;

		/**
		 * \brief The endpoint list type.
		 */
		typedef std::vector<endpoint> endpoint_list;

		/**
		 * \brief The hostname resolution protocol type.
		 */
		enum hostname_resolution_protocol_type
		{
			HRP_IPV4 = PF_INET, /**< \brief The IPv4 protocol. */
			HRP_IPV6 = PF_INET6 /**< \brief The IPv6 protocol. */
		};

		/**
		 * \brief The certificate type.
		 */
		typedef fscp::identity_store::cert_type cert_type;

		/**
		 * \brief The certificate list type.
		 */
		typedef std::vector<cert_type> cert_list_type;

		/**
		 * \brief The ip_network_address list type.
		 */
		typedef std::vector<ip_network_address> ip_network_address_list_type;

		/**
		 * \brief Create a new FSCP configuration.
		 */
		fscp_configuration();

		/**
		 * \brief The endpoint to listen on.
		 */
		endpoint listen_on;

		/**
		 * \brief The contact list.
		 */
		endpoint_list contact_list;

		/**
		 * \brief The "accept contact requests" flag.
		 */
		bool accept_contact_requests;

		/**
		 * \brief The "accept contacts" flag.
		 */
		bool accept_contacts;

		/**
		 * \brief The dynamic contact list.
		 */
		cert_list_type dynamic_contact_list;

		/**
		 * \brief The never contact list.
		 */
		ip_network_address_list_type never_contact_list;

		/**
		 * \brief The hostname resolution protocol.
		 */
		hostname_resolution_protocol_type hostname_resolution_protocol;

		/**
		 * \brief The hello timeout.
		 */
		boost::posix_time::time_duration hello_timeout;

		/**
		 * \brief The list of allowed cipher algorithms.
		 */
		fscp::cipher_algorithm_list_type cipher_capabilities;
	};

	/**
	 * \brief The security related options type.
	 */
	struct security_configuration
	{
		/**
		 * \brief The certificate validation method type.
		 */
		enum certificate_validation_method_type
		{
			CVM_DEFAULT, /**< \brief The default certificate validation method. */
			CVM_NONE /**< \brief No certificate validation. */
		};

		/**
		 * \brief The certificate revocation validation method type.
		 */
		enum certificate_revocation_validation_method_type
		{
			CRVM_LAST, /**< \brief Only the last certificate of the chain is checked for revocation. */
			CRVM_ALL, /**< \brief All certificates from the chain are checked for revocation. */
			CRVM_NONE /**< \brief No certificate is checked for revocation. */
		};

		/**
		 * \brief The certificate type.
		 */
		typedef fscp::identity_store::cert_type cert_type;

		/**
		 * \brief The certificate revocation list type.
		 */
		typedef cryptoplus::x509::certificate_revocation_list crl_type;

		/**
		 * \brief The certificate list type.
		 */
		typedef std::vector<cert_type> cert_list_type;

		/**
		 * \brief The certificate revocation list list type.
		 */
		typedef std::vector<crl_type> crl_list_type;

		/**
		 * \brief The certificate validation callback type.
		 */
		typedef boost::function<bool (core&, cert_type)> certificate_validation_callback_type;

		/**
		 * \brief Constructor.
		 */
		security_configuration();

		/**
		 * \brief The identity.
		 */
		boost::optional<fscp::identity_store> identity;

		/**
		 * \brief The certificate validation method.
		 */
		certificate_validation_method_type certificate_validation_method;

		/**
		 * \brief The certificate validation callback.
		 */
		certificate_validation_callback_type certificate_validation_callback;

		/**
		 * \brief The certificate authorities.
		 */
		cert_list_type certificate_authority_list;

		/**
		 * \brief The certificate revocation validation method.
		 */
		certificate_revocation_validation_method_type certificate_revocation_validation_method;

		/**
		 * \brief The certificate revocation lists.
		 */
		crl_list_type certificate_revocation_list_list;
	};

	/**
	 * \brief Tap adapter related options type.
	 */
	struct tap_adapter_configuration
	{
		/**
		 * \brief The ethernet address type.
		 */
		typedef asiotap::osi::ethernet_address ethernet_address_type;

		/**
		 * \brief The tap adapter type.
		 */
		enum tap_adapter_type
		{
			TAT_TAP = 0,
			TAT_TUN = 1
		};

		/**
		 * \brief Constructor.
		 */
		tap_adapter_configuration();

		/**
		 * \brief Whether the tap adapter is enabled.
		 */
		bool enabled;

		/**
		 * \brief The tap adapter type.
		 */
		tap_adapter_type type;

		/**
		 * \brief The tap adapter's name.
		 */
		std::string name;

		/**
		 * \brief The tap adapter's MTU.
		 */
		mtu_type mtu;

		/**
		 * \brief The IPv4 tap adapter address.
		 */
		ipv4_network_address ipv4_address_prefix_length;

		/**
		 * \brief The IPv6 tap adapter address.
		 */
		ipv6_network_address ipv6_address_prefix_length;

		/**
		 * \brief Whether to enable the ARP proxy.
		 */
		bool arp_proxy_enabled;

		/**
		 * \brief The ARP proxy fake ethernet address.
		 */
		ethernet_address_type arp_proxy_fake_ethernet_address;

		/**
		 * \brief Whether to enable the DHCP proxy.
		 */
		bool dhcp_proxy_enabled;

		/**
		 * \brief The DHCP server IPv4 address.
		 */
		ipv4_network_address dhcp_server_ipv4_address_prefix_length;

		/**
		 * \brief The DHCP server IPv6 address.
		 */
		ipv6_network_address dhcp_server_ipv6_address_prefix_length;

		/**
		 * \brief The remote IPv4 address.
		 */
		boost::optional<boost::asio::ip::address_v4> remote_ipv4_address;

		/**
		 * \brief The up callback type.
		 */
		typedef boost::function<void (core&, const asiotap::tap_adapter&)> up_callback_type;

		/**
		 * \brief The down callback type.
		 */
		typedef up_callback_type down_callback_type;

		/**
		 * \brief The up callback.
		 */
		up_callback_type up_callback;

		/**
		 * \brief The down callback.
		 */
		down_callback_type down_callback;
	};

	/**
	 * \brief The switch related options type.
	 */
	struct switch_configuration
	{
		/**
		 * \brief The routing method type.
		 */
		enum routing_method_type
		{
			RM_SWITCH, /**< \brief The switch routing method. */
			RM_HUB /**< \brief The hub routing method. */
		};

		/**
		 * \brief Constructor.
		 */
		switch_configuration();

		/**
		 * \brief The routing method.
		 */
		routing_method_type routing_method;

		/**
		 * \brief Whether to enable the relay mode.
		 */
		bool relay_mode_enabled;
	};

	/**
	 * \brief The router related options type.
	 */
	struct router_configuration
	{
		/**
		 * \brief Constructor.
		 */
		router_configuration();

		/**
		 * \brief The local IP routes.
		 *
		 * These routes are sent to the other hosts for VPN routing.
		 */
		routes_type local_ip_routes;

		/**
		 * \brief Whether to enable client routing.
		 */
		bool client_routing_enabled;
	};

	/**
	 * \brief The configuration structure.
	 */
	struct configuration
	{
		/**
		 * \brief The server related options.
		 */
		freelan::server_configuration server;

		/**
		 * \brief The FSCP related options.
		 */
		freelan::fscp_configuration fscp;

		/**
		 * \brief The security configuration.
		 */
		freelan::security_configuration security;

		/**
		 * \brief The Tap adapter related options.
		 */
		freelan::tap_adapter_configuration tap_adapter;

		/**
		 * \brief The switch related options.
		 */
		freelan::switch_configuration switch_;

		/**
		 * \brief The router related options.
		 */
		freelan::router_configuration router;

		/**
		 * \brief The constructor.
		 */
		configuration();
	};

	/**
	 * \brief Input a server protocol.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, server_configuration::server_protocol_type& value);

	/**
	 * \brief Output a server protocol to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, server_configuration::server_protocol_type& value);

	/**
	 * \brief Convert a hostname resolution protocol type into a boost asio udp endpoint type.
	 * \param value The value to convert.
	 * \return The boost::asio::ip::udp.
	 */
	boost::asio::ip::udp to_protocol(fscp_configuration::hostname_resolution_protocol_type value);

	/**
	 * \brief Input a hostname resolution protocol.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, fscp_configuration::hostname_resolution_protocol_type& value);

	/**
	 * \brief Output an hostname resolution protocol to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, fscp_configuration::hostname_resolution_protocol_type& value);

	/**
	 * \brief Input a certificate validation method.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, security_configuration::certificate_validation_method_type& value);

	/**
	 * \brief Output a certificate validation method to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, security_configuration::certificate_validation_method_type& value);

	/**
	 * \brief Input a certificate revocation validation method.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, security_configuration::certificate_revocation_validation_method_type& value);

	/**
	 * \brief Output a certificate revocation validation method to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, security_configuration::certificate_revocation_validation_method_type& value);

	/**
	 * \brief Input a tap adapter type.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, tap_adapter_configuration::tap_adapter_type& value);

	/**
	 * \brief Output a tap adapter type to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::tap_adapter_type& value);

	/**
	 * \brief Input a routing method.
	 * \param is The input stream.
	 * \param value The value to read.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, switch_configuration::routing_method_type& value);

	/**
	 * \brief Output a routing method to a stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, switch_configuration::routing_method_type& value);
}

#endif /* FREELAN_CONFIGURATION_HPP */

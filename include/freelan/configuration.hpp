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

#include <vector>
#include <iostream>
#include <iomanip>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptoplus/x509/certificate_revocation_list.hpp>

#include <fscp/server.hpp>
#include <fscp/identity_store.hpp>

#include <asiotap/tap_adapter.hpp>
#include <asiotap/osi/arp_proxy.hpp>

#include "endpoint.hpp"

namespace freelan
{
	class core;

	/**
	 * \brief The FSCP related options type.
	 */
	struct fscp_configuration
	{
		/**
		 * \brief The endpoint type.
		 */
		typedef boost::shared_ptr<endpoint> ep_type;

		/**
		 * \brief The endpoint list type.
		 */
		typedef std::vector<ep_type> ep_list_type;

		/**
		 * \brief The hostname resolution protocol type.
		 */
		enum hostname_resolution_protocol_type
		{
			HRP_IPV4 = PF_INET, /**< \brief The IPv4 protocol. */
			HRP_IPV6 = PF_INET6/**< \brief The IPv6 protocol. */
		};

		/**
		 * \brief Create a new FSCP configuration.
		 */
		fscp_configuration();

		/**
		 * \brief The endpoint to listen on.
		 */
		ep_type listen_on;

		/**
		 * \brief The contact list.
		 */
		ep_list_type contact_list;

		/**
		 * \brief The hostname resolution protocol.
		 */
		hostname_resolution_protocol_type hostname_resolution_protocol;

		/**
		 * \brief The hello timeout.
		 */
		boost::posix_time::time_duration hello_timeout;
	};

	/**
	 * \brief Convert a hostname resolution protocol type into a boost asio udp endpoint type.
	 * \param value The value to convert.
	 * \return The boost::asio::ip::udp.
	 */
	boost::asio::ip::udp convert(fscp_configuration::hostname_resolution_protocol_type value);

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
		typedef asiotap::osi::proxy<asiotap::osi::arp_frame>::ethernet_address_type ethernet_address_type;

		/**
		 * \brief The IPv4 address/prefix length pair.
		 */
		struct ipv4_address_prefix_length_type
		{
			boost::asio::ip::address_v4 address;
			unsigned int prefix_length;
		};

		/**
		 * \brief The IPv6 address/prefix length pair.
		 */
		struct ipv6_address_prefix_length_type
		{
			boost::asio::ip::address_v6 address;
			unsigned int prefix_length;
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
		 * \brief The IPv4 tap adapter address.
		 */
		boost::optional<ipv4_address_prefix_length_type> ipv4_address_prefix_length;

		/**
		 * \brief The IPv6 tap adapter address.
		 */
		boost::optional<ipv6_address_prefix_length_type> ipv6_address_prefix_length;

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
		boost::optional<ipv4_address_prefix_length_type> dhcp_server_ipv4_address_prefix_length;

		/**
		 * \brief The DHCP server IPv6 address.
		 */
		boost::optional<ipv6_address_prefix_length_type> dhcp_server_ipv6_address_prefix_length;

		/**
		 * \brief The up callback type.
		 */
		typedef boost::function<void (const asiotap::tap_adapter&)> up_callback_type;

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
	 * \brief The configuration structure.
	 */
	struct configuration
	{
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
		 * \brief The constructor.
		 */
		configuration();
	};

	/**
	 * \brief Output an Ethernet address to a stream.
	 * \param os The output stream.
	 * \param value The Ethernet address.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ethernet_address_type value);

	/**
	 * \brief Output an IPv4 address/prefix length to a stream.
	 * \param os The output stream.
	 * \param value The IPv4 address/prefix length.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ipv4_address_prefix_length_type value);

	/**
	 * \brief Output an IPv6 address/prefix length to a stream.
	 * \param os The output stream.
	 * \param value The IPv6 address/prefix length.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ipv6_address_prefix_length_type value);

	inline std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ipv4_address_prefix_length_type value)
	{
		return os << value.address << "/" << std::dec << std::setw(0) << value.prefix_length;
	}

	inline std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ipv6_address_prefix_length_type value)
	{
		return os << value.address << "/" << std::dec << std::setw(0) << value.prefix_length;
	}
}

#endif /* FREELAN_CONFIGURATION_HPP */

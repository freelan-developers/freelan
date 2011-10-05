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

#include <fscp/server.hpp>
#include <fscp/identity_store.hpp>

#include <asiotap/osi/arp_proxy.hpp>

namespace freelan
{
	/**
	 * \brief The configuration structure.
	 */
	struct configuration
	{
		/**
		 * \brief The ethernet address type.
		 */
		typedef asiotap::osi::proxy<asiotap::osi::arp_frame>::ethernet_address_type ethernet_address_type;
			
		/**
		 * \brief The endpoint type.
		 */
		typedef fscp::server::ep_type ep_type;

		/**
		 * \brief The endpoint list type.
		 */
		typedef std::vector<ep_type> ep_list_type;

		/**
		 * \brief The hostname resolution protocol type.
		 */
		typedef boost::asio::ip::udp::resolver::query::protocol_type hostname_resolution_protocol_type;

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
		 * \brief The routing method type.
		 */
		enum routing_method_type
		{
			RM_SWITCH, /**< \brief The switch routing method. */
			RM_HUB /**< \brief The hub routing method. */
		};

		/**
		 * \brief The certificate validation method type.
		 */
		enum certificate_validation_method_type
		{
			CVM_DEFAULT, /**< \brief The default certificate validation method. */
			CVM_NONE /**< \brief No certificate validation. */
		};

		/**
		 * \brief The certificate type.
		 */
		typedef fscp::identity_store::cert_type cert_type;

		/**
		 * \brief The certificate validation callback type.
		 */
		typedef boost::function<bool (cert_type)> certificate_validation_callback_type;

		/**
		 * \brief The constructor.
		 */
		configuration();

		/**
		 * \brief The endpoint to listen on.
		 */
		ep_type listen_on;

		/**
		 * \brief The hostname resolution protocol.
		 */
		hostname_resolution_protocol_type hostname_resolution_protocol;

		/**
		 * \brief The IPv4 tap adapter address.
		 */
		boost::optional<ipv4_address_prefix_length_type> tap_adapter_ipv4_address_prefix_length;

		/**
		 * \brief The IPv6 tap adapter address.
		 */
		boost::optional<ipv6_address_prefix_length_type> tap_adapter_ipv6_address_prefix_length;

		/**
		 * \brief Whether to enable the ARP proxy.
		 */
		bool enable_arp_proxy;

		/**
		 * \brief The ARP proxy fake ethernet address.
		 */
		ethernet_address_type arp_proxy_fake_ethernet_address;

		/**
		 * \brief Whether to enable the DHCP proxy.
		 */
		bool enable_dhcp_proxy;

		/**
		 * \brief The DHCP server IPv4 address.
		 */
		boost::optional<ipv4_address_prefix_length_type> dhcp_server_ipv4_address_prefix_length;

		/**
		 * \brief The DHCP server IPv6 address.
		 */
		boost::optional<ipv6_address_prefix_length_type> dhcp_server_ipv6_address_prefix_length;

		/**
		 * \brief The routing method.
		 */
		routing_method_type routing_method;

		/**
		 * \brief The hello timeout.
		 */
		boost::posix_time::time_duration hello_timeout;

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
		 * \brief The contact list.
		 */
		ep_list_type contact_list;

		/**
		 * \brief Whether to use the whitelist.
		 */
		bool use_whitelist;

		/**
		 * \brief The whitelist.
		 */
		ep_list_type whitelist;

		/**
		 * \brief Whether to use the blacklist.
		 */
		bool use_blacklist;

		/**
		 * \brief The blacklist.
		 */
		ep_list_type blacklist;
	};

	/**
	 * \brief Output an Ethernet address to a stream.
	 * \param os The output stream.
	 * \param value The Ethernet address.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, configuration::ethernet_address_type value);

	/**
	 * \brief Input an Ethernet address from a stream.
	 * \param is The input stream.
	 * \param value The Ethernet address.
	 * \return in.
	 */
	std::istream& operator>>(std::istream& is, configuration::ethernet_address_type& value);

	/**
	 * \brief Output an IPv4 address/prefix length to a stream.
	 * \param os The output stream.
	 * \param value The IPv4 address/prefix length.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, configuration::ipv4_address_prefix_length_type value);

	/**
	 * \brief Input an IPv4 address/prefix length from a stream.
	 * \param is The input stream.
	 * \param value The IPv4 address/prefix length.
	 * \return in.
	 */
	std::istream& operator>>(std::istream& is, configuration::ipv4_address_prefix_length_type& value);

	/**
	 * \brief Output an IPv6 address/prefix length to a stream.
	 * \param os The output stream.
	 * \param value The IPv6 address/prefix length.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, configuration::ipv6_address_prefix_length_type value);

	/**
	 * \brief Input an IPv6 address/prefix length from a stream.
	 * \param is The input stream.
	 * \param value The IPv6 address/prefix length.
	 * \return in.
	 */
	std::istream& operator>>(std::istream& is, configuration::ipv6_address_prefix_length_type& value);

	inline std::ostream& operator<<(std::ostream& os, configuration::ipv4_address_prefix_length_type value)
	{
		return os << value.address << "/" << std::dec << std::setw(0) << value.prefix_length;
	}

	inline std::ostream& operator<<(std::ostream& os, configuration::ipv6_address_prefix_length_type value)
	{
		return os << value.address << "/" << std::dec << std::setw(0) << value.prefix_length;
	}
}

#endif /* FREELAN_CONFIGURATION_HPP */

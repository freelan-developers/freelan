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
 * \file configuration.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The configuration structure.
 */

#include "configuration.hpp"

#include <sstream>
#include <stdexcept>
#include <cassert>

namespace freelan
{
	server_configuration::server_configuration() :
		enabled(false),
		listen_on(asiotap::ipv4_endpoint(boost::asio::ip::address_v4::any(), 443)),
		protocol(server_protocol_type::https),
		authentication_script(),
		registration_validity_duration(boost::posix_time::minutes(30))
	{
	}

	client_configuration::client_configuration() :
		enabled(false),
		server_endpoint(),
		protocol(client_protocol_type::https),
		disable_peer_verification(false),
		disable_host_verification(false),
		username(),
		password(),
		public_endpoint_list()
	{
	}

	fscp_configuration::fscp_configuration() :
		listen_on(),
		listen_on_device(),
		contact_list(),
		accept_contact_requests(true),
		accept_contacts(true),
		hostname_resolution_protocol(HRP_IPV4),
		hello_timeout(boost::posix_time::seconds(3))
	{
	}

	security_configuration::security_configuration() :
		identity(),
		certificate_validation_method(CVM_DEFAULT),
		certificate_validation_script(),
		certificate_authority_list(),
		certificate_revocation_validation_method(CRVM_NONE),
		certificate_revocation_list_list()
	{
	}

	tap_adapter_configuration::tap_adapter_configuration() :
		enabled(true),
		type(tap_adapter_type::tap),
		ipv4_address_prefix_length(),
		ipv6_address_prefix_length(),
		arp_proxy_enabled(false),
		arp_proxy_fake_ethernet_address(),
		dhcp_proxy_enabled(false),
		dhcp_server_ipv4_address_prefix_length(),
		dhcp_server_ipv6_address_prefix_length(),
		up_script(),
		down_script()
	{
	}

	switch_configuration::switch_configuration() :
		routing_method(RM_SWITCH),
		relay_mode_enabled(false)
	{
	}

	router_configuration::router_configuration() :
		local_ip_routes(),
		local_dns_servers(),
		client_routing_enabled(false),
		accept_routes_requests(true),
		internal_route_acceptance_policy(internal_route_scope_type::unicast_in_network),
		system_route_acceptance_policy(system_route_scope_type::none),
		maximum_routes_limit(1),
		dns_servers_acceptance_policy(dns_servers_scope_type::in_network),
		dns_script()
	{
	}

	configuration::configuration() :
		server(),
		fscp(),
		security(),
		tap_adapter(),
		switch_(),
		router()
	{
	}

	std::istream& operator>>(std::istream& is, server_configuration::server_protocol_type& v)
	{
		std::string value;

		is >> value;

		if (value == "http")
			v = server_configuration::server_protocol_type::http;
		else if (value == "https")
			v = server_configuration::server_protocol_type::https;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const server_configuration::server_protocol_type& value)
	{
		switch (value)
		{
			case server_configuration::server_protocol_type::http:
				return os << "http";
			case server_configuration::server_protocol_type::https:
				return os << "https";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	boost::asio::ip::udp to_protocol(fscp_configuration::hostname_resolution_protocol_type value)
	{
		switch (value)
		{
			case fscp_configuration::HRP_IPV4:
				return boost::asio::ip::udp::v4();
			case fscp_configuration::HRP_IPV6:
				return boost::asio::ip::udp::v6();
		}

		assert(false);
		throw std::logic_error("Invalid hostname_resolution_protocol_type");
	}

	std::istream& operator>>(std::istream& is, fscp_configuration::hostname_resolution_protocol_type& v)
	{
		std::string value;

		is >> value;

		if (value == "ipv4")
			v = fscp_configuration::HRP_IPV4;
		else if (value == "ipv6")
			v = fscp_configuration::HRP_IPV6;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const fscp_configuration::hostname_resolution_protocol_type& value)
	{
		switch (value)
		{
			case fscp_configuration::HRP_IPV4:
				return os << "ipv4";
			case fscp_configuration::HRP_IPV6:
				return os << "ipv6";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, security_configuration::certificate_validation_method_type& v)
	{
		std::string value;

		is >> value;

		if (value == "default")
			v = security_configuration::CVM_DEFAULT;
		else if (value == "none")
			v = security_configuration::CVM_NONE;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const security_configuration::certificate_validation_method_type& value)
	{
		switch (value)
		{
			case security_configuration::CVM_DEFAULT:
				return os << "default";
			case security_configuration::CVM_NONE:
				return os << "none";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, security_configuration::certificate_revocation_validation_method_type& v)
	{
		std::string value;

		is >> value;

		if (value == "last")
			v = security_configuration::CRVM_LAST;
		else if (value == "all")
			v = security_configuration::CRVM_ALL;
		else if (value == "none")
			v = security_configuration::CRVM_NONE;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const security_configuration::certificate_revocation_validation_method_type& value)
	{
		switch (value)
		{
			case security_configuration::CRVM_LAST:
				return os << "last";
			case security_configuration::CRVM_ALL:
				return os << "all";
			case security_configuration::CRVM_NONE:
				return os << "none";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, tap_adapter_configuration::tap_adapter_type& v)
	{
		std::string value;

		is >> value;

		if (value == "tap")
			v = tap_adapter_configuration::tap_adapter_type::tap;
		else if (value == "tun")
			v = tap_adapter_configuration::tap_adapter_type::tun;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const tap_adapter_configuration::tap_adapter_type& value)
	{
		switch (value)
		{
			case tap_adapter_configuration::tap_adapter_type::tap:
				return os << "tap";
			case tap_adapter_configuration::tap_adapter_type::tun:
				return os << "tun";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, switch_configuration::routing_method_type& v)
	{
		std::string value;

		is >> value;

		if (value == "switch")
			v = switch_configuration::RM_SWITCH;
		else if (value == "hub")
			v = switch_configuration::RM_HUB;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const switch_configuration::routing_method_type& value)
	{
		switch (value)
		{
			case switch_configuration::RM_SWITCH:
				return os << "switch";
			case switch_configuration::RM_HUB:
				return os << "hub";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, router_configuration::internal_route_scope_type& v)
	{
		std::string value;

		is >> value;

		if (value == "none")
			v = router_configuration::internal_route_scope_type::none;
		else if (value == "unicast_in_network")
			v = router_configuration::internal_route_scope_type::unicast_in_network;
		else if (value == "unicast")
			v = router_configuration::internal_route_scope_type::unicast;
		else if (value == "subnet")
			v = router_configuration::internal_route_scope_type::subnet;
		else if (value == "any")
			v = router_configuration::internal_route_scope_type::any;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const router_configuration::internal_route_scope_type& value)
	{
		switch (value)
		{
			case router_configuration::internal_route_scope_type::none:
				return os << "none";
			case router_configuration::internal_route_scope_type::unicast_in_network:
				return os << "unicast_in_network";
			case router_configuration::internal_route_scope_type::unicast:
				return os << "unicast";
			case router_configuration::internal_route_scope_type::subnet:
				return os << "subnet";
			case router_configuration::internal_route_scope_type::any:
				return os << "any";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, router_configuration::system_route_scope_type& v)
	{
		std::string value;

		is >> value;

		if (value == "none")
			v = router_configuration::system_route_scope_type::none;
		else if (value == "unicast")
			v = router_configuration::system_route_scope_type::unicast;
		else if (value == "any")
			v = router_configuration::system_route_scope_type::any;
		else if (value == "unicast_with_gateway")
			v = router_configuration::system_route_scope_type::unicast_with_gateway;
		else if (value == "any_with_gateway")
			v = router_configuration::system_route_scope_type::any_with_gateway;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const router_configuration::system_route_scope_type& value)
	{
		switch (value)
		{
			case router_configuration::system_route_scope_type::none:
				return os << "none";
			case router_configuration::system_route_scope_type::unicast:
				return os << "unicast";
			case router_configuration::system_route_scope_type::any:
				return os << "any";
			case router_configuration::system_route_scope_type::unicast_with_gateway:
				return os << "unicast_with_gateway";
			case router_configuration::system_route_scope_type::any_with_gateway:
				return os << "any_with_gateway";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}

	std::istream& operator>>(std::istream& is, router_configuration::dns_servers_scope_type& v)
	{
		std::string value;

		is >> value;

		if (value == "none")
			v = router_configuration::dns_servers_scope_type::none;
		else if (value == "in_network")
			v = router_configuration::dns_servers_scope_type::in_network;
		else if (value == "any")
			v = router_configuration::dns_servers_scope_type::any;
		else
			throw boost::bad_lexical_cast();

		return is;
	}

	std::ostream& operator<<(std::ostream& os, const router_configuration::dns_servers_scope_type& value)
	{
		switch (value)
		{
		case router_configuration::dns_servers_scope_type::none:
			return os << "none";
		case router_configuration::dns_servers_scope_type::in_network:
			return os << "in_network";
		case router_configuration::dns_servers_scope_type::any:
			return os << "any";
		}

		assert(false);
		throw std::logic_error("Unexpected value");
	}
}

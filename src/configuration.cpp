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
	fscp_configuration::fscp_configuration() :
		listen_on(),
		contact_list(),
		hostname_resolution_protocol(HRP_IPV4),
		hello_timeout(boost::posix_time::seconds(3))
	{
	}

	security_configuration::security_configuration() :
		identity(),
		certificate_validation_method(CVM_DEFAULT),
		certificate_validation_callback(0),
		certificate_authority_list(),
		certificate_revocation_validation_method(CRVM_NONE),
		certificate_revocation_list_list()
	{
	}

	tap_adapter_configuration::tap_adapter_configuration() :
		enabled(true),
		ipv4_address_prefix_length(),
		ipv6_address_prefix_length(),
		arp_proxy_enabled(false),
		arp_proxy_fake_ethernet_address(),
		dhcp_proxy_enabled(false),
		dhcp_server_ipv4_address_prefix_length(),
		dhcp_server_ipv6_address_prefix_length(),
		up_callback(0),
		down_callback(0)
	{
	}

	switch_configuration::switch_configuration() :
		routing_method(RM_SWITCH),
		relay_mode_enabled(false)
	{
	}

	configuration::configuration() :
		fscp(),
		security(),
		tap_adapter(),
		switch_()
	{
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

	std::ostream& operator<<(std::ostream& os, fscp_configuration::hostname_resolution_protocol_type& value)
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

	std::ostream& operator<<(std::ostream& os, security_configuration::certificate_validation_method_type& value)
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

	std::ostream& operator<<(std::ostream& os, security_configuration::certificate_revocation_validation_method_type& value)
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

	std::ostream& operator<<(std::ostream& os, tap_adapter_configuration::ethernet_address_type value)
	{
		os<< std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(value[0]);

		for (size_t i = 1; i < value.size(); ++i)
		{
			os << ':' << static_cast<unsigned int>(value[1]);
		}

		return os;
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

	std::ostream& operator<<(std::ostream& os, switch_configuration::routing_method_type& value)
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
}

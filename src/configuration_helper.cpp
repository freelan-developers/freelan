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

#include <boost/asio.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "endpoint.hpp"
#include "endpoint_parser.hpp"

namespace po = boost::program_options;
namespace qi = boost::spirit::qi;
namespace ph = boost::phoenix;
namespace fl = freelan;

namespace
{
	const unsigned int DEFAULT_PORT = 12000;

	fl::configuration::hostname_resolution_protocol_type parse_network_hostname_resolution_protocol(const std::string& str)
	{
		if (str == "system_default")
			return fl::configuration::HRP_SYSTEM_DEFAULT;
		else if (str == "ipv4")
			return fl::configuration::HRP_IPV4;
		else if (str == "ipv6")
			return fl::configuration::HRP_IPV6;

		throw po::invalid_option_value(str);
	}

	boost::asio::ip::udp::resolver::query::protocol_type to_protocol_type(fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol)
	{
		switch (hostname_resolution_protocol)
		{
			case fl::configuration::HRP_SYSTEM_DEFAULT:
			case fl::configuration::HRP_IPV4:
				return boost::asio::ip::udp::v4();
			case fl::configuration::HRP_IPV6:
				return boost::asio::ip::udp::v6();
		}

		throw std::logic_error("invalid hostname_resolution_protocol");
	}

	fl::configuration::ep_type parse_endpoint(const std::string& str, fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol)
	{
		typedef boost::asio::ip::udp::resolver::query query;

		boost::shared_ptr<endpoint> ep;

		std::string::const_iterator first = str.begin();
		bool r = qi::phrase_parse(first, str.end(), custom_parser::endpoint[ph::ref(ep) = qi::_1], qi::space);

		if (r && (first == str.end()) && ep)
		{
			return ep->to_boost_asio_endpoint(to_protocol_type(hostname_resolution_protocol), query::address_configured | query::passive);
		}

		throw po::invalid_option_value(str);
	}
}

po::options_description get_network_options()
{
	po::options_description result("Network options");

	result.add_options()
		("network.hostname_resolution_protocol", po::value<std::string>()->default_value("system_default"), "The hostname resolution protocol to use.")
		("network.listen_on", po::value<std::string>()->default_value("0.0.0.0:12000"), "The endpoint to listen on.")
		("network.tap_adapter_addresses", po::value<std::string>()->multitoken()->default_value("9.0.0.1/24"), "The tap adapter network addresses.")
		("network.enable_dhcp_proxy", po::value<bool>()->default_value(true), "Whether to enable the DHCP proxy.")
		("network.enable_arp_proxy", po::value<bool>()->default_value(false), "Whether to enable the ARP proxy.")
		("network.routing_method", po::value<std::string>()->default_value("switch"), "The routing method for messages.")
		("network.hello_timeout", po::value<std::string>()->default_value("3000"), "The default hello message timeout, in milliseconds.")
		;

	return result;
}

po::options_description get_security_options()
{
	po::options_description result("Security options");

	result.add_options()
		("security.certificate_file", po::value<std::string>()->required(), "The certificate file to use.")
		("security.private_key_file", po::value<std::string>()->required(), "The private key file to use.")
		("security.certificate_validation_method", po::value<std::string>()->default_value("default"), "The certificate validation method.")
		("security.certificate_validation_script", po::value<std::string>(), "The certificate validation script to use.")
		("security.use_whitelist", po::value<bool>()->default_value(false), "Whether to use the whitelist.")
		("security.use_blacklist", po::value<bool>()->default_value(true), "Whether to use the blacklist.")
		("security.whitelist", po::value<std::string>()->multitoken()->composing(), "The whitelist.")
		("security.blacklist", po::value<std::string>()->multitoken()->composing(), "The blacklist.")
		;

	return result;
}

void setup_configuration(fl::configuration& configuration, const po::variables_map& vm)
{
	configuration.hostname_resolution_protocol = parse_network_hostname_resolution_protocol(vm["network.hostname_resolution_protocol"].as<std::string>());
	configuration.listen_on = parse_endpoint(vm["network.listen_on"].as<std::string>(), configuration.hostname_resolution_protocol);
}

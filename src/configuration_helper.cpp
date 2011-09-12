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
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "ipv4_address_parser.hpp"
#include "ipv6_address_parser.hpp"

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

	fl::configuration::ep_type parse_endpoint(const std::string& str, fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol)
	{
		namespace as = boost::asio;
		namespace ip = boost::asio::ip;
		using custom_parser::ipv4_address;
		using custom_parser::ipv6_address;

		typedef qi::uint_parser<uint16_t, 10, 1, 5> port_parser;

		boost::asio::ip::address_v4 address_v4;
		boost::asio::ip::address_v6 address_v6;
		unsigned int port = DEFAULT_PORT;
		std::string hostname;
		std::string service;

		bool r;
		std::string::const_iterator first;

		// Parse IPv4 addresses.
		first = str.begin();
		r = qi::parse(first, str.end(), ipv4_address[ph::ref(address_v4) = qi::_1] >> -(':' >> port_parser()[ph::ref(port) = qi::_1]), qi::space);

		if (r && (first == str.end()))
		{
			return fl::configuration::ep_type(address_v4, port);
		}

		// Parse IPv6 addresses.
		first = str.begin();
		r = qi::parse(first, str.end(), ipv6_address[ph::ref(address_v6) == qi::_1] | ('[' >> ipv6_address[ph::ref(address_v6) = qi::_1] >> ']' >> ':' >> port_parser()[ph::ref(port) = qi::_1]), qi::space);

		if (r && (first == str.end()))
		{
			return fl::configuration::ep_type(address_v6, port);
		}

		// Parse hostname notation.
		first = str.begin();
		//r = qi::parse(first, str.end(), (*(qi::repeat(1, 63)[qi::alnum | '-'] >> '.') >> qi::repeat(1, 63)[qi::alnum | '-']) >> -(':' >> (qi::repeat(1, 63)[qi::alnum])), qi::space);

		if (r && (first == str.end()))
		{
			as::io_service io_service;
			ip::udp::resolver resolver(io_service);
			ip::udp::resolver::query::protocol_type protocol_type = ip::udp::v4();

			switch (hostname_resolution_protocol)
			{
				case fl::configuration::HRP_SYSTEM_DEFAULT:
					protocol_type = ip::udp::v4();
					break;
				case fl::configuration::HRP_IPV4:
					protocol_type = ip::udp::v4();
					break;
				case fl::configuration::HRP_IPV6:
					protocol_type = ip::udp::v6();
					break;
			}

			ip::udp::resolver::query query(protocol_type, hostname, service, ip::udp::resolver::query::address_configured | ip::udp::resolver::query::passive);
			return *resolver.resolve(query);
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

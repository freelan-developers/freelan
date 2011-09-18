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
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "endpoint.hpp"
#include "endpoint_parser.hpp"
#include "ip_address_netmask_parser.hpp"

namespace po = boost::program_options;
namespace qi = boost::spirit::qi;
namespace ph = boost::phoenix;
namespace fl = freelan;

namespace
{
	const std::string DEFAULT_PORT = "12000";

	fl::configuration::hostname_resolution_protocol_type parse_network_hostname_resolution_protocol(const std::string& str)
	{
		if (str == "system_default")
			return boost::asio::ip::udp::v4();
		else if (str == "ipv4")
			return boost::asio::ip::udp::v4();
		else if (str == "ipv6")
			return boost::asio::ip::udp::v6();

		throw po::invalid_option_value(str);
	}

	fl::configuration::ep_type parse_endpoint(
			const std::string& str,
			fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol,
			boost::asio::ip::udp::resolver::query::flags flags
			)
	{
		boost::shared_ptr<endpoint> ep;

		std::string::const_iterator first = str.begin();
		bool r = qi::phrase_parse(first, str.end(), custom_parser::endpoint[ph::ref(ep) = qi::_1], qi::space);

		if (r && (first == str.end()) && ep)
		{
			try
			{
				return ep->to_boost_asio_endpoint(hostname_resolution_protocol, flags, DEFAULT_PORT);
			}
			catch (boost::system::system_error& se)
			{
				throw po::invalid_option_value(str + ": " + se.what());
			}
		}

		throw po::invalid_option_value(str);
	}

	std::vector<fl::configuration::ep_type> parse_endpoint_list(
			const std::string& str,
			fl::configuration::hostname_resolution_protocol_type hostname_resolution_protocol,
			boost::asio::ip::udp::resolver::query::flags flags
			)
	{
		typedef std::vector<boost::shared_ptr<endpoint> > list_type;
		
		list_type list;

		std::string::const_iterator first = str.begin();
		bool r = qi::phrase_parse(first, str.end(), *(custom_parser::endpoint[boost::bind(&list_type::push_back, &list, _1)]), qi::space);

		if (r && (first == str.end()))
		{
			typedef std::vector<fl::configuration::ep_type> result_type;

			result_type result;

			BOOST_FOREACH(boost::shared_ptr<endpoint>& ep, list)
			{
				try
				{
					result.push_back(ep->to_boost_asio_endpoint(hostname_resolution_protocol, flags, DEFAULT_PORT));
				}
				catch (boost::system::system_error&)
				{
				}
			}

			return result;
		}

		throw po::invalid_option_value(str);
	}

	std::vector<fl::configuration::ip_address_netmask_type> parse_ip_address_netmask_list(const std::string& str)
	{
		typedef std::vector<fl::configuration::ip_address_netmask_type> result_type;
		
		result_type result;

		std::string::const_iterator first = str.begin();
		bool r = qi::phrase_parse(first, str.end(), *(custom_parser::ip_address_netmask[boost::bind(&result_type::push_back, &result, _1)]), qi::space);

		if (r && (first == str.end()))
		{
			return result;
		}

		throw po::invalid_option_value(str);
	}

	fl::configuration::routing_method_type to_routing_method(const std::string& str)
	{
		if (str == "switch")
			return fl::configuration::RM_SWITCH;
		if (str == "hub")
			return fl::configuration::RM_HUB;

		throw po::invalid_option_value(str);
	}

	fl::configuration::certificate_validation_method_type to_certificate_validation_method(const std::string& str)
	{
		if (str == "default")
			return fl::configuration::CVM_DEFAULT;
		if (str == "none")
			return fl::configuration::CVM_NONE;

		throw po::invalid_option_value(str);
	}

	boost::posix_time::time_duration to_time_duration(unsigned int msduration)
	{
		return boost::posix_time::milliseconds(msduration);
	}

	cryptoplus::file load_file(const std::string& filename)
	{
		try
		{
			return cryptoplus::file::open(filename);
		}
		catch (std::runtime_error&)
		{
			throw std::runtime_error("Unable to open the specified file: " + filename);
		}
	}

	fl::configuration::cert_type load_certificate(const std::string& filename)
	{
		return fl::configuration::cert_type::from_certificate(load_file(filename));
	}

	cryptoplus::pkey::pkey load_private_key(const std::string& filename)
	{
		return cryptoplus::pkey::pkey::from_private_key(load_file(filename));
	}
}

po::options_description get_network_options()
{
	po::options_description result("Network options");

	result.add_options()
		("network.hostname_resolution_protocol", po::value<std::string>()->default_value("system_default"), "The hostname resolution protocol to use.")
		("network.listen_on", po::value<std::string>()->default_value("0.0.0.0:12000"), "The endpoint to listen on.")
		("network.tap_adapter_addresses", po::value<std::string>()->multitoken()->zero_tokens()->default_value("9.0.0.1/24"), "The tap adapter network addresses.")
		("network.enable_arp_proxy", po::value<bool>()->default_value(false), "Whether to enable the ARP proxy.")
		("network.enable_dhcp_proxy", po::value<bool>()->default_value(true), "Whether to enable the DHCP proxy.")
		("network.dhcp_server_ipv4_address", po::value<std::string>()->default_value("9.0.0.0"), "The DHCP proxy server IPv4 address.")
		("network.dhcp_server_ipv4_netmask", po::value<std::string>()->default_value("255.255.255.0"), "The DHCP proxy server IPv4 netmask.")
		("network.dhcp_client_ipv4_address", po::value<std::string>()->default_value("9.0.0.1"), "The DHCP proxy client IPv4 address.")
		("network.routing_method", po::value<std::string>()->default_value("switch"), "The routing method for messages.")
		("network.hello_timeout", po::value<std::string>()->default_value("3000"), "The default hello message timeout, in milliseconds.")
		("network.contact_list", po::value<std::string>()->multitoken()->zero_tokens(), "The contact list.")
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
		("security.whitelist", po::value<std::string>()->multitoken()->zero_tokens(), "The whitelist.")
		("security.blacklist", po::value<std::string>()->multitoken()->zero_tokens(), "The blacklist.")
		;

	return result;
}

void setup_configuration(fl::configuration& configuration, const po::variables_map& vm)
{
	typedef boost::asio::ip::udp::resolver::query query;
	typedef fl::configuration::cert_type cert_type;
	typedef cryptoplus::pkey::pkey pkey;

	configuration.hostname_resolution_protocol = parse_network_hostname_resolution_protocol(vm["network.hostname_resolution_protocol"].as<std::string>());
	configuration.listen_on = parse_endpoint(vm["network.listen_on"].as<std::string>(), configuration.hostname_resolution_protocol, query::address_configured | query::passive);
	configuration.tap_adapter_addresses = parse_ip_address_netmask_list(vm["network.tap_adapter_addresses"].as<std::string>());
	configuration.enable_arp_proxy = vm["network.enable_arp_proxy"].as<bool>();
	configuration.enable_dhcp_proxy = vm["network.enable_dhcp_proxy"].as<bool>();
	configuration.dhcp_server_ipv4_address = boost::asio::ip::address_v4::from_string(vm["network.dhcp_server_ipv4_address"].as<std::string>());
	configuration.dhcp_server_ipv4_netmask = boost::asio::ip::address_v4::from_string(vm["network.dhcp_server_ipv4_netmask"].as<std::string>());
	configuration.dhcp_client_ipv4_address = boost::asio::ip::address_v4::from_string(vm["network.dhcp_client_ipv4_address"].as<std::string>());
	configuration.routing_method = to_routing_method(vm["network.routing_method"].as<std::string>());
	configuration.hello_timeout = to_time_duration(boost::lexical_cast<unsigned int>(vm["network.hello_timeout"].as<std::string>()));
	configuration.contact_list = parse_endpoint_list(vm["network.contact_list"].as<std::string>(), configuration.hostname_resolution_protocol, query::address_configured);

	cert_type certificate = load_certificate(vm["security.certificate_file"].as<std::string>());
	pkey private_key = load_private_key(vm["security.private_key_file"].as<std::string>());

	configuration.identity = fscp::identity_store(certificate, private_key);
	configuration.certificate_validation_method = to_certificate_validation_method(vm["security.certificate_validation_method"].as<std::string>());
	//TODO: We need to create a callback
	//configuration.certificate_validation_script =
	configuration.use_whitelist = vm["security.use_whitelist"].as<bool>();
	configuration.whitelist = parse_endpoint_list(vm["security.whitelist"].as<std::string>(), configuration.hostname_resolution_protocol, query::address_configured);
	configuration.use_blacklist = vm["security.use_blacklist"].as<bool>();
	configuration.blacklist = parse_endpoint_list(vm["security.blacklist"].as<std::string>(), configuration.hostname_resolution_protocol, query::address_configured);
}

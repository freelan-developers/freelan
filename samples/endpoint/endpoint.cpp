/**
 * \file endpoint.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A endpoint test sample.
 */

#include <asiotap/types/endpoint.hpp>
#include <asiotap/types/ip_network_address.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

#include <boost/lexical_cast.hpp>

int main()
{
	try
	{
		asiotap::endpoint ep1 = asiotap::hostname_endpoint("localhost", "1234");
		asiotap::endpoint ep2 = asiotap::ipv4_endpoint(boost::asio::ip::address_v4::loopback(), 1234);
		asiotap::endpoint ep3 = asiotap::ipv6_endpoint(boost::asio::ip::address_v6::loopback(), 1234);
		asiotap::ipv4_network_address na1 = asiotap::ipv4_network_address(boost::asio::ip::address_v4::loopback(), 24);
		asiotap::ipv6_network_address na2 = asiotap::ipv6_network_address(boost::asio::ip::address_v6::loopback(), 64);
		asiotap::ipv4_network_address na3 = asiotap::ipv4_network_address(boost::asio::ip::address_v4::loopback());

		// Output
		std::cout << "ep1: " << ep1 << std::endl;
		std::cout << "ep2: " << ep2 << std::endl;
		std::cout << "ep3: " << ep3 << std::endl;
		std::cout << "na1: " << na1 << std::endl;
		std::cout << "na2: " << na2 << std::endl;
		std::cout << "na3: " << na3 << std::endl;

		// Comparison
		std::cout << "ep1 == ep2: " << (ep1 == ep2) << std::endl;
		std::cout << "ep1 != ep2: " << (ep1 != ep2) << std::endl;

		// Conversions
		std::cout << boost::lexical_cast<asiotap::endpoint>("127.0.0.1:45") << std::endl;
		std::cout << boost::lexical_cast<asiotap::endpoint>("[3::4:5ae]:78") << std::endl;
		std::cout << boost::lexical_cast<asiotap::endpoint>("some.host.com:service") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ipv4_network_address>("9.0.0.1/24") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ipv6_network_address>("fe80::1/10") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ip_network_address>("9.0.0.1/24") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ip_network_address>("fe80::1/10") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ip_network_address>("127.0.0.1") << std::endl;
		std::cout << boost::lexical_cast<asiotap::ip_network_address>("fe80::1") << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

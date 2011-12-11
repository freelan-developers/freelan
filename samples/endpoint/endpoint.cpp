/**
 * \file endpoint.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A endpoint test sample.
 */

#include <freelan/endpoint.hpp>
#include <freelan/ip_network_address.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

#include <boost/lexical_cast.hpp>

int main()
{
	try
	{
		freelan::endpoint ep1 = freelan::hostname_endpoint("localhost", "1234");
		freelan::endpoint ep2 = freelan::ipv4_endpoint(boost::asio::ip::address_v4::loopback(), 1234);
		freelan::endpoint ep3 = freelan::ipv6_endpoint(boost::asio::ip::address_v6::loopback(), 1234);
		freelan::ipv4_network_address na1 = freelan::ipv4_network_address(boost::asio::ip::address_v4::loopback(), 24);
		freelan::ipv6_network_address na2 = freelan::ipv6_network_address(boost::asio::ip::address_v6::loopback(), 64);

		// Output
		std::cout << "ep1: " << ep1 << std::endl;
		std::cout << "ep2: " << ep2 << std::endl;
		std::cout << "ep3: " << ep3 << std::endl;
		std::cout << "na1: " << na1 << std::endl;
		std::cout << "na2: " << na2 << std::endl;

		// Comparison
		std::cout << "ep1 == ep2: " << (ep1 == ep2) << std::endl;
		std::cout << "ep1 != ep2: " << (ep1 != ep2) << std::endl;

		// Conversions
		std::cout << boost::lexical_cast<freelan::endpoint>("127.1:45") << std::endl;
		std::cout << boost::lexical_cast<freelan::endpoint>("[3::4:5ae]:78") << std::endl;
		std::cout << boost::lexical_cast<freelan::endpoint>("some.host.com:service") << std::endl;
		std::cout << boost::lexical_cast<freelan::ipv4_network_address>("9.0.0.1/24") << std::endl;
		std::cout << boost::lexical_cast<freelan::ipv6_network_address>("fe80::1/10") << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

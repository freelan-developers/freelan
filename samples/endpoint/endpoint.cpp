/**
 * \file endpoint.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A endpoint test sample.
 */

#include <freelan/endpoint.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

int main()
{
	try
	{
		freelan::endpoint ep1 = freelan::hostname_endpoint("localhost", "1234");
		freelan::endpoint ep2 = freelan::ipv4_endpoint(boost::asio::ip::address_v4::loopback(), 1234);
		freelan::endpoint ep3 = freelan::ipv6_endpoint(boost::asio::ip::address_v6::loopback(), 1234);

		// Output
		std::cout << "ep1: " << ep1 << std::endl;
		std::cout << "ep2: " << ep2 << std::endl;
		std::cout << "ep3: " << ep3 << std::endl;

		// Comparison
		std::cout << "ep1 == ep2: " << (ep1 == ep2) << std::endl;
		std::cout << "ep1 != ep2: " << (ep1 != ep2) << std::endl;

		// Input
		std::cout << "ep1: " << std::flush;
		std::cin >> ep1;

		std::cout << "ep1: " << ep1 << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

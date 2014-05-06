/**
 * \file routing.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Test the netlink routing functions.
 */

#include <boost/asio.hpp>

#include <iostream>

#include <netlinkplus/manager.hpp>

int main()
{
	try
	{
		boost::asio::io_service io_service;

		netlinkplus::manager manager(io_service);

		const auto host = boost::asio::ip::address_v4::from_string("8.8.8.8");
		const auto route_info = manager.get_route_for(host);

		std::cout << "Route information to " << host << std::endl;
		std::cout << "Destination: " << route_info.destination << std::endl;
		std::cout << "Source: " << route_info.source << std::endl;
		std::cout << "Input interface: " << route_info.input_interface << std::endl;
		std::cout << "Output interface: " << route_info.output_interface << std::endl;
		std::cout << "Gateway: " << (route_info.gateway ? route_info.gateway->to_string() : "<none>") << std::endl;
		std::cout << "Priority: " << route_info.priority << std::endl;
		std::cout << "Metric: " << route_info.metric << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

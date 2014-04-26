/**
 * \file system.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Test some system calls.
 */

#include <asiotap/os.hpp>

#ifdef LINUX
#include <asiotap/linux/netlink.hpp>

#include <net/if.h>
#endif

int main()
{
	try
	{
#ifdef LINUX
		const auto host = boost::asio::ip::address_v4::from_string("8.8.8.8");
		const auto route_info = asiotap::netlink::get_route_for(host);
		char ifname_buf[IF_NAMESIZE];

		std::cout << "Route information to " << host << std::endl;
		std::cout << "Interface: " << ::if_indextoname(route_info.interface, ifname_buf) << std::endl;
		std::cout << "Gateway: " << (route_info.gateway ? route_info.gateway->to_string() : "<none>") << std::endl;
#endif
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

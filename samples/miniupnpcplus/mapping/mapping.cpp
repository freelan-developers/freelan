/**
 * \file mapping.cpp
 * \author Sebastien Vincent <sebastien.vincent@freelan.org>
 * \brief Test the UPnP functions.
 */

#include <iostream>

#include <miniupnpcplus/upnp_device.hpp>

int main(int argc, char** argv)
{
	static_cast<void>(argc);
	static_cast<void>(argv);

	try
	{
		miniupnpcplus::upnp_device dev(2000);
		std::string tmp;

		std::cout << "UPnP device created" << std::endl;

		dev.register_port_mapping(miniupnpcplus::TCP, "12000", "12000",
				"Test miniupnpcplus");

		std::cout << "UPnP port mapping registered" << std::endl;

		std::cout << "Press any key to continue..." << std::endl;
		tmp = std::cin.get();

		dev.unregister_port_mapping(miniupnpcplus::TCP, "12000");
		std::cout << "UPnP port mapping unregistered" << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception during UPnP: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


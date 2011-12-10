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
		freelan::hostname_endpoint he;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

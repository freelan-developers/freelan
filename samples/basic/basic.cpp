/**
 * \file basic.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A basic iconv sample.
 */

#include <iconvplus/iconvplus.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
	try
	{
		iconvplus::iconv converter("utf-8", "latin1");
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

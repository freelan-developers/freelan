/**
 * \file basic.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A basic iconv sample.
 */

#include <iconvplus/iconvplus.hpp>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

int main()
{
	try
	{
		std::ifstream is("material/latin1.txt");
		iconvplus::converter cv(is, std::cout);
		iconvplus::iconv ic("cp1252", "latin1");
		cv.convert(ic);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

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
		iconvplus::iconv windows_ic("cp1252", "latin1");
		iconvplus::iconv unix_ic("utf-8", "latin1");

		std::ifstream is("material/latin1.txt");
		iconvplus::converter cv(is, std::cout);

		cv.convert(unix_ic);

 		is.clear();
		is.seekg(0);
		cv.convert(windows_ic);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

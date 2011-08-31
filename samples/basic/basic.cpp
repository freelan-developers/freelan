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
		iconvplus::iconv_instance windows_ic("cp1252", "latin1");
		iconvplus::iconv_instance unix_ic("utf-8", "latin1");

		std::ifstream is("material/latin1.txt");

		iconvplus::converter cv;
		
		cv.convert(unix_ic, is, std::cout);

 		is.clear();
		is.seekg(0);
		cv.convert(windows_ic, is, std::cout);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

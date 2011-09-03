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
		iconvplus::iconv_instance latin1_to_cp1252("cp1252", "latin1");
		iconvplus::iconv_instance latin1_to_utf8("utf-8", "latin1");
		iconvplus::iconv_instance utf8_to_cp1252("cp1252", "utf-8");
		iconvplus::iconv_instance utf8_to_latin1("latin1", "utf-8");

		std::ifstream latin1("material/latin1.txt");
		std::ifstream utf8("material/utf8.txt");

		iconvplus::converter cv;
		
		cv.convert(latin1_to_utf8, latin1, std::wcout);

 		latin1.clear();
		latin1.seekg(0);
		cv.convert(latin1_to_cp1252, latin1, std::wcout);

		cv.convert(utf8_to_latin1, utf8, std::wcout);

 		utf8.clear();
		utf8.seekg(0);
		cv.convert(utf8_to_cp1252, utf8, std::wcout);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

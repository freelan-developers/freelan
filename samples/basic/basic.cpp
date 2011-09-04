/**
 * \file basic.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A basic iconv sample.
 */

#include <iconvplus/os.hpp>
#include <iconvplus/iconvplus.hpp>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

int main()
{
	try
	{
		std::ifstream latin1_file("material/latin1.txt");
		std::ifstream utf8_file("material/utf-8.txt");

#ifdef WINDOWS
		iconvplus::iconv_instance latin1_to_cp1252("cp1252", "latin1");
		iconvplus::iconv_instance utf8_to_wchar_t("wchar_t", "utf-8");

		iconvplus::converter<char, char>(latin1_to_cp1252).convert(latin1_file, std::cout);
		iconvplus::converter<char, wchar_t>(utf8_to_wchar_t).convert(utf8_file, std::wcout);
#elif defined(UNIX)
		iconvplus::iconv_instance latin1_to_utf8("utf-8", "latin1");
		iconvplus::iconv_instance utf8_to_wchar_t("wchar_t", "utf-8");

		iconvplus::converter<char, char>(latin1_to_utf8).convert(latin1_file, std::cout);
		iconvplus::converter<char, wchar_t>(utf8_to_wchar_t).convert(utf8_file, std::wcout);
#endif
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

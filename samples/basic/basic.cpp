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
		iconvplus::iconv converter("cp1252", "latin1");

		std::ifstream ifs("material/latin1.txt");

		if (ifs)
		{
			char buf[1024] = {};
			char result_buf[1024] = {};

			while (ifs.getline(buf, sizeof(buf)))
			{
				size_t result_len = converter.convert_all(buf, strlen(buf), result_buf, sizeof(result_buf));

				std::cout << std::string(result_buf, result_len) << std::endl;
			}
		} else
		{
			std::cerr << "Unable to open the specified file." << std::endl;
		}
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception caught: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

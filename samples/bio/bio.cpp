/**
 * \file bio.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A bio sample file.
 */

#include <cryptopen/bio/bio_chain.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;

	std::cout << "BIO sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	return EXIT_SUCCESS;
}

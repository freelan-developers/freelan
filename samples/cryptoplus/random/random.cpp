/**
 * \file random.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A random sample file.
 */

#include <cryptoplus/buffer.hpp>
#include <cryptoplus/random/random.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef MSV
#include <openssl/applink.c>
#endif

using cryptoplus::buffer;

int main()
{
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Random sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	buffer bytes = cryptoplus::random::get_random_bytes(16);

	std::cout << "Random bytes: " << bytes << std::endl;

	return EXIT_SUCCESS;
}

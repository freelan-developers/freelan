/**
 * \file random.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A random sample file.
 */

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

template <typename T>
std::string to_hex(const T& begin, const T& end)
{
	std::ostringstream oss;

	for (T i = begin; i != end; ++i)
	{
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(static_cast<unsigned char>(*i));
	}

	return oss.str();
}

int main()
{
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Random sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	std::string bytes = cryptoplus::random::get_random_bytes(16);

	std::cout << "Random bytes: " << to_hex(bytes.begin(), bytes.end()) << std::endl;

	return EXIT_SUCCESS;
}

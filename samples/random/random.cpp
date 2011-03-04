/**
 * \file random.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A random sample file.
 */

#include <cryptopen/random/random.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

template <typename T>
std::string to_hex(const T& begin, const T& end)
{
	std::ostringstream oss;

	for (T i = begin; i != end; ++i)
	{
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*i);
	}

	return oss.str();
}

std::string to_hex(const void* buf, size_t buf_len)
{
	return to_hex(static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf) + buf_len);
}

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;

	std::cout << "Random sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;


	return EXIT_SUCCESS;
}

/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/store.hpp>
#include <cryptoplus/x509/store_context.hpp>
#include <cryptoplus/x509/verify_param.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifdef MSV
#include <openssl/applink.c>
#endif

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Store sample" << std::endl;
	std::cout << "============" << std::endl;
	std::cout << std::endl;

	try
	{
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

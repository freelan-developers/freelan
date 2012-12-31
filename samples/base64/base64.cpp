/**
 * \file base64.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A base64 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/base64.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>

#include <cstdio>

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifdef MSV
#include <openssl/applink.c>
#endif

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "BASE64 sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	const std::string original_string = "libcryptoplus is great !";
	const std::string encoded_string = cryptoplus::base64_encode(cryptoplus::buffer(original_string));
	const std::string decoded_string = cryptoplus::base64_decode(encoded_string).to_string();

	std::cout << "Original string: " << original_string << std::endl;
	std::cout << "Base64 encoded string: " << encoded_string << std::endl;
	std::cout << "Base64 decoded string: " << decoded_string << std::endl;

	return EXIT_SUCCESS;
}

/**
 * \file pbkdf2.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A PBKDF2 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/hash/pbkdf2.hpp>
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

void pbkdf2(const std::string& name, const std::string& password, const std::string& salt, unsigned int iterations)
{
	try
	{
		cryptoplus::hash::message_digest_algorithm algorithm(name);

		const buffer key = cryptoplus::hash::pbkdf2(password.c_str(), password.size(), salt.c_str(), salt.size(), algorithm, iterations);
		std::cout << name << ": " << key << std::endl;
	}
	catch (std::invalid_argument&)
	{
		std::cerr << name << ": " << "Unsupported algorithm" << std::endl;
	}
	catch (const boost::system::system_error& ex)
	{
		std::cerr << name << ": " << ex.what() << std::endl;
	}
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "PBKDF2 sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	const std::string password = "this is a very long and secret key";
	const std::string salt = "a salt value";
	const unsigned int iterations = 1000;

	std::cout << "Password: " << password << std::endl;
	std::cout << "Salt: " << salt << std::endl;
	std::cout << "Iterations: " << iterations << std::endl;
	std::cout << std::endl;

#if OPENSSL_VERSION_NUMBER >= 0x01000000
	pbkdf2("MD5", password, salt, iterations);
	pbkdf2("MD4", password, salt, iterations);
#endif
	pbkdf2("SHA1", password, salt, iterations);
#if OPENSSL_VERSION_NUMBER >= 0x01000000
	pbkdf2("SHA", password, salt, iterations);
	pbkdf2("SHA224", password, salt, iterations);
	pbkdf2("SHA256", password, salt, iterations);
	pbkdf2("SHA384", password, salt, iterations);
	pbkdf2("SHA512", password, salt, iterations);
	pbkdf2("MDC2", password, salt, iterations);
	pbkdf2("whirlpool", password, salt, iterations);
	pbkdf2("RIPEMD160", password, salt, iterations);
#endif

	return EXIT_SUCCESS;
}

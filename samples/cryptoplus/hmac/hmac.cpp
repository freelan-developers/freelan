/**
 * \file hmac.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A HMAC sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/hash/hmac_context.hpp>
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

void hmac(const std::string& name, const std::string& key, const std::string& data)
{
	try
	{
		cryptoplus::hash::message_digest_algorithm algorithm(name);

		cryptoplus::hash::hmac_context ctx;

		ctx.initialize(key.c_str(), key.size(), &algorithm);
		ctx.update(data.c_str(), data.size());
		buffer hmac = ctx.finalize();
		std::cout << name << ": " << hmac << std::endl;
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

	std::cout << "HMAC sample" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << std::endl;

	const std::string key = "this is a very long and secret key";
	const std::string data = "some data from which we will compute the HMAC";

	std::cout << "Key: " << key << std::endl;
	std::cout << "Data: " << data << std::endl;
	std::cout << std::endl;

	hmac("MD5", key, data);
	hmac("MD4", key, data);
	hmac("SHA1", key, data);
	hmac("SHA", key, data);
	hmac("SHA224", key, data);
	hmac("SHA256", key, data);
	hmac("SHA384", key, data);
	hmac("SHA512", key, data);
#if OPENSSL_VERSION_NUMBER >= 0x01000000
	hmac("MDC2", key, data);
	hmac("whirlpool", key, data);
#endif
	hmac("RIPEMD160", key, data);

	return EXIT_SUCCESS;
}

/**
 * \file hmac.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A HMAC sample file.
 */

#include <cryptopen/hash/hmac_context.hpp>
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

void hmac(const std::string& name, const std::string& key, const std::string& data)
{
	try
	{
		cryptopen::hash::message_digest_algorithm algorithm(name);

		cryptopen::hash::hmac_context ctx;

		ctx.initialize(key.c_str(), key.size(), &algorithm);
		ctx.update(data.c_str(), data.size());
		std::vector<unsigned char> hmac = ctx.finalize<unsigned char>();
		std::cout << name << ": " << to_hex(hmac.begin(), hmac.end()) << std::endl;
	}
	catch (cryptopen::error::cryptographic_exception& ex)
	{
		std::cerr << name << ": " << ex.what() << std::endl;
	}
}

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;
	cryptopen::hash::message_digest_initializer message_digest_initializer;

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

/**
 * \file signature.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A message digest signature sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/pkey/pkey.hpp>
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
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Message digest signature sample" << std::endl;
	std::cout << "===============================" << std::endl;
	std::cout << std::endl;

	const std::string data = "some data from which we will compute the message digest";

	try
	{
		std::cout << "Generating RSA key. This can take some time..." << std::endl;

		cryptoplus::pkey::rsa_key rsa_key = cryptoplus::pkey::rsa_key::generate_private_key(1024, 17);
		cryptoplus::pkey::pkey pkey = cryptoplus::pkey::pkey::create();

		pkey.set_rsa_key(rsa_key);

		std::cout << "Data: " << data << std::endl;
		std::cout << std::endl;

		cryptoplus::hash::message_digest_algorithm algorithm("SHA256");
		cryptoplus::hash::message_digest_context ctx;

		ctx.sign_initialize(algorithm);
		ctx.sign_update(data.c_str(), data.size());
		std::vector<unsigned char> signature = ctx.sign_finalize<unsigned char>(pkey);

		std::cout << "Signature: " << to_hex(signature.begin(), signature.end()) << std::endl;

		cryptoplus::hash::message_digest_context ctx2;
		ctx2.verify_initialize(algorithm);
		ctx2.verify_update(data.c_str(), data.size());
		bool verification = ctx2.verify_finalize(&signature[0], signature.size(), pkey);

		std::cout << "Verification: " << (verification ? "OK" : "FAILED") << std::endl;
	}
	catch (cryptoplus::error::cryptographic_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

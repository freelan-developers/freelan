/**
 * \file signature.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A message digest signature sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
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

using cryptoplus::buffer;

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
		buffer signature = ctx.sign_finalize(pkey);

		std::cout << "Signature: " << signature << std::endl;

		cryptoplus::hash::message_digest_context ctx2;
		ctx2.verify_initialize(algorithm);
		ctx2.verify_update(data.c_str(), data.size());
		bool verification = ctx2.verify_finalize(signature, pkey);

		std::cout << "Verification: " << (verification ? "OK" : "FAILED") << std::endl;
	}
	catch (const boost::system::system_error& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

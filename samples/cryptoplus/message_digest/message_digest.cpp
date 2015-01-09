/**
 * \file message_digest.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A message digest sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
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

void message_digest(const std::string& name, const std::string& data)
{
	try
	{
		cryptoplus::hash::message_digest_algorithm algorithm(name);

		cryptoplus::hash::message_digest_context ctx;

		ctx.initialize(algorithm);
		ctx.update(data.c_str(), data.size());
		const buffer message_digest = ctx.finalize();

		std::cout << name << ": " << message_digest << std::endl;
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

	std::cout << "Message digest sample" << std::endl;
	std::cout << "=====================" << std::endl;
	std::cout << std::endl;

	const std::string data = "some data from which we will compute the message digest";

	std::cout << "Data: " << data << std::endl;
	std::cout << std::endl;

	message_digest("MD5", data);
	message_digest("MD4", data);
	message_digest("SHA1", data);
	message_digest("SHA", data);
	message_digest("SHA224", data);
	message_digest("SHA256", data);
	message_digest("SHA384", data);
	message_digest("SHA512", data);
#if OPENSSL_VERSION_NUMBER >= 0x01000000
	message_digest("MDC2", data);
	message_digest("whirlpool", data);
#endif
	message_digest("RIPEMD160", data);

	return EXIT_SUCCESS;
}

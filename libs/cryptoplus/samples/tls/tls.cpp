/**
 * \file tls.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A TLS sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/tls/tls.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef MSV
#include <openssl/applink.c>
#endif

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "TLS sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	const std::string key = "this is a very long and secret key";
	const std::string label = "some label";
	const std::string seed = "some seed";

	std::cout << "key: " << key << std::endl;
	std::cout << "label: " << label << std::endl;
	std::cout << "seed: " << seed << std::endl;
	std::cout << std::endl;

	const std::vector<std::string> algorithms {
		"MD5",
		"MD4",
		"SHA1",
		"SHA",
		"SHA224",
		"SHA256",
		"SHA384",
		"SHA512",
#if OPENSSL_VERSION_NUMBER >= 0x01000000
		"whirlpool",
#endif
		"RIPEMD160"
	};

	for (auto&& algorithm: algorithms)
	{
		const auto alg = cryptoplus::hash::message_digest_algorithm(algorithm);

		std::cout << "-- " << algorithm << " --" << std::endl;

		for (size_t size = 2; size < 256; size <<= 1)
		{
			try
			{
				const auto result = cryptoplus::tls::prf(size, key.c_str(), key.size(), label, seed.c_str(), seed.size(), alg);

				std::cout << std::setw(3) << std::setfill(' ') << buffer_size(result) << " bytes - " << result << std::endl;
			}
			catch (std::exception& ex)
			{
				std::cout << std::endl;
				std::cerr << "Exception: " << ex.what() << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}

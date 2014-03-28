/**
 * \file dh_key.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A DH key sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/pkey/dh_key.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <cstdio>

#ifdef MSV
#include <openssl/applink.c>
#endif

using cryptoplus::buffer;

namespace
{
	int pem_passphrase_callback(char* buf, int buf_len, int rwflag, void*)
	{
		std::cout << "Passphrase (max: " << buf_len << " characters): " << std::flush;
		std::string passphrase;
		std::getline(std::cin, passphrase);

		if (passphrase.empty())
		{
			std::cerr << "Passphrase cannot be empty." << std::endl;
			return 0;
		}

		if (passphrase.size() > static_cast<size_t>(buf_len))
		{
			std::cerr << "Passphrase cannot exceed " << buf_len << " characters." << std::endl;
			return 0;
		}

		if (rwflag != 0)
		{
			std::cout << "Confirm: " << std::flush;
			std::string passphrase_confirmation;
			std::getline(std::cin, passphrase_confirmation);

			if (passphrase_confirmation != passphrase)
			{
				std::cerr << "The two passphrases do not match !" << std::endl;
				return 0;
			}
		}

		std::copy(passphrase.begin(), passphrase.end(), buf);
		return passphrase.size();
	}
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "DH sample" << std::endl;
	std::cout << "=========" << std::endl;
	std::cout << std::endl;

	const int bits = 1024;
	const int generator = 2;

	std::cout << "Using DH keys of " << bits << " bits." << std::endl;

	const std::string parameters_filename = "parameters.pem";

	boost::shared_ptr<FILE> parameters_file(fopen(parameters_filename.c_str(), "w"), fclose);

	if (!parameters_file)
	{
		std::cerr << "Unable to open \"" << parameters_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Generating DH parameters. This can take some time..." << std::endl;

		cryptoplus::pkey::dh_key dh_key = cryptoplus::pkey::dh_key::generate_parameters(bits, generator);

		int codes = 0;

		dh_key.check(codes);

		if (codes != 0)
		{
			std::cerr << "Generation failed." << std::endl;

			if (codes & DH_CHECK_P_NOT_SAFE_PRIME)
			{
				std::cerr << "p is not a safe prime." << std::endl;
			}
			if (codes & DH_NOT_SUITABLE_GENERATOR)
			{
				std::cerr << "g is not a suitable generator." << std::endl;
			}

			if (codes & DH_UNABLE_TO_CHECK_GENERATOR)
			{
				std::cerr << "g is not a correct generator. Must be either 2 or 5." << std::endl;
			}

			return EXIT_FAILURE;
		}

		dh_key.write_parameters(parameters_file.get());

		std::cout << "DH parameters written succesfully to \"" << parameters_filename << "\"." << std::endl;
		std::cout << "Done." << std::endl;

		std::cout << "Generating DH key..." << std::endl;

		dh_key.generate_key();

		std::cout << "Done." << std::endl;

		parameters_file.reset(fopen(parameters_filename.c_str(), "r"), fclose);

		if (!parameters_file)
		{
			std::cerr << "Unable to open \"" << parameters_filename << "\" for reading." << std::endl;

			return EXIT_FAILURE;
		}

		std::cout << "Trying to read back the DH parameters from \"" << parameters_filename << "\"..." << std::endl;

		cryptoplus::pkey::dh_key dh_key2 = cryptoplus::pkey::dh_key::from_parameters(parameters_file.get(), pem_passphrase_callback);

		std::cout << "Done." << std::endl;

		std::cout << "Generating DH key..." << std::endl;

		dh_key2.generate_key();

		std::cout << "Done." << std::endl;

		std::cout << "Public key A: " << dh_key.public_key().to_dec() << std::endl;
		std::cout << "Public key B: " << dh_key2.public_key().to_dec() << std::endl;

		std::cout << "Computing key A..." << std::endl;

		buffer key_a = dh_key.compute_key(dh_key2.public_key());
		
		std::cout << "Done." << std::endl;

		std::cout << "Computing key B..." << std::endl;

		buffer key_b = dh_key2.compute_key(dh_key.public_key());
		
		std::cout << "Done." << std::endl;

		std::cout << "Comparing key A and key B: " << ((key_a == key_b) ? "IDENTICAL" : "DIFFERENT") << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

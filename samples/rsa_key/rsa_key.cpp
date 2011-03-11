/**
 * \file rsa_key.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A RSA sample file.
 */

#include <cryptopen/pkey/rsa_key.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <cstdio>

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
	cryptopen::error::error_strings_initializer error_strings_initializer;
	cryptopen::cipher::cipher_initializer cipher_initializer;

	std::cout << "RSA sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	const std::string filename = "rsa_key.pem";
	FILE* pfile = fopen(filename.c_str(), "w");

	if (!pfile)
	{
		std::cerr << "Unable to open \"" << filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Generating RSA key. This can take some time..." << std::endl;

		cryptopen::pkey::rsa_key rsa_key = cryptopen::pkey::rsa_key::generate_private_key(1024, 17);

		std::cout << "Done." << std::endl;

		rsa_key.write_private_key(pfile, cryptopen::cipher::cipher_algorithm("AES256"), pem_passphrase_callback);

		std::cout << "RSA key written succesfully to \"" << filename << "\"." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	fclose(pfile);

	pfile = fopen(filename.c_str(), "r");

	if (!pfile)
	{
		std::cerr << "Unable to open \"" << filename << "\" for reading." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Trying to read back the private RSA key from \"" << filename << "\"..." << std::endl;

		cryptopen::pkey::rsa_key rsa_key = cryptopen::pkey::rsa_key::from_private_key(pfile, pem_passphrase_callback);

		std::cout << "RSA key read succesfully from \"" << filename << "\"." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

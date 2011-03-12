/**
 * \file dh.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A DH sample file.
 */

#include <cryptopen/pkey/dh.hpp>
#include <cryptopen/hash/message_digest_context.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <boost/shared_ptr.hpp>

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

	std::cout << "DH sample" << std::endl;
	std::cout << "=========" << std::endl;
	std::cout << std::endl;

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

		cryptopen::pkey::dh dh = cryptopen::pkey::dh::generate_parameters(1024, NULL, 0, NULL, NULL);

		std::cout << "Done." << std::endl;

		dsa_key.write_private_key(private_key_file.get(), cryptopen::cipher::cipher_algorithm("AES256"), pem_passphrase_callback);

		std::cout << "Private DSA key written succesfully to \"" << private_key_filename << "\"." << std::endl;

		dsa_key.write_parameters(parameters_file.get());

		std::cout << "DSA parameters written succesfully to \"" << parameters_filename << "\"." << std::endl;

		dsa_key.write_certificate_public_key(certificate_public_key_file.get());

		std::cout << "Certificate public DSA key written succesfully to \"" << certificate_public_key_filename << "\"." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	certificate_public_key_file.reset();
	parameters_file.reset();
	private_key_file.reset(fopen(private_key_filename.c_str(), "r"), fclose);

	if (!private_key_file)
	{
		std::cerr << "Unable to open \"" << private_key_filename << "\" for reading." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Trying to read back the private DSA key from \"" << private_key_filename << "\"..." << std::endl;

		cryptopen::pkey::dsa_key dsa_key = cryptopen::pkey::dsa_key::from_private_key(private_key_file.get(), pem_passphrase_callback);

		std::cout << "Done." << std::endl;

		dsa_key.print(BIO_new_fd(fileno(stdout), BIO_NOCLOSE));

		const std::string str = "Hello World !";
		const std::string hash = "SHA256";

		std::cout << "Generating " << hash << " message digest for \"" << str << "\"..." << std::endl;

		cryptopen::hash::message_digest_algorithm algorithm(hash);
		cryptopen::hash::message_digest_context context;
		context.initialize(algorithm);
		context.update(str.c_str(), str.size());
		std::vector<unsigned char> str_hash = context.finalize<unsigned char>();

		std::cout << "Done." << std::endl;

		std::cout << "Generating DSA signature..." << std::endl;

		std::vector<unsigned char> str_sign = dsa_key.sign<unsigned char>(&str_hash[0], str_hash.size(), algorithm.type());

		std::cout << "Done." << std::endl;

		std::cout << "Verifying DSA signature..." << std::endl;

		dsa_key.verify(&str_sign[0], str_sign.size(), &str_hash[0], str_hash.size(), algorithm.type());

		std::cout << "Done." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

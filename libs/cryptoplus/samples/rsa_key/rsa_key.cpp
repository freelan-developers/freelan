/**
 * \file rsa_key.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A RSA sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/bio/bio_chain.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <cstdio>

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

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

	std::cout << "RSA sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	const std::string private_key_filename = "private_key.pem";
	const std::string public_key_filename = "public_key.pem";
	const std::string certificate_public_key_filename = "certificate_public_key.pem";

	boost::shared_ptr<FILE> private_key_file(fopen(private_key_filename.c_str(), "w"), fclose);
	boost::shared_ptr<FILE> public_key_file(fopen(public_key_filename.c_str(), "w"), fclose);
	boost::shared_ptr<FILE> certificate_public_key_file(fopen(certificate_public_key_filename.c_str(), "w"), fclose);

	if (!private_key_file)
	{
		std::cerr << "Unable to open \"" << private_key_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	if (!public_key_file)
	{
		std::cerr << "Unable to open \"" << public_key_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	if (!certificate_public_key_file)
	{
		std::cerr << "Unable to open \"" << certificate_public_key_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Generating RSA key. This can take some time..." << std::endl;

		cryptoplus::pkey::rsa_key rsa_key = cryptoplus::pkey::rsa_key::generate_private_key(1024, 17);

		std::cout << "Done." << std::endl;

		rsa_key.write_private_key(private_key_file.get(), cryptoplus::cipher::cipher_algorithm("AES256"), pem_passphrase_callback);

		std::cout << "Private RSA key written succesfully to \"" << private_key_filename << "\"." << std::endl;

		rsa_key.write_public_key(public_key_file.get());

		std::cout << "Public RSA key written succesfully to \"" << public_key_filename << "\"." << std::endl;

		rsa_key.write_certificate_public_key(certificate_public_key_file.get());

		std::cout << "Certificate public RSA key written succesfully to \"" << certificate_public_key_filename << "\"." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	certificate_public_key_file.reset();
	public_key_file.reset();
	private_key_file.reset(fopen(private_key_filename.c_str(), "r"), fclose);

	if (!private_key_file)
	{
		std::cerr << "Unable to open \"" << private_key_filename << "\" for reading." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Trying to read back the private RSA key from \"" << private_key_filename << "\"..." << std::endl;

		cryptoplus::pkey::rsa_key rsa_key = cryptoplus::pkey::rsa_key::from_private_key(private_key_file.get(), pem_passphrase_callback);

		std::cout << "Done." << std::endl;

		cryptoplus::bio::bio_chain bio_chain(BIO_new_fd(STDOUT_FILENO, BIO_NOCLOSE));
		rsa_key.print(bio_chain.first());

		const std::string str = "Hello World !";
		const std::string hash = "SHA256";

		std::cout << "Generating " << hash << " message digest for \"" << str << "\"..." << std::endl;

		cryptoplus::hash::message_digest_algorithm algorithm(hash);
		cryptoplus::hash::message_digest_context context;
		context.initialize(algorithm);
		context.update(str.c_str(), str.size());
    const buffer str_hash = context.finalize();

		std::cout << "Done." << std::endl;

		std::cout << "Generating RSA signature..." << std::endl;

    buffer str_sign = rsa_key.sign(str_hash, algorithm.type());

		std::cout << "Done." << std::endl;

		std::cout << "Verifying RSA signature..." << std::endl;

		rsa_key.verify(str_sign, str_hash, algorithm.type());

		std::cout << "Done." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

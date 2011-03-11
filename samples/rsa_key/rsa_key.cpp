/**
 * \file rsa_key.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A RSA sample file.
 */

#include <cryptopen/pkey/rsa_key.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <iostream>
#include <string>

namespace
{
	int pem_passphrase_callback(char* buf, int buf_len, int rwflag, void* arg)
	{
		return 1;
	}
}

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;

	std::cout << "RSA sample" << std::endl;
	std::cout << "==========" << std::endl;
	std::cout << std::endl;

	cryptopen::pkey::rsa_key rsa_key = cryptopen::rsa_key::generate(4096, 17);

	cryptopen::bio::bio_chain bio_chain(BIO_f_base64());
	bio_chain.first().push(BIO_new_fd(fileno(stdout), BIO_NOCLOSE));

	return EXIT_SUCCESS;
}

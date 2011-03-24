/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/bio/bio_chain.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

int main()
{
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "X509 sample" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << std::endl;

	try
	{
		cryptoplus::x509::certificate certificate;

		const char cn[] = "My common name";
		certificate.subject().push_back("CN", MBSTRING_ASC, cn, sizeof(cn) - 1);

		cryptoplus::bio::bio_chain bio_chain(BIO_new_fd(STDOUT_FILENO, BIO_NOCLOSE));
		certificate.print(bio_chain.first());
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

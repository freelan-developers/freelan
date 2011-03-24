/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
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
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "X509 sample" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << std::endl;

	try
	{
		cryptoplus::x509::certificate certificate;

		const char cn[] = "My common name";
		const char c[] = "FR";
		const char o[] = "My organization";

		certificate.subject().push_back("CN", MBSTRING_ASC, cn, sizeof(cn) - 1);
		certificate.subject().push_back("C", MBSTRING_ASC, c, sizeof(c) - 1);
		certificate.subject().push_back("O", MBSTRING_ASC, o, sizeof(o) - 1);

		std::cout << "Copying data to the issuer field..." << std::endl;

		certificate.issuer().insert(certificate.issuer().begin(), certificate.subject().begin(), certificate.subject().end());

		cryptoplus::bio::bio_chain bio_chain(BIO_new_fd(STDOUT_FILENO, BIO_NOCLOSE));

		std::cout << "Subject: " << certificate.subject().oneline() << std::endl;
		std::cout << "Issuer: " << certificate.issuer().oneline() << std::endl;

		certificate.print(bio_chain.first());
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

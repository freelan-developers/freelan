/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/bio/bio_chain.hpp>
#include <cryptoplus/error/error_strings.hpp>
#include <cryptoplus/asn1/utctime.hpp>

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
		cryptoplus::x509::certificate certificate = cryptoplus::x509::certificate::create();

		const char cn[] = "My common name";
		const char c[] = "FR";
		const char o[] = "My organization";

		certificate.subject().push_back("CN", MBSTRING_ASC, cn, sizeof(cn) - 1);
		certificate.subject().push_back("C", MBSTRING_ASC, c, sizeof(c) - 1);
		certificate.subject().push_back("O", MBSTRING_ASC, o, sizeof(o) - 1);

		// We copy the data from subject() to issuer().
		certificate.set_issuer(certificate.subject());

		cryptoplus::asn1::utctime not_before = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time());
		cryptoplus::asn1::utctime not_after = cryptoplus::asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time() + boost::posix_time::hours(1));

		certificate.set_not_before(not_before);
		certificate.set_not_after(not_after);

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

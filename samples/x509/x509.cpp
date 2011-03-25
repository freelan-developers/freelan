/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/bio/bio_chain.hpp>
#include <cryptoplus/error/error_strings.hpp>
#include <cryptoplus/asn1/utctime_ptr.hpp>

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

		// We copy the data from subject() to issuer().
		certificate.issuer().insert(certificate.issuer().begin(), certificate.subject().begin(), certificate.subject().end());

		boost::shared_ptr<ASN1_UTCTIME> not_before(ASN1_UTCTIME_new(), ASN1_UTCTIME_free);
		boost::shared_ptr<ASN1_UTCTIME> not_after(ASN1_UTCTIME_new(), ASN1_UTCTIME_free);

		cryptoplus::asn1::utctime_ptr(not_before.get()).set_time(boost::posix_time::second_clock::local_time());
		cryptoplus::asn1::utctime_ptr(not_after.get()).set_time(boost::posix_time::second_clock::local_time() + boost::posix_time::hours(1));

		certificate.set_not_before(not_before.get());
		certificate.set_not_after(not_after.get());

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

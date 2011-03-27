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
		using namespace cryptoplus;

		x509::certificate certificate = x509::certificate::create();

		const char cn[] = "My common name";
		const char c[] = "FR";
		const char o[] = "My organization";

		certificate.subject().push_back("CN", MBSTRING_ASC, cn, sizeof(cn) - 1);
		certificate.subject().push_back("C", MBSTRING_ASC, c, sizeof(c) - 1);
		certificate.subject().push_back("O", MBSTRING_ASC, o, sizeof(o) - 1);

		// We copy the data from subject() to issuer().
		certificate.set_issuer(certificate.subject());

		asn1::utctime not_before = asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time() - boost::gregorian::years(12));
		asn1::utctime not_after = asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time() + boost::posix_time::hours(1));

		not_before = asn1::utctime::from_string("110326145020+0205");

		certificate.set_not_before(not_before);
		certificate.set_not_after(not_after);

		std::cout << asn1::string(not_before.raw()).str() << std::endl;
		std::cout << boost::posix_time::to_iso_string(not_before.to_ptime()) << std::endl;

		bio::bio_chain bio_chain(BIO_new_fd(STDOUT_FILENO, BIO_NOCLOSE));

		certificate.print(bio_chain.first());
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/extension.hpp>
#include <cryptoplus/bio/bio_chain.hpp>
#include <cryptoplus/error/error_strings.hpp>
#include <cryptoplus/asn1/utctime.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cryptoplus/pkey/pkey.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "X509 sample" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << std::endl;

	try
	{
		using namespace cryptoplus;

		x509::certificate certificate = x509::certificate::create();

		// Subject and issuer names

		const char cn[] = "My common name";
		const char c[] = "FR";
		const char o[] = "My organization";

		certificate.subject().push_back("CN", MBSTRING_ASC, cn, sizeof(cn) - 1);
		certificate.subject().push_back("C", MBSTRING_ASC, c, sizeof(c) - 1);
		certificate.subject().push_back("O", MBSTRING_ASC, o, sizeof(o) - 1);

		// We copy the data from subject() to issuer().
		certificate.set_issuer(certificate.subject());

		// Serial number

		certificate.set_serial_number(asn1::integer::from_long(42));

		// Validity

		asn1::utctime not_before = asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time() - boost::gregorian::years(12));
		asn1::utctime not_after = asn1::utctime::from_ptime(boost::posix_time::second_clock::local_time() + boost::posix_time::hours(1));

		certificate.set_not_before(not_before);
		certificate.set_not_after(not_after);

		// Public key

		cryptoplus::pkey::rsa_key rsa_key = cryptoplus::pkey::rsa_key::generate_private_key(1024, 17);

		certificate.set_public_key(pkey::pkey::from_rsa_key(rsa_key));

		// X509 extensions

		certificate.push_back(x509::extension::from_nid(NID_basic_constraints, true, asn1::string::from_string("CA:FALSE")));

		// Sign the certificate

		certificate.sign(pkey::pkey::from_rsa_key(rsa_key), hash::message_digest_algorithm(NID_sha1));

		// Save the certificate

		boost::shared_ptr<FILE> certificate_file(fopen("certificate.crt", "w"), fclose);

		if (certificate_file)
		{
			certificate.write_certificate(certificate_file.get());
		}

		// Let's print the result

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

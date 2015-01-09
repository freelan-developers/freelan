/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/extension.hpp>
#include <cryptoplus/error/error_strings.hpp>
#include <cryptoplus/asn1/utctime.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cryptoplus/pkey/pkey.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifdef MSV
#include <openssl/applink.c>
#endif

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

		// Set the version
		certificate.set_version(2);

		// Subject and issuer names
		certificate.subject().push_back("CN", MBSTRING_ASC, "My common name");
		certificate.subject().push_back("C", MBSTRING_ASC, "FR");
		certificate.subject().push_back("O", MBSTRING_ASC, "My organization");

		std::cout << "Setting subject to: " << certificate.subject() << std::endl;

		// We copy the data from subject() to issuer().
		certificate.set_issuer(certificate.subject());

		std::cout << "Setting issuer to: " << certificate.issuer() << std::endl;

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

		// Thanks to Christer Palm for the explanations on this structure.
		unsigned char ca_true[] = {
			0x30, // BER Type (0x30 = SEQUENCE)
			0x03, // BER Length (0x03 = 3 bytes)
			0x01, // BER Value (for a SEQUENCE this is a variable number of BER elements - here 0x01 means BER Type BOOLEAN)
			0x01, // BER Length (0x01 = 1 byte)
			0xff  // BER Value (0xFF = TRUE - Any non-zero is truthy)
		};

		certificate.push_back(x509::extension::from_nid(NID_basic_constraints, true, asn1::string::from_data(ca_true, sizeof(ca_true))));

		// One could also do that
		//certificate.push_back(x509::extension::from_nconf_nid(NID_basic_constraints, "critical,CA:TRUE"));

		// Sign the certificate
		certificate.sign(pkey::pkey::from_rsa_key(rsa_key), hash::message_digest_algorithm(NID_sha1));

		// Save the certificate
		boost::shared_ptr<FILE> certificate_file(fopen("certificate.crt", "w"), fclose);

		if (certificate_file)
		{
			certificate.write_certificate(certificate_file.get());
		}

		// Let's print the result
		std::cout << certificate << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

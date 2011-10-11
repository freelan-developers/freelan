/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/store.hpp>
#include <cryptoplus/x509/store_context.hpp>
#include <cryptoplus/x509/verify_param.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifdef MSV
#include <openssl/applink.c>
#endif

int verification_callback(int ok, X509_STORE_CTX* ctx)
{
	using namespace cryptoplus;

	x509::store_context store_context(ctx);
	x509::certificate current_cert = store_context.get_current_certificate();

	std::cout << "Verifying \"" << current_cert.subject().oneline() << "\"..." << std::endl;
	if (!ok)
	{
		long error = store_context.get_error();
		int depth = store_context.get_error_depth();
		std::string error_str = store_context.get_error_string();

		std::cerr << "Error " << error << "(depth: " << depth << "): " << error_str << std::endl;
	} else
	{
		std::cout << "Ok." << std::endl;
	}

	return ok;
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Store sample" << std::endl;
	std::cout << "============" << std::endl;
	std::cout << std::endl;

	try
	{
		using namespace cryptoplus;

		// Setup the certificate store
		x509::store store = x509::store::create();

		store.set_verification_callback(&verification_callback);

		store.add_certificate(x509::certificate::from_trusted_certificate(file::open("ca.crt")));
		store.add_certificate(x509::certificate::from_trusted_certificate(file::open("intermediate.crt")));

		// Load the certificate to verify
		x509::certificate cert = x509::certificate::from_certificate(file::open("final.crt"));

		// Create a store context to proceed to verification
		x509::store_context store_context = x509::store_context::create();
		store_context.initialize(store, cert, NULL);

		// Verify !
		std::cout << "Verify: " << store_context.verify() << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

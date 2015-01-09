/**
 * \file x509.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A x509 sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/x509/certificate_revocation_list.hpp>
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

namespace
{
	static int my_index = 0;
}

int verification_callback(int ok, X509_STORE_CTX* ctx)
{
	using namespace cryptoplus;

	// Create a store_context wrapping instance
	x509::store_context store_context(ctx);

	// Get the custom data from the store context
	unsigned int& verified_count = *static_cast<unsigned int*>(store_context.get_external_data(my_index));

	// Increment the verified_count
	verified_count++;
	
	// Get the currently verified certificate
	x509::certificate current_cert = store_context.get_current_certificate();

	std::cout << "Verifying \"" << current_cert.subject() << "\"..." << std::endl;

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
		
		// Register a new index
		// In a real application, you probably want to keep this value in a
		// highly-accessible context as it is not bound to any particular instance
		// and will be destroyed when OpenSSL unloads.
		my_index = x509::store_context::register_index();

		// Setup the certificate store
		x509::store store = x509::store::create();

		store.add_certificate(x509::certificate::from_trusted_certificate(file::open("ca.crt")));
		store.add_certificate(x509::certificate::from_trusted_certificate(file::open("intermediate.crt")));
		store.add_certificate_revocation_list(x509::certificate_revocation_list::from_certificate_revocation_list(file::open("ca.crl")));
		store.add_certificate_revocation_list(x509::certificate_revocation_list::from_certificate_revocation_list(file::open("intermediate.crl")));

		store.set_verification_flags(X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);

		// Load the certificate to verify
		x509::certificate cert = x509::certificate::from_certificate(file::open("final.crt"));

		// Create a store context to proceed to verification
		x509::store_context store_context = x509::store_context::create();

		store_context.initialize(store, cert, NULL);

		// Ensure to set the verification callback *AFTER* you called initialize or it will be ignored.
		store_context.set_verification_callback(&verification_callback);

		// Put some custom data in the store context
		unsigned int verified_count = 0;
		store_context.set_external_data(my_index, &verified_count);

		// Verify !
		std::cout << "Verify: " << store_context.verify() << std::endl;

		// Ouput the custom data.
		std::cout << "Verified certificates count: " << verified_count << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

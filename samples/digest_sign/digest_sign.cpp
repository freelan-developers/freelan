/**
 * \file digest_sign.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A digest sign sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <vector>

#ifdef MSV
#include <openssl/applink.c>
#endif

using cryptoplus::buffer;

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "Digest/Sign sample" << std::endl;
	std::cout << "==================" << std::endl;
	std::cout << std::endl;

	for (auto&& size: { 1024, 2048 }) {
		std::cout << "Generating a " << size << " bits RSA key. This can take some time..." << std::endl;

		cryptoplus::pkey::rsa_key rsa_key = cryptoplus::pkey::rsa_key::generate_private_key(size, 17);

		std::cout << "Done." << std::endl;

		auto pkey = cryptoplus::pkey::pkey::from_rsa_key(rsa_key);
		const std::string message = "my authentic message";

		const std::vector<std::string> algorithms {
			"MD5",
			"MD4",
			"SHA1",
			"SHA",
			"SHA224",
			"SHA256",
			"SHA384",
			"SHA512",
#if OPENSSL_VERSION_NUMBER >= 0x01000000
			"whirlpool",
#endif
			"RIPEMD160"
		};

		cryptoplus::hash::message_digest_context ctx;

		for (auto&& algorithm: algorithms)
		{
			try
			{
				std::cout << algorithm << ": ";

				const auto alg = cryptoplus::hash::message_digest_algorithm(algorithm);
				EVP_PKEY_CTX* evp_ctx = nullptr;

				ctx.digest_sign_initialize(alg, pkey, &evp_ctx);
				EVP_PKEY_CTX_set_rsa_padding(evp_ctx, RSA_PKCS1_PSS_PADDING);
				EVP_PKEY_CTX_set_rsa_pss_saltlen(evp_ctx, -1);
				ctx.digest_sign_update(message.c_str(), message.size());
				const auto result = ctx.digest_sign_finalize();

				std::cout << buffer_size(result) << " bytes - " << result;

				ctx.digest_verify_initialize(alg, pkey, &evp_ctx);
				EVP_PKEY_CTX_set_rsa_padding(evp_ctx, RSA_PKCS1_PSS_PADDING);
				EVP_PKEY_CTX_set_rsa_pss_saltlen(evp_ctx, -1);
				ctx.digest_verify_update(message.c_str(), message.size());
				const auto verify = ctx.digest_verify_finalize(result);

				std::cout << " - " << (verify ? "ok" : "verification failed") << std::endl;
			}
			catch (std::exception& ex)
			{
				std::cout << std::endl;
				std::cerr << "Exception: " << ex.what() << std::endl;
			}
		}
	}

	try
	{
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

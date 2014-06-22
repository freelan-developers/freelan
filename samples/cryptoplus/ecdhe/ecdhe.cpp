/**
 * \file ecdhe.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An elliptic curve DHE sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/pkey/ecdhe.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <cstdio>

#ifdef MSV
#include <openssl/applink.c>
#endif

using cryptoplus::buffer;

namespace
{
	struct named_nid_type
	{
		int nid;
		std::string name;
		size_t result;
		std::string error;
	};
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	std::cout << "ECDHE sample" << std::endl;
	std::cout << "============" << std::endl;
	std::cout << std::endl;

	std::vector<named_nid_type> named_nids = {
		{ NID_X9_62_c2pnb163v1, "NID_X9_62_c2pnb163v1", 0, "" },
		{ NID_X9_62_c2pnb163v2, "NID_X9_62_c2pnb163v2", 0, "" },
		{ NID_X9_62_c2pnb163v3, "NID_X9_62_c2pnb163v3", 0, "" },
		{ NID_X9_62_c2pnb176v1, "NID_X9_62_c2pnb176v1", 0, "" },
		{ NID_X9_62_c2tnb191v1, "NID_X9_62_c2tnb191v1", 0, "" },
		{ NID_X9_62_c2tnb191v2, "NID_X9_62_c2tnb191v2", 0, "" },
		{ NID_X9_62_c2tnb191v3, "NID_X9_62_c2tnb191v3", 0, "" },
		{ NID_X9_62_c2pnb208w1, "NID_X9_62_c2pnb208w1", 0, "" },
		{ NID_X9_62_c2tnb239v1, "NID_X9_62_c2tnb239v1", 0, "" },
		{ NID_X9_62_c2tnb239v2, "NID_X9_62_c2tnb239v2", 0, "" },
		{ NID_X9_62_c2tnb239v3, "NID_X9_62_c2tnb239v3", 0, "" },
		{ NID_X9_62_c2pnb272w1, "NID_X9_62_c2pnb272w1", 0, "" },
		{ NID_X9_62_c2pnb304w1, "NID_X9_62_c2pnb304w1", 0, "" },
		{ NID_X9_62_c2tnb359v1, "NID_X9_62_c2tnb359v1", 0, "" },
		{ NID_X9_62_c2pnb368w1, "NID_X9_62_c2pnb368w1", 0, "" },
		{ NID_X9_62_c2tnb431r1, "NID_X9_62_c2tnb431r1", 0, "" },
		{ NID_X9_62_prime256v1, "NID_X9_62_prime256v1", 0, "" },
		{ NID_secp112r1, "NID_secp112r1", 0, "" },
		{ NID_secp112r2, "NID_secp112r2", 0, "" },
		{ NID_secp128r1, "NID_secp128r1", 0, "" },
		{ NID_secp128r2, "NID_secp128r2", 0, "" },
		{ NID_secp160k1, "NID_secp160k1", 0, "" },
		{ NID_secp160r1, "NID_secp160r1", 0, "" },
		{ NID_secp160r2, "NID_secp160r2", 0, "" },
		{ NID_secp192k1, "NID_secp192k1", 0, "" },
		{ NID_secp224k1, "NID_secp224k1", 0, "" },
		{ NID_secp224r1, "NID_secp224r1", 0, "" },
		{ NID_secp256k1, "NID_secp256k1", 0, "" },
		{ NID_secp384r1, "NID_secp384r1", 0, "" },
		{ NID_secp521r1, "NID_secp521r1", 0, "" },
		{ NID_sect113r1, "NID_sect113r1", 0, "" },
		{ NID_sect113r2, "NID_sect113r2", 0, "" },
		{ NID_sect131r1, "NID_sect131r1", 0, "" },
		{ NID_sect131r2, "NID_sect131r2", 0, "" },
		{ NID_sect163k1, "NID_sect163k1", 0, "" },
		{ NID_sect163r1, "NID_sect163r1", 0, "" },
		{ NID_sect163r2, "NID_sect163r2", 0, "" },
		{ NID_sect193r1, "NID_sect193r1", 0, "" },
		{ NID_sect193r2, "NID_sect193r2", 0, "" },
		{ NID_sect233k1, "NID_sect233k1", 0, "" },
		{ NID_sect233r1, "NID_sect233r1", 0, "" },
		{ NID_sect239k1, "NID_sect239k1", 0, "" },
		{ NID_sect283k1, "NID_sect283k1", 0, "" },
		{ NID_sect283r1, "NID_sect283r1", 0, "" },
		{ NID_sect409k1, "NID_sect409k1", 0, "" },
		{ NID_sect409r1, "NID_sect409r1", 0, "" },
		{ NID_sect571k1, "NID_sect571k1", 0, "" },
		{ NID_sect571r1, "NID_sect571r1", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls1, "NID_wap_wsg_idm_ecid_wtls1", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls3, "NID_wap_wsg_idm_ecid_wtls3", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls4, "NID_wap_wsg_idm_ecid_wtls4", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls5, "NID_wap_wsg_idm_ecid_wtls5", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls7, "NID_wap_wsg_idm_ecid_wtls7", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls8, "NID_wap_wsg_idm_ecid_wtls8", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls9, "NID_wap_wsg_idm_ecid_wtls9", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls10, "NID_wap_wsg_idm_ecid_wtls10", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls11, "NID_wap_wsg_idm_ecid_wtls11", 0, "" },
		{ NID_wap_wsg_idm_ecid_wtls12, "NID_wap_wsg_idm_ecid_wtls12", 0, "" }
	};

	for (auto&& named_nid : named_nids)
	{
		std::cout << "==================================================" << std::endl;
		std::cout << "Testing with nid: " << named_nid.name << std::endl;
		std::cout << "==================================================" << std::endl;
		std::cout << std::endl;

		try
		{
			cryptoplus::pkey::ecdhe_context ctx_a(named_nid.nid);
			cryptoplus::pkey::ecdhe_context ctx_b(named_nid.nid);

			const buffer public_key_a = ctx_a.get_public_key();
			const buffer public_key_b = ctx_b.get_public_key();

			std::cout << "A public key is: " << std::endl << public_key_a.to_string() << std::endl;
			std::cout << "B public key is: " << std::endl << public_key_b.to_string() << std::endl;

			if (public_key_a == public_key_b)
			{
				std::cout << "Public keys are equal. That's not good." << std::endl;

				break;
			}

			const buffer secret_key_a = ctx_a.derive_secret_key(public_key_b);
			const buffer secret_key_b = ctx_b.derive_secret_key(public_key_a);

			std::cout << "A secret key is: " << secret_key_a << std::endl;
			std::cout << "B secret key is: " << secret_key_b << std::endl;

			std::cout << "A and B secret keys should be identical: " << ((secret_key_a == secret_key_b) ? "true" : "false") << std::endl;

			if (secret_key_a == secret_key_b)
			{
				named_nid.result = buffer_size(secret_key_a);
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Exception: " << ex.what() << std::endl;
			named_nid.error = ex.what();
		}

		std::cout << std::endl;
	}

	std::cout << "Summary of results:" << std::endl;

	for (auto&& named_nid : named_nids)
	{
		if (named_nid.result) {
			std::cout << named_nid.name << " (nid: " << named_nid.nid << "): " << named_nid.result * 8 << std::endl;
		} else if (!named_nid.error.empty()) {
			std::cout << named_nid.name << " (nid: " << named_nid.nid << "): " << named_nid.error << std::endl;
		} else {
			std::cout << named_nid.name << " (nid: " << named_nid.nid << "): " << "failed" << std::endl;
		}
	}

	return EXIT_SUCCESS;
}

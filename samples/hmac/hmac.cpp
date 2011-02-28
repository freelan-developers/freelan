#include <cryptopen/hash/hmac_context.hpp>

#include <iostream>
#include <string>
#include <vector>

int main()
{
	cryptopen::hash::hmac_context ctx;

	const std::string key = "my secret key";
	const std::string data = "some data";
	std::vector<unsigned char> hmac(EVP_MD_size(EVP_sha256()));

	ctx.initialize(key.c_str(), key.size(), EVP_sha256());
	ctx.update(data.c_str(), data.size());
	size_t cnt = ctx.finalize(&hmac[0], hmac.size());

	for (size_t i = 0; i < cnt; ++i)
	{
		printf("%02x ", hmac[i]);
	}

	printf("\n");

	return EXIT_SUCCESS;
}

/**
 * \file cipher.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A cipher sample file.
 */

#include <cryptopen/cipher/cipher_context.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

template <typename T>
std::string to_hex(const T& begin, const T& end)
{
	std::ostringstream oss;

	for (T i = begin; i != end; ++i)
	{
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*i);
	}

	return oss.str();
}

std::string to_hex(const void* buf, size_t buf_len)
{
	return to_hex(static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf) + buf_len);
}

void cipher(const std::string& name)
{
	try
	{
		cryptopen::cipher::cipher_algorithm algorithm(name);

		cryptopen::cipher::cipher_context ctx;

		std::vector<unsigned char> data(algorithm.block_size());
		std::vector<unsigned char> key(algorithm.key_length());
		std::vector<unsigned char> iv(algorithm.iv_length());
		std::vector<unsigned char> result(data.size() + algorithm.block_size());

		std::cout << "Cipher: " << name << " (block size: " << algorithm.block_size() << ")" << std::endl;
		std::cout << "Data: " << to_hex(data.begin(), data.end()) << std::endl;
		std::cout << "Key: " << to_hex(key.begin(), key.end()) << std::endl;
		std::cout << "IV: " << to_hex(iv.begin(), iv.end()) << std::endl;

		unsigned char* out = &result[0];
		size_t out_len = result.size();

		ctx.initialize(algorithm, cryptopen::cipher::cipher_context::encrypt, &key[0], &iv[0]);
		ctx.set_padding(false);
		ctx.update(out, out_len, &data[0], data.size());
		
		out += out_len; out_len = result.size() - (out - &result[0]);

		ctx.finalize(out, out_len);

		out += out_len;
		result.resize(out - &result[0]);

		std::cout << "Result: " << to_hex(result.begin(), result.end()) << std::endl;
	}
	catch (cryptopen::error::cryptographic_exception& ex)
	{
		std::cerr << name << ": " << ex.what() << std::endl;
	}
	
	std::cout << std::endl;
}

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;
	cryptopen::cipher::cipher_initializer cipher_initializer;

	std::cout << "Cipher sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	cipher("DES");
	cipher("AES128");
	cipher("AES192");
	cipher("AES256");
	cipher("blowfish");

	return EXIT_SUCCESS;
}

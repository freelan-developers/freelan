/**
 * \file cipher.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A cipher sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/cipher/cipher_stream.hpp>
#include <cryptoplus/error/error_strings.hpp>

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
		cryptoplus::cipher::cipher_algorithm algorithm(name);

		std::vector<unsigned char> data(algorithm.block_size());
		std::vector<unsigned char> key(algorithm.key_length());
		std::vector<unsigned char> iv(algorithm.iv_length());

		std::cout << "Cipher: " << name << " (block size: " << algorithm.block_size() << ")" << std::endl;
		std::cout << "Data: " << to_hex(data.begin(), data.end()) << std::endl;
		std::cout << "Key: " << to_hex(key.begin(), key.end()) << std::endl;
		std::cout << "IV: " << to_hex(iv.begin(), iv.end()) << std::endl;

		cryptoplus::cipher::cipher_stream stream(data.size() + algorithm.block_size());

		stream.initialize(algorithm, cryptoplus::cipher::cipher_stream::encrypt, &key[0], &iv[0]);
		stream.set_padding(false);
		stream.append(&data[0], data.size());
		stream.finalize();

		std::cout << "Result: " << to_hex(stream.result().begin(), stream.result().end()) << std::endl;
	}
	catch (cryptoplus::error::cryptographic_exception& ex)
	{
		std::cerr << name << ": " << ex.what() << std::endl;
	}
	
	std::cout << std::endl;
}

int main()
{
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

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

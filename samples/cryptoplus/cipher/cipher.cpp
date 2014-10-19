/**
 * \file cipher.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A cipher sample file.
 */

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/cipher/cipher_stream.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef MSV
#include <openssl/applink.c>
#endif

using cryptoplus::buffer;
using cryptoplus::buffer_cast;
using cryptoplus::buffer_size;

void cipher(const std::string& name)
{
	try
	{
		cryptoplus::cipher::cipher_algorithm algorithm(name);

		buffer data(buffer::storage_type(algorithm.block_size(), 'd'));
		buffer key(buffer::storage_type(algorithm.key_length(), 'k'));
		buffer iv(buffer::storage_type(algorithm.iv_length(), 'i'));

		std::cout << "Cipher: " << name << " (block size: " << algorithm.block_size() << ")" << std::endl;
		std::cout << "Data: " << data << std::endl;
		std::cout << "Key: " << key << std::endl;
		std::cout << "IV: " << iv << std::endl;

		cryptoplus::cipher::cipher_stream stream(buffer_size(data) + algorithm.block_size());

		stream.initialize(algorithm, cryptoplus::cipher::cipher_stream::encrypt, buffer_cast<const uint8_t*>(key), buffer_size(key), buffer_cast<const uint8_t*>(iv));
		stream.set_padding(false);
		stream.append(data);
		stream.finalize();

		std::cout << "Result: " << stream.result() << std::endl;
	}
	catch (const boost::system::system_error& ex)
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

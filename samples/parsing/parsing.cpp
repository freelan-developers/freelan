/**
 * \file parsing.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A parsing sample file.
 */

#include <kfather/kfather.hpp>
#include <kfather/parser.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <streambuf>

int main()
{
	std::cout << "Parsing sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	std::ifstream file("json/glossary.json");

	json::parser parser;

	std::string str = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	std::string::size_type error_pos = 0;

	if (!parser.parse(str, &error_pos))
	{
		if (error_pos < str.size())
		{
			std::cerr << "Parsing error at character " << error_pos << " ('" << str[error_pos] << "')" << std::endl;
		}
		else
		{
			std::cerr << "Parsing error at the end of the string." << std::endl;
		}
	}

	return EXIT_SUCCESS;
}

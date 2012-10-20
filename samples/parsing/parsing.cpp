/**
 * \file parsing.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A parsing sample file.
 */

#include <kfather/kfather.hpp>
#include <kfather/parser.hpp>
#include <kfather/formatter.hpp>
#include <kfather/serializer.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>

void parse_sample(json::parser& parser, const std::string& sample)
{
	std::cout << "Processing sample \"" << sample << "\"..." << std::endl;

	std::ifstream file(("json/" + sample + ".json").c_str());

	while (file)
	{
		if (parser.parse(file))
		{
			std::cout << std::endl;
		}
		else
		{
			std::cout << std::endl;

			break;
		}
	}
}

int main()
{
	std::cout << "Parsing sample" << std::endl;
	std::cout << "=============" << std::endl;
	std::cout << std::endl;

	json::parser parser;
	json::formatter formatter(parser, std::cout);

	parse_sample(parser, "constants");
	parse_sample(parser, "numbers");
	parse_sample(parser, "strings");
	parse_sample(parser, "arrays");
	parse_sample(parser, "objects");
	parse_sample(parser, "glossary");

	return EXIT_SUCCESS;
}

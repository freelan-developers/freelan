/**
 * \file parsing.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A parsing sample file.
 */

#include <kfather/kfather.hpp>
#include <kfather/parser.hpp>
#include <kfather/formatter.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>

void parse_sample(json::parser& parser, const std::string& sample)
{
	//json::compact_formatter formatter;
	json::inline_formatter formatter;
	//json::pretty_print_formatter formatter;

	std::cout << "Processing sample \"" << sample << "\"..." << std::endl;

	std::ifstream file(("json/" + sample + ".json").c_str());

	json::value_type value;

	while (file)
	{
		if (parser.parse(value, file))
		{
			formatter.format(std::cout, value);

			std::cout << std::endl;
		}
		else
		{
			if ((file >> std::ws).bad())
			{
				std::cerr << "Parsing error." << std::endl;
			}

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

	parse_sample(parser, "constants");
	parse_sample(parser, "numbers");
	parse_sample(parser, "strings");
	parse_sample(parser, "arrays");
	parse_sample(parser, "objects");
	parse_sample(parser, "glossary");

	return EXIT_SUCCESS;
}

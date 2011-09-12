/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file main.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The program entry point.
 */

#include <iostream>
#include <cstdlib>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "configuration_helper.hpp"

const std::string DEFAULT_CONFIGURATION_FILE = "config/freelan.cfg";

namespace po = boost::program_options;
namespace fl = freelan;

bool parse_options(int argc, char** argv, fl::configuration& configuration)
{
	po::options_description generic_options("Generic options");
	generic_options.add_options()
		("help,h", "Produce help message.")
		("configuration_file,c", po::value<std::string>()->default_value(DEFAULT_CONFIGURATION_FILE), "The configuration file to use")
		;

	po::options_description visible_options;
	visible_options.add(generic_options);

	po::options_description configuration_options;
	configuration_options.add(get_network_options());
	configuration_options.add(get_security_options());

	po::options_description all_options;
	all_options.add(generic_options);
	all_options.add(configuration_options);

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, all_options), vm);

	if (vm.count("configuration_file"))
	{
		const std::string configuration_file_str = vm["configuration_file"].as<std::string>();

		std::ifstream configuration_file(configuration_file_str.c_str());

		if (!configuration_file)
		{
			throw po::reading_file(configuration_file_str.c_str());
		}

		po::store(po::parse_config_file(configuration_file, configuration_options, true), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << visible_options << std::endl;

		return false;
	}

	setup_configuration(configuration, vm);

	return true;
}

int main(int argc, char** argv)
{
	try
	{
		fl::configuration configuration;

		if (parse_options(argc, argv, configuration))
		{
			std::cout << "network.hostname_resolution_protocol: " << configuration.hostname_resolution_protocol << std::endl;
			std::cout << "network.listen_on: " << configuration.listen_on << std::endl;
		}
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

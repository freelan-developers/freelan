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
#include <csignal>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <freelan/freelan.hpp>

#include "configuration_helper.hpp"

namespace po = boost::program_options;
namespace fl = freelan;

const std::string DEFAULT_CONFIGURATION_FILE = "config/freelan.cfg";

static boost::function<void ()> stop_function = 0;

static void signal_handler(int code)
{
	switch (code)
	{
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
			if (stop_function)
			{
				std::cerr << "Signal caught: stopping..." << std::endl;

				stop_function();
				stop_function = 0;
			}
			break;
		default:
			break;
	}
}

static bool register_signal_handlers()
{
	if (signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGTERM signals." << std::endl;
		return false;
	}

	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGINT signals." << std::endl;
		return false;
	}

	if (signal(SIGABRT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Failed to catch SIGABRT signals." << std::endl;
		return false;
	}

	return true;
}

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
	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	if (!register_signal_handlers())
	{
		return EXIT_FAILURE;
	}

	try
	{
		fl::configuration configuration;

		if (parse_options(argc, argv, configuration))
		{
			boost::asio::io_service io_service;

			freelan::core core(io_service, configuration);

			core.open();

			stop_function = boost::bind(&freelan::core::close, boost::ref(core));

			std::cout << "Starting freelan core using tap adapter: " << core.tap_adapter().name() << std::endl;

			io_service.run();

			stop_function = 0;
		}
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

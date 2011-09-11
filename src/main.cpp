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

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void parse_options(int argc, char** argv)
{
	po::options_description generic("Generic options");
	generic.add_options()
		("help,h", "Produce help message.")
		;

	po::options_description network("Network options");
	network.add_options()
		("network.listen_on", po::value<std::string>()->default_value("0.0.0.0:12000"), "The endpoint to listen on.")
		("network.hostname_resolution_protocol", "The hostname resolution protocol to use.")
		;

	po::options_description visible;
	visible.add(generic).add(network);

	po::variables_map vm;
	std::ifstream config_file("config/freelan.cfg");
	po::store(po::parse_config_file(config_file, network, true), vm);
	po::store(po::parse_command_line(argc, argv, visible), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << visible << std::endl;
	}

	if (vm.count("network.listen_on"))
	{
		std::cout << "Listen on: " << vm["network.listen_on"].as<std::string>() << std::endl;
	}
}

int main(int argc, char** argv)
{
	try
	{
		parse_options(argc, argv);
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

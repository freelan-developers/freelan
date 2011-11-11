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
 * \file service.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Command-Line Interface program entry point.
 */

#include <cstdlib>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/foreach.hpp>

#include <windows.h>

namespace
{
	fs::path get_module_directory()
	{
		return get_module_filename().parent_path();
	}

	fs::path get_root_directory()
	{
		return get_module_directory().parent_path();
	}

	fs::path get_bin_directory()
	{
		return get_root_directory() / "bin";
	}

	fs::path get_log_directory()
	{
		return get_root_directory() / "log";
	}

	fs::path get_config_directory()
	{
		return get_root_directory() / "config";
	}

	fs::path get_main_log_filename()
	{
		return get_log_directory() / "freelan.log";
	}

	std::vector<fs::path> get_configuration_files()
	{
		std::vector<fs::path> configuration_files;

		for (fs::directory_iterator entry = fs::directory_iterator(get_config_directory()); entry != fs::directory_iterator(); ++entry)
		{
			if (entry->status().type() == fs::regular_file)
			{
				if (entry->path().extension() == ".cfg")
				{
					configuration_files.push_back(entry->path());
				}
			}
		}

		return configuration_files;
	}
}

int main(int argc, char** argv)
{
	po::options_description generic_options("Generic options");

	generic_options.add_options()
	("help,h", "Produce help message.")
	("install", "Install the service.")
	("uninstall", "Uninstall the service.")
	;

	po::variables_map vm;

	try
	{
		po::store(po::parse_command_line(argc, argv, generic_options), vm);
		po::notify(vm);
	}
	catch (boost::program_options::error& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		std::cerr << generic_options << std::endl;

		return EXIT_FAILURE;
	}

	if (vm.count("help"))
	{
		std::cout << generic_options << std::endl;

		return EXIT_SUCCESS;
	}

	try
	{
		if (vm.count("install"))
		{
			if (vm.count("uninstall"))
			{
				std::cerr << "Cannot specify both --install and --uninstall options." << std::endl;

				return EXIT_FAILURE;
			}

			InstallService();
		}
		else if (vm.count("uninstall"))
		{
			UninstallService();
		}
		else
		{
			RunService();
		}
	}
	catch (boost::system::system_error& ex)
	{
		std::cerr << ex.code() << ":" << ex.what() << std::endl;

		return EXIT_FAILURE;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


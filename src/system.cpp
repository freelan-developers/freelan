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
 * \file system.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief System related functions.
 */

#include "system.hpp"

#include <stdexcept>

#include <cryptoplus/os.hpp>

#ifdef WINDOWS
#include <shlobj.h>
#endif

std::string get_home_directory()
{
#ifdef WINDOWS
	char path[MAX_PATH] = {};

	HRESULT ret = SHGetFolderPath(0, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, path);

	if (FAILED(ret))
	{
		throw std::runtime_error("Unable to determine the home directory");
	}

	return path;
#else
	char* path = getenv("HOME");

	if (path == NULL)
	{
		throw std::runtime_error("Unable to determine the home directory");
	}

	return path;
#endif
}

std::string get_application_directory()
{
#ifdef WINDOWS
	char path[MAX_PATH] = {};

	HRESULT ret = SHGetFolderPath(0, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	if (FAILED(ret))
	{
		throw std::runtime_error("Unable to determine the application directory");
	}

	return std::string(path) + "\\freelan";
#else
	return "/etc/freelan";
#endif
}

std::vector<std::string> get_configuration_files()
{
	std::vector<std::string> configuration_files;

#ifdef WINDOWS
	configuration_files.push_back(get_home_directory() + "\\freelan.cfg");
	configuration_files.push_back(get_application_directory() + "\\freelan.cfg");
#else
	configuration_files.push_back(get_home_directory() + "/.freelan/freelan.cfg");
	configuration_files.push_back(get_application_directory() + "/freelan.cfg");
#endif

	return configuration_files;
}

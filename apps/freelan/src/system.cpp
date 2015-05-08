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
#include <cstdlib>
#include <cstdarg>
#include <sstream>

// This file is generated locally.
#include <defines.hpp>

#ifdef WINDOWS
#include <executeplus/windows_system.hpp>
#include <asiotap/windows/registry.hpp>

#include <shlobj.h>
#else
#include <executeplus/posix_system.hpp>
#endif

namespace fs = boost::filesystem;

#ifdef WINDOWS
fs::path get_module_filename()
{
	TCHAR path[_MAX_PATH + 1];

	if (::GetModuleFileName(NULL, path, sizeof(path) / sizeof(path[0])) > 0)
	{
		return path;
	}
	else
	{
		throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "GetModuleFileName()");
	}
}
#endif

boost::filesystem::path get_installation_directory()
{
#ifdef WINDOWS
	try
	{
		asiotap::registry_key installation_key(HKEY_LOCAL_MACHINE, "SOFTWARE\\FreeLAN");

		return installation_key.query_path("installation_path");
	}
	catch (const boost::system::system_error&)
	{
		return get_module_filename().parent_path().parent_path();
	}
#else
	return FREELAN_INSTALL_PREFIX;
#endif
}

fs::path get_home_directory()
{
#ifdef WINDOWS
	TCHAR path[MAX_PATH] = {};

	HRESULT ret = SHGetFolderPath(0, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, path);

	if (FAILED(ret))
	{
		throw std::runtime_error("Unable to determine the home directory");
	}
#else
	char* path = getenv("HOME");

	if (path == NULL)
	{
		throw std::runtime_error("Unable to determine the home directory");
	}
#endif

	return fs::path(path) / "." FREELAN_NAME;
}

fs::path get_data_directory()
{
#ifdef WINDOWS
	TCHAR path[MAX_PATH] = {};

	HRESULT ret = SHGetFolderPath(0, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	if (FAILED(ret))
	{
		throw std::runtime_error("Unable to determine the application directory");
	}

	return fs::path(path) / FREELAN_NAME;
#else
	return get_installation_directory() / "var/" FREELAN_NAME;
#endif
}

fs::path get_configuration_directory()
{
#ifdef WINDOWS
	return get_installation_directory() / "config/";
#else
	return get_installation_directory() / "etc/" FREELAN_NAME;
#endif
}

fs::path get_temporary_directory()
{
#ifdef WINDOWS
	TCHAR path[MAX_PATH] = {};

	HRESULT ret = ::GetTempPath(sizeof(path), path);

	if (FAILED(ret))
	{
		throw std::runtime_error("Unable to determine the temporary directory");
	}

	return path;
#else
	return "/tmp/";
#endif
}

#if defined(WINDOWS) && defined(UNICODE)
int execute(const fscp::logger& logger, fs::path script, const std::vector<std::wstring>& args, const std::map<std::wstring, std::wstring>& env)
#else
int execute(const fscp::logger& logger, fs::path script, const std::vector<std::string>& args, const std::map<std::string, std::string>& env)
#endif
{
#if defined(WINDOWS) && defined(UNICODE)
	std::vector<std::wstring> real_args = { script.wstring() };
#else
	std::vector<std::string> real_args = { script.string() };
#endif
	real_args.insert(real_args.end(), args.begin(), args.end());
	auto new_env = executeplus::get_current_environment();

	for (auto&& pair : env)
	{
		new_env[pair.first] = pair.second;
	}

	logger(fscp::log_level::debug) << "Calling script " << script.string() << "...";

#if defined(WINDOWS)
	const auto return_code = executeplus::execute(real_args, new_env);
#else
	std::ostringstream oss;

	const auto return_code = executeplus::execute(real_args, new_env, &oss);
#endif

	const auto log_level = (return_code == 0) ? fscp::log_level::debug : fscp::log_level::warning;
	logger(log_level) << "Script " << script.string() << " returned " << return_code << ".";

#if !defined(WINDOWS)
	const auto output = oss.str();

	if (!output.empty())
	{
		logger(fscp::log_level::debug) << "Output follows:\n" << output;
	}
#endif

	return return_code;
}

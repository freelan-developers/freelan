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
 * \file system.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief System related functions.
 */

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <vector>
#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include <freelan/os.hpp>

#include <fscp/logger.hpp>

#ifdef WINDOWS
/**
 * \brief Get the filename of the current module.
 * \return The current module filename.
 */
boost::filesystem::path get_module_filename();
#endif

/**
 * \brief Get the installation directory.
 * \return The installation directory.
 */
boost::filesystem::path get_installation_directory();

/**
 * \brief Get the home directory for the current user.
 * \return The home directory for the current user.
 */
boost::filesystem::path get_home_directory();

/**
 * \brief Get the application data directory.
 * \return The application data directory.
 */
boost::filesystem::path get_data_directory();

/**
 * \brief Get the application configuration directory.
 * \return The application configuration directory.
 */
boost::filesystem::path get_configuration_directory();

/**
 * \brief Get the temporary directory.
 * \return The temporary directory.
 */
boost::filesystem::path get_temporary_directory();

/**
 * \brief Execute a script and get the exit status.
 * \param script The script to execute.
 * \param args The parameters.
 * \param env Variables to inject into the environment.
 * \return The exit status.
 */
#if defined(WINDOWS) && defined(UNICODE)
int execute(const fscp::logger& logger, boost::filesystem::path script, const std::vector<std::wstring>& args, const std::map<std::wstring, std::wstring>& env = std::map<std::wstring, std::wstring>());
#else
int execute(const fscp::logger& logger, boost::filesystem::path script, const std::vector<std::string>& args, const std::map<std::string, std::string>& env = std::map<std::string, std::string>());
#endif

#endif /* SYSTEM_HPP */

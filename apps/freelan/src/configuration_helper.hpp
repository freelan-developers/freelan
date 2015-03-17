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
 * \file configuration_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A configuration helper.
 */

#ifndef CONFIGURATION_HELPER_HPP
#define CONFIGURATION_HELPER_HPP

#include <freelan/configuration.hpp>

#include <fscp/logger.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

/**
 * \brief Get the server options.
 * \return The server options.
 */
boost::program_options::options_description get_server_options();

/**
 * \brief Get the client options.
 * \return The client options.
 */
boost::program_options::options_description get_client_options();

/**
 * \brief Get the fscp options.
 * \return The fscp options.
 */
boost::program_options::options_description get_fscp_options();

/**
 * \brief Get the security options.
 * \return The security options.
 */
boost::program_options::options_description get_security_options();

/**
 * \brief Get the tap adapter options.
 * \return The tap adapter options.
 */
boost::program_options::options_description get_tap_adapter_options();

/**
 * \brief Get the switch options.
 * \return The switch options.
 */
boost::program_options::options_description get_switch_options();

/**
 * \brief Get the router options.
 * \return The router options.
 */
boost::program_options::options_description get_router_options();

/**
 * \brief Set the paths options relative to the specified root.
 * \param vm The variables map.
 * \param root The root directory for relative paths.
 */
void make_paths_absolute(boost::program_options::variables_map& vm, const boost::filesystem::path& root);

/**
 * \brief Setup a freelan configuration from a variables map.
 * \param logger The logger to use.
 * \param configuration The configuration to setup.
 * \param vm The variables map.
 * \warning On error, a boost::program_options::error might be thrown.
 */
void setup_configuration(const fscp::logger& logger, freelan::configuration& configuration, const boost::program_options::variables_map& vm);

#endif /* CONFIGURATION_HELPER_HPP */

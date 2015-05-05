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
 * \file tools.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Tools.
 */

#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <freelan/os.hpp>
#include <freelan/core.hpp>

#include <fscp/logger.hpp>

#include <asiotap/tap_adapter.hpp>

#ifndef WINDOWS
/**
 * \brief Convert the specified log level to its syslog equivalent priority.
 * \param level The log level.
 * \return The syslog equivalent priority.
 */
int log_level_to_syslog_priority(fscp::log_level level);
#endif

/**
 * \brief Convert the specified log level to its string representation.
 * \param level The log level.
 * \return The string representation of level.
 */
const char* log_level_to_string(fscp::log_level level);

/**
 * \brief The tap adapter up function.
 * \param script The script to call.
 * \param logger The logger instance.
 * \param tap_adapter The tap_adapter instance.
 */
void execute_tap_adapter_up_script(const boost::filesystem::path& script, const fscp::logger& logger, const asiotap::tap_adapter& tap_adapter);

/**
 * \brief The tap adapter down function.
 * \param script The script to call.
 * \param logger The logger instance.
 * \param tap_adapter The tap_adapter instance.
 */
void execute_tap_adapter_down_script(const boost::filesystem::path& script, const fscp::logger& logger, const asiotap::tap_adapter& tap_adapter);

/**
 * \brief The certificate validation function.
 * \param script The script to call.
 * \param logger The logger instance.
 * \param cert The certificate.
 * \return The execution result of the specified script.
 */
bool execute_certificate_validation_script(const boost::filesystem::path& script, const fscp::logger& logger, freelan::security_configuration::cert_type cert);

/**
 * \brief The authentication function.
 * \param script The script to call.
 * \param logger The logger instance.
 * \param username The username.
 * \param password The password.
 * \param remote_host The remote host.
 * \param remote_port The remote port.
 * \return The execution result of the specified script.
 */
bool execute_authentication_script(const boost::filesystem::path& script, const fscp::logger& logger, const std::string& username, const std::string& password, const std::string& remote_host, uint16_t remote_port);

/**
 * \brief The DNS function.
 * \param script The script to call.
 * \param logger The logger instance.
 * \param tap_adapter The tap_adapter instance.
 * \param action A flag that indicates the action.
 * \param dns_server The DNS server address to add or remove.
 * \return The execution result of the specified script.
 */
bool execute_dns_script(const boost::filesystem::path& script, const fscp::logger& logger, const std::string& tap_adapter, freelan::core::DnsAction action, const boost::asio::ip::address& dns_server);

#endif /* TOOLS_HPP */

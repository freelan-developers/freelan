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
 * \file tools.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Tools.
 */

#include "tools.hpp"

#ifndef WINDOWS
#include <syslog.h>
#endif

#include <boost/lexical_cast.hpp>

#include <fscp/logger.hpp>
#include <iconvplus/converter.hpp>

#include "system.hpp"

namespace fs = boost::filesystem;
namespace fl = freelan;

#ifndef WINDOWS
int log_level_to_syslog_priority(fscp::log_level level)
{
	switch (level)
	{
		case fscp::log_level::trace:
			return LOG_DEBUG;
		case fscp::log_level::debug:
			return LOG_DEBUG;
		case fscp::log_level::information:
			return LOG_INFO;
		case fscp::log_level::important:
			return LOG_NOTICE;
		case fscp::log_level::warning:
			return LOG_WARNING;
		case fscp::log_level::error:
			return LOG_ERR;
		case fscp::log_level::fatal:
			return LOG_CRIT;
	}

	assert(false);
	throw std::logic_error("Unsupported enumeration value");
}
#else
	namespace
	{
		std::wstring to_wstring(const std::string& str)
		{
			iconvplus::iconv_instance iconv("utf-16", "");
			iconvplus::converter<char, wchar_t> converter(iconv);
			std::wostringstream out;
			std::istringstream in(str);
			converter.convert(in, out);
			
			return out.str();
		}
	}
#endif

const char* log_level_to_string(fscp::log_level level)
{
	switch (level)
	{
		case fscp::log_level::trace:
			return "TRACE";
		case fscp::log_level::debug:
			return "DEBUG";
		case fscp::log_level::information:
			return "INFORMATION";
		case fscp::log_level::important:
			return "IMPORTANT";
		case fscp::log_level::warning:
			return "WARNING";
		case fscp::log_level::error:
			return "ERROR";
		case fscp::log_level::fatal:
			return "FATAL";
	}

	assert(false);
	throw std::logic_error("Unsupported enumeration value");
}

void execute_tap_adapter_up_script(const boost::filesystem::path& script, const fscp::logger& logger, const asiotap::tap_adapter& tap_adapter)
{
#if defined(WINDOWS) && defined(UNICODE)
	int exit_status = execute(logger, script, { to_wstring(tap_adapter.name()) });
#else
	int exit_status = execute(logger, script, { tap_adapter.name() });
#endif

	if (exit_status != 0)
	{
		logger(fscp::log_level::warning) << "Up script exited with a non-zero exit status: " << exit_status;
	}
}

void execute_tap_adapter_down_script(const boost::filesystem::path& script, const fscp::logger& logger, const asiotap::tap_adapter& tap_adapter)
{
#if defined(WINDOWS) && defined(UNICODE)
	int exit_status = execute(logger, script, { to_wstring(tap_adapter.name()) });
#else
	int exit_status = execute(logger, script, { tap_adapter.name() });
#endif

	if (exit_status != 0)
	{
		logger(fscp::log_level::warning) << "Down script exited with a non-zero exit status: " << exit_status;
	}
}

bool execute_certificate_validation_script(const fs::path& script, const fscp::logger& logger, fl::security_configuration::cert_type cert)
{
	static boost::mutex mutex;

	// This prevents two scripts validation to run at the same time.
	boost::mutex::scoped_lock lock(mutex);

	try
	{
		const fs::path filename = get_temporary_directory() / ("freelan_certificate.crt");

		if (logger.level() <= fscp::log_level::debug)
		{
			logger(fscp::log_level::debug) << "Writing temporary certificate file at: " << filename;
		}

#ifdef WINDOWS
#ifdef UNICODE
		cert.write_certificate(cryptoplus::file::open(filename.string<std::basic_string<TCHAR> >(), L"w"));
#else
		cert.write_certificate(cryptoplus::file::open(filename.string<std::basic_string<TCHAR> >(), "w"));
#endif
#else
		cert.write_certificate(cryptoplus::file::open(filename.string<std::basic_string<char> >(), "w"));
#endif

#if defined(WINDOWS) && defined(UNICODE)
		const int exit_status = execute(logger, script, { filename.wstring() });
#else
		const int exit_status = execute(logger, script, { filename.string() });
#endif
		if (logger.level() <= fscp::log_level::debug)
		{
			logger(fscp::log_level::debug) << script << " terminated execution with exit status " << exit_status ;
		}

		fs::remove(filename);

		return (exit_status == 0);
	}
	catch (std::exception& ex)
	{
		logger(fscp::log_level::warning) << "Error while executing certificate validation script (" << script << "): " << ex.what() ;

		return false;
	}
}

bool execute_authentication_script(const boost::filesystem::path& script, const fscp::logger& logger, const std::string& username, const std::string& password, const std::string& remote_host, uint16_t remote_port)
{
#if defined(WINDOWS) && defined(UNICODE)
	std::map<std::wstring, std::wstring> env;
	env[L"FREELAN_USERNAME"] = to_wstring(username);
	env[L"FREELAN_PASSWORD"] = to_wstring(password);
	env[L"FREELAN_REMOTE_HOST"] = to_wstring(remote_host);
	env[L"FREELAN_REMOTE_PORT"] = to_wstring(boost::lexical_cast<std::string>(remote_port));

	int exit_status = execute(logger, script, {}, env);
#else
	std::map<std::string, std::string> env;
	env["FREELAN_USERNAME"] = username;
	env["FREELAN_PASSWORD"] = password;
	env["FREELAN_REMOTE_HOST"] = remote_host;
	env["FREELAN_REMOTE_PORT"] = boost::lexical_cast<std::string>(remote_port);

	int exit_status = execute(logger, script, {}, env);
#endif

	if (exit_status != 0)
	{
		logger(fscp::log_level::warning) << "Authentication script exited with a non-zero exit status: " << exit_status;
	}

	return (exit_status == 0);
}

bool execute_dns_script(const boost::filesystem::path& script, const fscp::logger& logger, const std::string& tap_adapter, freelan::core::DnsAction action, const boost::asio::ip::address& dns_server)
{
	const std::string str_action = (action == freelan::core::DnsAction::Add) ? "add" : "remove";

#if defined(WINDOWS) && defined(UNICODE)
	int exit_status = execute(logger, script, { to_wstring(tap_adapter), to_wstring(str_action), to_wstring(dns_server.to_string()) });
#else
	int exit_status = execute(logger, script, { tap_adapter, str_action, dns_server.to_string() });
#endif

	if (exit_status != 0)
	{
		logger(fscp::log_level::warning) << "DNS script exited with a non-zero exit status: " << exit_status;
	}

	return (exit_status == 0);
}
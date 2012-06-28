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

#include <freelan/logger_stream.hpp>

#include "system.hpp"

namespace fs = boost::filesystem;
namespace fl = freelan;

#ifndef WINDOWS
int log_level_to_syslog_priority(freelan::log_level level)
{
	switch (level)
	{
		case freelan::LL_DEBUG:
			return LOG_DEBUG;
		case freelan::LL_INFORMATION:
			return LOG_INFO;
		case freelan::LL_WARNING:
			return LOG_WARNING;
		case freelan::LL_ERROR:
			return LOG_ERR;
		case freelan::LL_FATAL:
			return LOG_CRIT;
	}

	assert(false);
	throw std::logic_error("Unsupported enumeration value");
}
#endif

const char* log_level_to_string(freelan::log_level level)
{
	switch (level)
	{
		case freelan::LL_DEBUG:
			return "DEBUG";
		case freelan::LL_INFORMATION:
			return "INFORMATION";
		case freelan::LL_WARNING:
			return "WARNING";
		case freelan::LL_ERROR:
			return "ERROR";
		case freelan::LL_FATAL:
			return "FATAL";
	}

	assert(false);
	throw std::logic_error("Unsupported enumeration value");
}

void execute_tap_adapter_up_script(const boost::filesystem::path& script, freelan::core& core, const asiotap::tap_adapter& tap_adapter)
{
	int exit_status = execute(script, tap_adapter.name().c_str(), NULL);

	if (exit_status != 0)
	{
		core.logger()(freelan::LL_WARNING) << "Up script exited with a non-zero exit status: " << exit_status;
	}
}

void execute_tap_adapter_down_script(const boost::filesystem::path& script, freelan::core& core, const asiotap::tap_adapter& tap_adapter)
{
	int exit_status = execute(script, tap_adapter.name().c_str(), NULL);

	if (exit_status != 0)
	{
		core.logger()(freelan::LL_WARNING) << "Down script exited with a non-zero exit status: " << exit_status;
	}
}

bool execute_certificate_validation_script(const fs::path& script, fl::core& core, fl::security_configuration::cert_type cert)
{
	static unsigned int counter = 0;

	try
	{
		const fs::path filename = get_temporary_directory() / ("freelan_certificate_" + boost::lexical_cast<std::string>(counter++) + ".crt");

		if (core.logger().level() <= freelan::LL_DEBUG)
		{
			core.logger()(freelan::LL_DEBUG) << "Writing temporary certificate file at: " << filename;
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

		const int exit_status = execute(script, filename.c_str(), NULL);

		if (core.logger().level() <= freelan::LL_DEBUG)
		{
			core.logger()(freelan::LL_DEBUG) << script << " terminated execution with exit status " << exit_status ;
		}

		fs::remove(filename);

		return (exit_status == 0);
	}
	catch (std::exception& ex)
	{
		core.logger()(freelan::LL_WARNING) << "Error while executing certificate validation script (" << script << "): " << ex.what() ;

		return false;
	}
}

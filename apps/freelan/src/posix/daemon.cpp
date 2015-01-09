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
 * \file daemon.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief POSIX related daemon functions.
 */

#include "daemon.hpp"

#include <boost/system/system_error.hpp>

#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include "../tools.hpp"

namespace posix
{
	void daemonize()
	{
		pid_t pid = ::fork();

		if (pid < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category(), "Cannot fork the current process.");
		}

		if (pid > 0)
		{
			exit(EXIT_SUCCESS);
		}

		::openlog("freelan", LOG_PID, LOG_DAEMON);

		pid_t sid = ::setsid();

		if (sid < 0)
		{
			::syslog(LOG_ERR, "setsid():%u:%s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (::chdir("/") < 0)
		{
			::syslog(LOG_ERR, "chdir():%u:%s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
		::close(STDERR_FILENO);
	}

	void syslog(fscp::log_level level, const std::string& msg, const boost::posix_time::ptime&)
	{
		// We ignore the timestamp.

		::syslog(log_level_to_syslog_priority(level), "%s", msg.c_str());
	}
}

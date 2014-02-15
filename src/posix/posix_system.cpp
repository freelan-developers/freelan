/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file posix_system.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Posix system primitives.
 */

#include "posix_system.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace asiotap
{
	int execute(const std::vector<std::string>& args, boost::system::error_code& ec)
	{
		int fd[2];

		if (::pipe(fd) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::system_category());

			return -1;
		}

		const pid_t pid = fork();

		switch (pid)
		{
			case -1:
				{
					// fork() failed.
					::close(fd[0]);
					::close(fd[1]);

					ec = boost::system::error_code(errno, boost::system::system_category());

					return -1;
				}
			case 0:
				{
					// Child process
					const int fdlimit = ::sysconf(_SC_OPEN_MAX);

					for (int n = 0; n < fdlimit; ++n)
					{
						if (n != fd[1])
						{
							::close(n);
						}
					}

					fcntl(fd[1], F_SETFD, FD_CLOEXEC);

					// Estimate the required size for the argv buffer.

					// One null-terminated byte per arg.
					size_t buffer_size = args.size();

					for (auto&& arg : args)
					{
						buffer_size += arg.size();
					}

					std::vector<char> buffer(buffer_size, 0x00);
					std::vector<char*> argv(args.size() + 1, nullptr);
					auto offset = buffer.begin();

					for (size_t i = 0; i != args.size(); ++i)
					{
						const auto& arg = args[i];

						argv[i] = &*offset;
						offset = std::copy(arg.begin(), arg.end(), offset);
						*(offset++) = '\0';
					}

					// Execute the file specified
					::execv(argv[0], &argv[0]);

					// Something went wrong. Sending back errno to parent process then exiting.
					if (::write(fd[1], &errno, sizeof(errno))) {}

					_exit(127);

					break;
				}
			default:
				{
					// Parent process
					::close(fd[1]);

					int child_errno = 0;

					const ssize_t readcnt = ::read(fd[0], &child_errno, sizeof(child_errno));

					::close(fd[0]);

					if (readcnt < 0)
					{
						ec = boost::system::error_code(errno, boost::system::system_category());

						return -1;
					}
					else if (readcnt == sizeof(child_errno))
					{
						ec = boost::system::error_code(child_errno, boost::system::system_category());

						return -1;
					}

					int status = 0;

					if (::waitpid(pid, &status, 0) != pid)
					{
						ec = boost::system::error_code(child_errno, boost::system::system_category());

						return -1;
					}
					else
					{
						if (WIFEXITED(status))
						{
							return WEXITSTATUS(status);
						}
					}

					break;
				}
		}

		return EXIT_FAILURE;
	}

	int execute(const std::vector<std::string>& args)
	{
		boost::system::error_code ec;

		const auto result = execute(args, ec);

		if (result < 0)
		{
			throw boost::system::system_error(ec);
		}

		return result;
	}

	bool ifconfig(const std::string& interface, const std::vector<std::string>& args)
	{
		std::vector<std::string> real_args;
		real_args.reserve(args.size() + 2);
		real_args.push_back("/sbin/ifconfig");
		real_args.push_back(interface);
		real_args.insert(real_args.end(), args.begin(), args.end());

		return (execute(real_args) == 0);
	}
}

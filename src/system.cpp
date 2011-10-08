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

#include <cryptoplus/os.hpp>

#include <boost/system/system_error.hpp>

#ifdef WINDOWS
#include <shlobj.h>
#include <shellapi.h>
#elif defined(UNIX)
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <cstring>
#endif

namespace
{
#ifdef WINDOWS
	void throw_system_error(LONG error)
	{
		throw boost::system::system_error(error, boost::system::system_category());
	}

	DWORD create_process(const char* application, char* command_line, bool enable_stdout = false)
	{
		DWORD exit_status;

		STARTUPINFO si;
		si.cb = sizeof(si);
		si.lpReserved = NULL;
		si.lpDesktop = NULL;
		si.lpTitle = NULL;
		si.dwX = 0;
		si.dwY = 0;
		si.dwXSize = 0;
		si.dwYSize = 0;
		si.dwXCountChars = 0;
		si.dwYCountChars = 0;
		si.dwFillAttribute = 0;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // Remove STARTF_USESTDHANDLES to show stdout
		si.wShowWindow = SW_HIDE;
		si.cbReserved2 = 0;
		si.lpReserved2 = NULL;
		si.hStdInput = INVALID_HANDLE_VALUE;
		si.hStdOutput = enable_stdout ? ::GetStdHandle(STD_OUTPUT_HANDLE) : INVALID_HANDLE_VALUE;
		si.hStdError = INVALID_HANDLE_VALUE;

		PROCESS_INFORMATION pi;

		if (!::CreateProcess(
					application,
					command_line,
					NULL,
					NULL,
					FALSE,
					0,
					NULL, //environment
					NULL,
					&si,
					&pi
					))
		{
			throw_system_error(::GetLastError());
		}

		::CloseHandle(pi.hThread);

		DWORD wait_result = ::WaitForSingleObject(pi.hProcess, INFINITE);

		try
		{
			switch (wait_result)
			{
				case WAIT_OBJECT_0:
					{
						DWORD exit_code = 0;

						if (::GetExitCodeProcess(pi.hProcess, &exit_code))
						{
							exit_status = static_cast<int>(exit_code);
						} else
						{
							throw_system_error(::GetLastError());
						}

						break;
					}
				default:
					{
						throw_system_error(::GetLastError());
					}
			}
		}
		catch (...)
		{
			::CloseHandle(pi.hProcess);

			throw;
		}

		::CloseHandle(pi.hProcess);

		return exit_status;
	}

#elif defined(UNIX)
	void throw_system_error(int error)
	{
		char error_str[256] = { 0 };

		if (strerror_r(error, error_str, sizeof(error_str)) != 0)
		{
			throw boost::system::system_error(error, boost::system::system_category());
		}
		else
		{
			throw boost::system::system_error(error, boost::system::system_category(), error_str);
		}
	}

	int execute_script(const char* file, char* const argv[], bool enable_stdout = false)
	{
		int exit_status = 255;
		int fd[2];

		if (::pipe(fd) < 0)
		{
			throw_system_error(errno);
		}

		pid_t pid;

		switch (pid = fork())
		{
			case -1:
				{
					::close(fd[0]);
					::close(fd[1]);

					throw_system_error(errno);
					break;
				}

			case 0:
				{
					// Child process
					int fdlimit = ::sysconf(_SC_OPEN_MAX);

					for (int n = 0; n < fdlimit; ++n)
					{
						if (n != fd[1])
						{
							if (!enable_stdout || (n != STDOUT_FILENO))
							{
								::close(n);
							}
						}
					}

					fcntl(fd[1], F_SETFD, FD_CLOEXEC);

					// Execute the file specified
					::execv(file, argv);

					// Something went wrong. Sending back errno to parent process then exiting.
					if (::write(fd[1], &errno, sizeof(errno))) {}
					_exit(EXIT_FAILURE);
					break;
				}

			default:
				{
					// Parent process
					int errno_child = 0;
					::close(fd[1]);

					ssize_t readcnt = ::read(fd[0], &errno_child, sizeof(errno_child));
					::close(fd[0]);

					if (readcnt < 0)
					{
						throw_system_error(errno);
					}
					else if (readcnt == sizeof(errno_child))
					{
						throw_system_error(errno_child);
					} else
					{
						int status;

						if (::waitpid(pid, &status, 0) < 0)
						{
							throw_system_error(errno);
						} else
						{
							if (WIFEXITED(status))
							{
								exit_status = WEXITSTATUS(status);
							}
						}
					}

					break;
				}
		}

		return exit_status;
	}

#endif
}

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

int execute(const char* file, ...)
{
	int exit_status;

	va_list vl;
	va_start(vl, file);

	try
	{
#ifdef WINDOWS
		char command_line[32768] = {};
		size_t offset = 0;
		
		for (const char* arg = va_arg(vl, const char*); arg != NULL; arg = va_arg(vl, const char*))
		{
			command_line[offset++] = '"';

			for (; *arg != '\0'; ++arg)
			{
				if (*arg == '"')
				{
					command_line[offset++] = '\\';
				}

				command_line[offset++] = *arg;
			}

			command_line[offset++] = '"';
			command_line[offset++] = ' ';
		}
		
		exit_status = create_process(file, command_line, true);

#elif defined(UNIX)
		char* argv[256] = {};
		size_t cnt = 0;
		char args[32728] = {};
		size_t offset = 0;

		for (const char* arg = va_arg(vl, const char*); arg != NULL; arg = va_arg(vl, const char*))
		{
			argv[cnt++] = &args[offset];

			for (; *arg != '\0'; ++arg)
			{
				args[offset++] = *arg;
			}

			args[offset++] = '\0';
		}

		exit_status = execute_script(file, argv);
		
#endif
	}
	catch (...)
	{
		va_end(vl);

		throw;
	}

	va_end(vl);

	return exit_status;
}

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
 * \file service.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Command-Line Interface program entry point.
 */

#include <cstdlib>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>

#include <windows.h>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include "common/system.hpp"
#include "common/tools.hpp"

#ifdef UNICODE
#define SERVICE_NAME L"FreeLAN Service"
#else
#define SERVICE_NAME "FreeLAN Service"
#endif

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace fl = freelan;

namespace
{
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

	fs::path get_module_directory()
	{
		return get_module_filename().parent_path();
	}

	fs::path get_root_directory()
	{
		return get_module_directory().parent_path();
	}

	fs::path get_bin_directory()
	{
		return get_root_directory() / "bin";
	}

	fs::path get_log_directory()
	{
		return get_root_directory() / "log";
	}

	fs::path get_config_directory()
	{
		return get_root_directory() / "config";
	}
}

struct service_context
{
	SERVICE_STATUS_HANDLE service_status_handle;
	SERVICE_STATUS service_status;
};

DWORD HandlerEx(DWORD control, DWORD event_type, void* event_data, void* context)
{
	(void)control;
	(void)event_type;
	(void)event_data;

	service_context& ctx = *static_cast<service_context*>(context);

	switch (control)
	{
		case SERVICE_CONTROL_INTERROGATE:
			return NO_ERROR;
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			ctx.service_status.dwCurrentState = SERVICE_STOP_PENDING;
			::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);
			return NO_ERROR;
		case SERVICE_CONTROL_PAUSE:
			break;
		case SERVICE_CONTROL_CONTINUE:
			break;
		default:
			if (control >= 128 && control <= 255)
			{
				return ERROR_CALL_NOT_IMPLEMENTED;
			}
	}

	return NO_ERROR;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	(void)argc;
	(void)argv;

	cryptoplus::crypto_initializer crypto_initializer;
	cryptoplus::algorithms_initializer algorithms_initializer;
	cryptoplus::error::error_strings_initializer error_strings_initializer;

	service_context ctx;

	ctx.service_status.dwServiceType = SERVICE_WIN32;
	ctx.service_status.dwCurrentState = SERVICE_STOPPED;
	ctx.service_status.dwControlsAccepted = 0;
	ctx.service_status.dwWin32ExitCode = NO_ERROR;
	ctx.service_status.dwServiceSpecificExitCode = NO_ERROR;
	ctx.service_status.dwCheckPoint = 0;
	ctx.service_status.dwWaitHint = 0;

	ctx.service_status_handle = ::RegisterServiceCtrlHandlerEx(SERVICE_NAME, &HandlerEx, &ctx);

	if (ctx.service_status_handle != 0)
	{
		ctx.service_status.dwCurrentState = SERVICE_START_PENDING;

		// Start pending
		::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

		//TODO: Initialization
		boost::asio::io_service io_service;

		// Running
		ctx.service_status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ctx.service_status.dwCurrentState = SERVICE_RUNNING;
		::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

		io_service.run();

		// Stop
		ctx.service_status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ctx.service_status.dwCurrentState = SERVICE_STOPPED;
		::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);
	}
}

void RunService()
{
	TCHAR service_name[] = SERVICE_NAME;

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{service_name, &ServiceMain},
		{NULL, NULL}
	};

	std::cout << "Starting service..." << std::endl;

	if (!::StartServiceCtrlDispatcher(ServiceTable))
	{
		DWORD last_error = ::GetLastError();

		switch (last_error)
		{
			case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
				throw std::runtime_error("This program is supposed to run as a Windows service.");
			default:
				throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "StartServiceCtrlDispatcher()");
		}
	}
}

void InstallService()
{
	SC_HANDLE service_control_manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (service_control_manager)
	{
		try
		{
			const fs::path path = get_module_filename();

			SC_HANDLE service = ::CreateService(
			                        service_control_manager,
			                        SERVICE_NAME,
			                        SERVICE_NAME,
			                        SERVICE_ALL_ACCESS,
			                        SERVICE_WIN32_OWN_PROCESS,
			                        SERVICE_AUTO_START,
			                        SERVICE_ERROR_IGNORE,
			                        path.string<std::basic_string<TCHAR> >().c_str(),
			                        NULL,
			                        NULL,
			                        NULL,
			                        NULL,
			                        NULL
			                    );

			if (service)
			{
				std::cout << "Service installed." << std::endl;

				::CloseServiceHandle(service);
			}
			else
			{
				DWORD last_error = ::GetLastError();

				switch (last_error)
				{
					case ERROR_SERVICE_EXISTS:
						std::cout << "The service is already installed. Ignoring." << std::endl;
						break;
					default:
						throw boost::system::system_error(last_error, boost::system::system_category(), "CreateService()");
				}
			}
		}
		catch (...)
		{
			::CloseServiceHandle(service_control_manager);

			throw;
		}

		::CloseServiceHandle(service_control_manager);
	}
	else
	{
		throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "OpenSCManager()");
	}
}

void UninstallService()
{
	SC_HANDLE service_control_manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (service_control_manager)
	{
		try
		{
			SC_HANDLE service = ::OpenService(
			                        service_control_manager,
			                        SERVICE_NAME,
			                        SERVICE_QUERY_STATUS | DELETE
			                    );

			if (service)
			{
				try
				{
					SERVICE_STATUS service_status;

					if (::QueryServiceStatus(service, &service_status))
					{
						if (service_status.dwCurrentState == SERVICE_STOPPED)
						{
							if (::DeleteService(service))
							{
								std::cout << "Service uninstalled." << std::endl;
							}
							else
							{
								DWORD last_error = ::GetLastError();

								switch (last_error)
								{
									case ERROR_SERVICE_MARKED_FOR_DELETE:
										std::cout << "The service has already been marked for deletion. Ignoring." << std::endl;
										break;
									default:
										throw boost::system::system_error(last_error, boost::system::system_category(), "DeleteService()");
								}
							}
						}
						else
						{
							std::cout << "The service is still running. Doing nothing." << std::endl;
						}
					}
					else
					{
						throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "QueryServiceStatus()");
					}
				}
				catch (...)
				{
					::CloseServiceHandle(service);

					throw;
				}

				::CloseServiceHandle(service);
			}
			else
			{
				DWORD last_error = ::GetLastError();

				switch (last_error)
				{
					case ERROR_SERVICE_DOES_NOT_EXIST:
						std::cout << "The service is not currently installed. Ignoring." << std::endl;
						break;
					default:
						throw boost::system::system_error(last_error, boost::system::system_category(), "OpenService()");
				}
			}
		}
		catch (...)
		{
			::CloseServiceHandle(service_control_manager);

			throw;
		}

		::CloseServiceHandle(service_control_manager);
	}
	else
	{
		throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "OpenSCManager()");
	}
}

int main(int argc, char** argv)
{
	po::options_description generic_options("Generic options");

	generic_options.add_options()
	("help,h", "Produce help message.")
	("install", "Install the service.")
	("uninstall", "Uninstall the service.")
	;

	po::variables_map vm;

	try
	{
		po::store(po::parse_command_line(argc, argv, generic_options), vm);
		po::notify(vm);
	}
	catch (boost::program_options::error& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		std::cerr << generic_options << std::endl;

		return EXIT_FAILURE;
	}

	if (vm.count("help"))
	{
		std::cout << generic_options << std::endl;

		return EXIT_SUCCESS;
	}

	try
	{
		if (vm.count("install"))
		{
			if (vm.count("uninstall"))
			{
				std::cerr << "Cannot specify both --install and --uninstall options." << std::endl;

				return EXIT_FAILURE;
			}

			InstallService();
		}
		else if (vm.count("uninstall"))
		{
			UninstallService();
		}
		else
		{
			RunService();
		}
	}
	catch (boost::system::system_error& ex)
	{
		std::cerr << ex.code() << ":" << ex.what() << std::endl;

		return EXIT_FAILURE;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


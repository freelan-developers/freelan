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
 * \brief Windows related service functions.
 */

#include "service.hpp"

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <cryptoplus/cryptoplus.hpp>
#include <cryptoplus/error/error_strings.hpp>

#include <freelan/freelan.hpp>
#include <freelan/logger_stream.hpp>

#include "../tools.hpp"
#include "../system.hpp"
#include "../configuration_helper.hpp"

namespace fs = boost::filesystem;
namespace fl = freelan;

#ifdef UNICODE
#define SERVICE_NAME L"FreeLAN Service"
#define SERVICE_DEPENDENCIES L"tap0901\0Dhcp\0"
#define SERVICE_START_NAME NULL
#else
#define SERVICE_NAME "FreeLAN Service"
#define SERVICE_DEPENDENCIES "tap0901\0Dhcp\0"
#define SERVICE_START_NAME NULL
#endif

namespace win32
{
	namespace
	{
		/* Local types */
		struct service_configuration
		{
			fs::path configuration_file;
			bool debug;
			fs::path log_file;
		};

		struct service_context
		{
			SERVICE_STATUS_HANDLE service_status_handle;
			SERVICE_STATUS service_status;
			boost::mutex stop_function_mutex;
			boost::function<void ()> stop_function;
		};
	}

	/* Local functions declarations */
	void parse_service_options(int argc, LPTSTR* argv, service_configuration& configuration);
	fl::logger create_logger(const service_configuration& configuration);
	void log_function(boost::shared_ptr<std::ostream> os, fl::log_level level, const std::string& msg);
	fl::configuration get_freelan_configuration(const service_configuration& configuration);
	DWORD WINAPI handler_ex(DWORD control, DWORD event_type, void* event_data, void* context);
	VOID WINAPI service_main(DWORD argc, LPTSTR* argv);

	/* Exposed functions definitions */
	bool run_service()
	{
		TCHAR service_name[] = SERVICE_NAME;

		SERVICE_TABLE_ENTRY service_table[] =
		{
			{service_name, &service_main},
			{NULL, NULL}
		};

		if (!::StartServiceCtrlDispatcher(service_table))
		{
			DWORD last_error = ::GetLastError();

			switch (last_error)
			{
				case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
					return false;
				default:
					throw boost::system::system_error(::GetLastError(), boost::system::system_category(), "StartServiceCtrlDispatcher()");
			}
		}

		return true;
	}

	bool install_service()
	{
		bool result = false;

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
				                        SERVICE_ERROR_NORMAL,
				                        path.string<std::basic_string<TCHAR> >().c_str(),
				                        NULL,
				                        NULL,
				                        SERVICE_DEPENDENCIES,
				                        SERVICE_START_NAME,
				                        NULL
				                    );

				if (service)
				{
					result = true;

					::CloseServiceHandle(service);
				}
				else
				{
					DWORD last_error = ::GetLastError();

					switch (last_error)
					{
						case ERROR_SERVICE_EXISTS:
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
		
		return result;
	}

	bool uninstall_service()
	{
		bool result = false;

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
						if (::DeleteService(service))
						{
							result = true;
						}
						else
						{
							DWORD last_error = ::GetLastError();

							switch (last_error)
							{
								case ERROR_SERVICE_MARKED_FOR_DELETE:
									break;
								default:
									throw boost::system::system_error(last_error, boost::system::system_category(), "DeleteService()");
							}
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

		return result;
	}

	/* Local functions definitions */
	void parse_service_options(int argc, LPTSTR* argv, service_configuration& configuration)
	{
		namespace po = boost::program_options;

		po::options_description service_options("Service options");
		service_options.add_options()
		("configuration_file,c", po::value<std::string>(), "The configuration file to use")
		("debug,d", "Enables debug output.")
		("log_file,l", po::value<std::string>(), "The log file to use")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, service_options), vm);
		po::notify(vm);

		const fs::path execution_root_directory = get_execution_root_directory();

		if (vm.count("configuration_file"))
		{
			configuration.configuration_file = fs::absolute(vm["configuration_file"].as<std::string>(), execution_root_directory);
		}

		configuration.debug = (vm.count("debug") > 0);

		if (vm.count("log_file"))
		{
			configuration.log_file = fs::absolute(vm["log_file"].as<std::string>(), execution_root_directory);
		}
		else
		{
			configuration.log_file = execution_root_directory / "log" / "freelan.log";
		}
	}

	fl::logger create_logger(const service_configuration& configuration)
	{
		if (configuration.log_file.empty())
		{
			return fl::logger();
		}
		else
		{
			boost::shared_ptr<std::ostream> log_stream = boost::make_shared<fs::basic_ofstream<char> >(configuration.log_file);

			return fl::logger(boost::bind(&log_function, log_stream, _1, _2), configuration.debug ? fl::LL_DEBUG : fl::LL_INFORMATION);
		}
	}

	void log_function(boost::shared_ptr<std::ostream> os, fl::log_level level, const std::string& msg)
	{
		if (os)
		{
			(*os) << boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time()) << " [" << log_level_to_string(level) << "] " << msg << std::endl;
		}
	}

	fl::configuration get_freelan_configuration(const service_configuration& configuration)
	{
		namespace po = boost::program_options;

		po::options_description configuration_options("Configuration");
		configuration_options.add(get_fscp_options());
		configuration_options.add(get_security_options());
		configuration_options.add(get_tap_adapter_options());
		configuration_options.add(get_switch_options());

		const fs::path execution_root_directory = get_execution_root_directory();

		fl::configuration fl_configuration;

		po::variables_map vm;

		fs::path configuration_file = configuration.configuration_file;

		if (configuration_file.empty())
		{
			configuration_file = execution_root_directory / "config" / "freelan.cfg";
		}

		fs::basic_ifstream<char> ifs(configuration_file);

		if (!ifs)
		{
			throw std::runtime_error("Unable to open the specified configuration file: " + configuration_file.string());
		}

		po::store(po::parse_config_file(ifs, configuration_options, true), vm);

		po::notify(vm);

		setup_configuration(fl_configuration, execution_root_directory, vm);

		const fs::path tap_adapter_up_script = get_tap_adapter_up_script(execution_root_directory, vm);

		if (!tap_adapter_up_script.empty())
		{
			fl_configuration.tap_adapter.up_callback = boost::bind(&execute_tap_adapter_up_script, tap_adapter_up_script, _1);
		}

		const fs::path tap_adapter_down_script = get_tap_adapter_down_script(execution_root_directory, vm);

		if (!tap_adapter_down_script.empty())
		{
			fl_configuration.tap_adapter.down_callback = boost::bind(&execute_tap_adapter_down_script, tap_adapter_down_script, _1);
		}

		const fs::path certificate_validation_script = get_certificate_validation_script(execution_root_directory, vm);

		if (!certificate_validation_script.empty())
		{
			fl_configuration.security.certificate_validation_callback = boost::bind(&execute_certificate_validation_script, certificate_validation_script, _1, _2);
		}

		return fl_configuration;
	}

	DWORD WINAPI handler_ex(DWORD control, DWORD event_type, void* event_data, void* context)
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
				{
					boost::lock_guard<boost::mutex> lock(ctx.stop_function_mutex);

					if (ctx.stop_function)
					{
						ctx.stop_function();
						ctx.stop_function = NULL;
					}

					ctx.service_status.dwCurrentState = SERVICE_STOP_PENDING;
					::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

					return NO_ERROR;
				}
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

	VOID WINAPI service_main(DWORD argc, LPTSTR* argv)
	{
		service_configuration configuration;

		parse_service_options(argc, argv, configuration);

		fl::logger logger = create_logger(configuration);

		logger(fl::LL_INFORMATION) << "Log starts at " << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());

		/* Initializations */
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

		ctx.service_status_handle = ::RegisterServiceCtrlHandlerEx(SERVICE_NAME, &handler_ex, &ctx);

		if (ctx.service_status_handle != 0)
		{
			ctx.service_status.dwCurrentState = SERVICE_START_PENDING;

			// Start pending
			::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

			try
			{
				boost::asio::io_service io_service;

				fl::configuration fl_configuration = get_freelan_configuration(configuration);
				freelan::core core(io_service, fl_configuration, logger);

				core.open();

				boost::unique_lock<boost::mutex> lock(ctx.stop_function_mutex);

				ctx.stop_function = boost::bind(&fl::core::close, boost::ref(core));

				lock.unlock();

				// Running
				ctx.service_status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
				ctx.service_status.dwCurrentState = SERVICE_RUNNING;
				::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

				io_service.run();

				lock.lock();

				ctx.stop_function = NULL;

				lock.unlock();
			}
			catch (boost::system::system_error& ex)
			{
				ctx.service_status.dwWin32ExitCode = ex.code().value();
				::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

				logger(fl::LL_ERROR) << "Error: " << ex.code() << ":" << ex.code().message() << ":" << ex.what();
			}
			catch (std::exception& ex)
			{
				ctx.service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
				ctx.service_status.dwServiceSpecificExitCode = 1;
				::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);

				logger(fl::LL_ERROR) << "Error: " << ex.what();
			}

			// Stop
			ctx.service_status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
			ctx.service_status.dwCurrentState = SERVICE_STOPPED;
			::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);
		}

		logger(fl::LL_INFORMATION) << "Log stops at " << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
	}
}

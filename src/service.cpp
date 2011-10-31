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

#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>

#include <windows.h>

#include "common/tools.hpp"

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

	boost::asio::io_service io_service;

	service_context ctx;

	ctx.service_status.dwServiceType = SERVICE_WIN32;
	ctx.service_status.dwCurrentState = SERVICE_STOPPED;
	ctx.service_status.dwControlsAccepted = 0;
	ctx.service_status.dwWin32ExitCode = NO_ERROR;
	ctx.service_status.dwServiceSpecificExitCode = NO_ERROR;
	ctx.service_status.dwCheckPoint = 0;
	ctx.service_status.dwWaitHint = 0;

	ctx.service_status_handle = ::RegisterServiceCtrlHandlerEx("FreeLAN Service", &HandlerEx, &ctx);

	if (ctx.service_status_handle != 0)
	{
		ctx.service_status.dwCurrentState = SERVICE_START_PENDING;

		// Start pending
		::SetServiceStatus(ctx.service_status_handle, &ctx.service_status);
		
		//TODO: Initialization

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

int main()
{
	char service_name[] = "FreeLAN Service";

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{service_name, &ServiceMain},
		{NULL, NULL}
	};

	::StartServiceCtrlDispatcher(ServiceTable);

	return EXIT_SUCCESS;
}

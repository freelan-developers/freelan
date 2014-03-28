/*
 * libfreelan - open-source and multi-platform peer-to-peer VPN library.
 * Copyright 2007-2010 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file tap_setup.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The tap-setup main file.
 */

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h>
#include <regstr.h>
#include <infstr.h>
#include <cfgmgr32.h>
#include <string.h>
#include <malloc.h>
#include <newdev.h>
#include <objbase.h>
#include <strsafe.h>

#include <list>
#include <string>
#include <iostream>

#define EXIT_REBOOT_REQUIRED 2

typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesFunc)(HWND hwndParent, LPCTSTR HardwareId, LPCTSTR FullInfPath, DWORD InstallFlags, PBOOL bRebootRequired);

bool reboot();
bool install(const std::wstring& inf, const std::wstring& hardware_id, bool& reboot_required);
bool update(const std::wstring& inf, const std::wstring& hardware_id, bool& reboot_required);
bool remove(const std::wstring& class_name, bool& reboot_required);
LPTSTR* getDeviceProperty(HDEVINFO devices, const SP_DEVINFO_DATA& device_info, DWORD property);

bool reboot()
{
	HANDLE token;
	TOKEN_PRIVILEGES new_privileges;
	LUID luid;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
	{
		if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
		{
			new_privileges.PrivilegeCount = 1;
			new_privileges.Privileges[0].Luid = luid;
			new_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			AdjustTokenPrivileges(token, FALSE, &new_privileges, 0, NULL, NULL);
		}

		CloseHandle(token);
	}

#pragma warning(suppress: 28159)
	return (InitiateSystemShutdownEx(NULL, NULL, 0, FALSE, TRUE, REASON_PLANNED_FLAG | REASON_HWINSTALL) == TRUE);
}

bool install(const std::wstring& inf, const std::wstring& hardware_id, bool& reboot_required)
{
	TCHAR inf_path[MAX_PATH];

	if (GetFullPathName(inf.c_str(), MAX_PATH, inf_path, NULL) >= MAX_PATH)
	{
		return false;
	}

	std::wcerr << "INF file full path: " << inf_path << std::endl;

	bool result = false;

	TCHAR hardware_id_list[LINE_LEN + 4];
	ZeroMemory(hardware_id_list, sizeof(hardware_id_list));

	if (SUCCEEDED(StringCchCopy(hardware_id_list, LINE_LEN, hardware_id.c_str())))
	{
		std::wcerr << "Hardware identifier list: " << hardware_id_list << std::endl;

		GUID class_guid;
		TCHAR class_name[MAX_CLASS_NAME_LEN];

		if (SetupDiGetINFClass(inf_path, &class_guid, class_name, sizeof(class_name) / sizeof(class_name[0]), 0))
		{
			std::wcerr << "Class name: " << class_name << std::endl;

			HDEVINFO device_info_set = INVALID_HANDLE_VALUE;

			device_info_set = SetupDiCreateDeviceInfoList(&class_guid, 0);

			if (device_info_set != INVALID_HANDLE_VALUE)
			{
				std::wcerr << "Device information set created." << std::endl;

				SP_DEVINFO_DATA device_info_data;
				device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

				if (SetupDiCreateDeviceInfo(device_info_set,
							class_name,
							&class_guid,
							NULL,
							0,
							DICD_GENERATE_ID,
							&device_info_data))
				{
					std::wcerr << "Device information element created." << std::endl;

					if (SetupDiSetDeviceRegistryProperty(device_info_set,
								&device_info_data,
								SPDRP_HARDWAREID,
								(LPBYTE)hardware_id_list,
								(lstrlen(hardware_id_list) + 1 + 1) * sizeof(TCHAR)))
					{
						std::wcerr << "Hardware id set." << std::endl;

						if (SetupDiCallClassInstaller(DIF_REGISTERDEVICE, device_info_set, &device_info_data))
						{
							std::wcerr << "Device registered." << std::endl;

							result = update(inf, hardware_id, reboot_required);
						}
					}
				}

				SetupDiDestroyDeviceInfoList(device_info_set);
			}
		}
	}

	return result;
}

bool update(const std::wstring& inf, const std::wstring& hardware_id, bool& reboot_required)
{
	TCHAR inf_path[MAX_PATH];

	if (GetFullPathName(inf.c_str(), MAX_PATH, inf_path, NULL) >= MAX_PATH)
	{
		return false;
	}

	if (GetFileAttributes(inf_path) == (DWORD)(-1))
	{
		return false;
	}

	bool result = false;

	DWORD flags = INSTALLFLAG_FORCE;

	HMODULE new_device_module = LoadLibrary(TEXT("newdev.dll"));

	if (new_device_module)
	{
		std::wcerr << "Loaded newdev.dll." << std::endl;

		UpdateDriverForPlugAndPlayDevicesFunc update_function = 
			reinterpret_cast<UpdateDriverForPlugAndPlayDevicesFunc>(
					GetProcAddress(new_device_module, "UpdateDriverForPlugAndPlayDevicesW")
					);

		if (update_function)
		{
			std::wcerr << "Update function loaded." << std::endl;

			BOOL reboot = FALSE;

			if (update_function(NULL, hardware_id.c_str(), inf_path, flags, &reboot))
			{
				std::wcerr << "Update function suceeded. Device is up to date." << std::endl;

				reboot_required = (reboot == TRUE);

				result = true;
			} else
			{
				std::wcerr << "Unable to update the device." << std::endl;
			}
		} else
		{
			std::wcerr << "Unable to load the update function." << std::endl;
		}

		FreeLibrary(new_device_module);
	}

	return result;
}

bool getDeviceProperty(HDEVINFO devices, SP_DEVINFO_DATA& device_info, DWORD property, std::wstring& value)
{
	DWORD size = 8192;

	try
	{
		DWORD data_type;
		DWORD requested_size;
		LPTSTR buffer = new TCHAR[(size / sizeof(TCHAR)) + 1];
		memset(buffer, 0x00, (size / sizeof(TCHAR)) + 1);

		if (buffer)
		{
			bool success = true;

			while (!SetupDiGetDeviceRegistryProperty(devices, &device_info, property, &data_type, reinterpret_cast<LPBYTE>(buffer), size, &requested_size))
			{
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				{
					success = false;
					break;
				}

				if (data_type != REG_SZ)
				{
					success = false;
					break;
				}

				size = requested_size;
				delete[] buffer;
				buffer = new TCHAR[(size / sizeof(TCHAR)) + 1];
				memset(buffer, 0x00, (size / sizeof(TCHAR)) + 1);

				if (!buffer)
				{
					success = false;
					break;
				}
			}

			if (success)
			{
				value = std::wstring(buffer);
			}

			delete[] buffer;

			return true;
		}
	}
	catch (...)
	{
		std::wcerr << "Allocation error. This is serious !" << std::endl;
	}

	return false;
}

bool getDeviceProperty(HDEVINFO devices, SP_DEVINFO_DATA& device_info, DWORD property, std::list<std::wstring>& values)
{
	DWORD size = 8192;

	try
	{
		DWORD data_type;
		DWORD requested_size;
		LPTSTR buffer = new TCHAR[(size / sizeof(TCHAR)) + 2];
		memset(buffer, 0x00, (size / sizeof(TCHAR)) + 2);

		if (buffer)
		{
			bool success = true;

			while (!SetupDiGetDeviceRegistryProperty(devices, &device_info, property, &data_type, reinterpret_cast<LPBYTE>(buffer), size, &requested_size))
			{
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				{
					success = false;
					break;
				}

				if (data_type != REG_MULTI_SZ)
				{
					success = false;
					break;
				}

				size = requested_size;
				delete[] buffer;
				buffer = new TCHAR[(size / sizeof(TCHAR)) + 2];
				memset(buffer, 0x00, (size / sizeof(TCHAR)) + 2);

				if (!buffer)
				{
					success = false;
					break;
				}
			}

			if (success)
			{
				values = std::list<std::wstring>();

				for (LPTSTR value = buffer; value[0]; value += lstrlen(value) + 1)
				{
					values.push_back(std::wstring(value));
				}
			}

			delete[] buffer;

			return true;
		}
	}
	catch (...)
	{
		std::wcerr << "Allocation error. This is serious !" << std::endl;
	}

	return false;
}

bool remove(const std::wstring& hardware_id, bool& reboot_required)
{
	bool result = false;

	HDEVINFO devices = SetupDiGetClassDevsEx(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT, NULL, NULL, NULL);

	if (devices != INVALID_HANDLE_VALUE)
	{
		std::wcerr << "Got device information set." << std::endl;

		SP_DEVINFO_LIST_DETAIL_DATA device_info_list_detail;
		memset(&device_info_list_detail, 0x00, sizeof(device_info_list_detail));
		device_info_list_detail.cbSize = sizeof(device_info_list_detail);

		if (SetupDiGetDeviceInfoListDetail(devices, &device_info_list_detail))
		{
			std::wcerr << "Got device information list details." << std::endl;

			SP_DEVINFO_DATA device_info;
			device_info.cbSize = sizeof(device_info);

			for (DWORD index = 0; SetupDiEnumDeviceInfo(devices, index, &device_info); ++index)
			{
				TCHAR device_id[MAX_DEVICE_ID_LEN];

				if (CM_Get_Device_ID_Ex(device_info.DevInst, device_id, MAX_DEVICE_ID_LEN, 0, device_info_list_detail.RemoteMachineHandle) == CR_SUCCESS)
				{
					std::list<std::wstring> device_hardware_id_list;

					if (getDeviceProperty(devices, device_info, SPDRP_HARDWAREID, device_hardware_id_list))
					{
						bool match = false;

						for (std::list<std::wstring>::const_iterator device_hardware_id = device_hardware_id_list.begin(); device_hardware_id != device_hardware_id_list.end(); ++device_hardware_id)
						{
							if (*device_hardware_id == hardware_id)
							{
								match = true;

								break;
							}
						}

						if (match)
						{
							std::wstring friendly_name;

							if (getDeviceProperty(devices, device_info, SPDRP_FRIENDLYNAME, friendly_name) && (friendly_name.length() > 0))
							{
								std::wcerr << "Removing device: " << friendly_name << " (" << device_id << ")" << std::endl;
							} else
							{
								std::wcerr << "Removing device: " << device_id << std::endl;
							}

							SP_REMOVEDEVICE_PARAMS remove_device_params;
							remove_device_params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
							remove_device_params.ClassInstallHeader.InstallFunction = DIF_REMOVE;
							remove_device_params.Scope = DI_REMOVEDEVICE_GLOBAL;
							remove_device_params.HwProfile = 0;

							result = true;

							if (!SetupDiSetClassInstallParams(devices, &device_info, &remove_device_params.ClassInstallHeader, sizeof(remove_device_params)) || 
									!SetupDiCallClassInstaller(DIF_REMOVE, devices, &device_info))
							{
								std::wcerr << "Failed to set the class installer." << std::endl;

								result = false;
							}

							if (result)
							{
								reboot_required = false;

								SP_DEVINSTALL_PARAMS device_params;

								if (SetupDiGetDeviceInstallParams(devices, &device_info, &device_params))
								{
									if (device_params.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))
									{
										reboot_required = true;
									}
								}
							}
						}
					}
				}
			}
		}

		SetupDiDestroyDeviceInfoList(devices);
	}

	return result;
}

int _cdecl wmain(int argc, wchar_t** argv)
{
	bool show_usage = false;
	int exit_code = EXIT_FAILURE;
	bool reboot_required = false;

	if (argc <= 1)
	{
		show_usage = true;
	} else
	{
		std::wstring command(argv[1]);

		if (command == TEXT("install"))
		{
			if (argc <= 3)
			{
				show_usage = true;
			} else
			{
				std::wstring inf(argv[2]);
				std::wstring hardware_id(argv[3]);

				if (install(inf, hardware_id, reboot_required))
				{
					exit_code = reboot_required ? EXIT_REBOOT_REQUIRED : EXIT_SUCCESS;

					std::wcerr << "Installation of \"" << inf << "\" (" << hardware_id << ") succeeded." << std::endl;
					
					if (reboot_required)
					{
						std::wcerr << "A reboot is required to complete the operation." << std::endl;
					}
				} else
				{
					std::wcerr << "Installation of \"" << inf << "\" (" << hardware_id << ") failed." << std::endl;
				}
			}
		} else if (command == TEXT("remove"))
		{
			if (argc <= 2)
			{
				show_usage = true;
			} else
			{
				std::wstring hardware_id(argv[2]);

				if (remove(hardware_id, reboot_required))
				{
					exit_code = reboot_required ? EXIT_REBOOT_REQUIRED : EXIT_SUCCESS;

					std::wcerr << "Removal of \"" << hardware_id << "\" succeeded." << std::endl;

					if (reboot_required)
					{
						std::wcerr << "A reboot is required to complete the operation." << std::endl;
					}
				} else
				{
					std::wcerr << "Removal of \"" << hardware_id << "\" failed." << std::endl;
				}
			}
		} else
		{
			std::wcerr << "Unknown command \"" << command << "\"." << std::endl;
			show_usage = true;
		}
	}

	if (show_usage)
	{
		std::wcerr << "Usage:\ttap-setup.exe install <inf> <hardware_id>" << std::endl;
		std::wcerr << "      \ttap-setup.exe remove <hardware_id>" << std::endl;
		std::wcerr << std::endl;
		std::wcerr << "Returns 0 on success, 1 on failure and 2 if a reboot is required to complete the operation." << std::endl;

		exit_code = EXIT_FAILURE;
	}

	return exit_code;
}



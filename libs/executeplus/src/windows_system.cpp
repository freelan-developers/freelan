/*
 * libexecuteplus - A portable execution library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libexecuteplus.
 *
 * libexecuteplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libexecuteplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libexecuteplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file windows_system.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Windows system primitives.
 */

#include "windows_system.hpp"

#ifdef WINDOWS

#include "error.hpp"

#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <memory>

#include <boost/lexical_cast.hpp>

#include <shlobj.h>
#include <shellapi.h>

namespace executeplus
{
	namespace
	{
		template <typename CharType>
		struct argument_helper;

		template <>
		struct argument_helper<char>
		{
			static const char ESCAPE_CHARACTER = '\\';
			static const char QUOTE_CHARACTER = '"';
			static const char EQUAL_CHARACTER = '=';
			static const char NULL_CHARACTER = '\0';
		};

		template <>
		struct argument_helper<wchar_t>
		{
			static const wchar_t ESCAPE_CHARACTER = L'\\';
			static const wchar_t QUOTE_CHARACTER = L'"';
			static const wchar_t EQUAL_CHARACTER = L'=';
			static const wchar_t NULL_CHARACTER = L'\0';
		};

		struct EnvironmentStringsDeleter
		{
			void operator()(wchar_t* p)
			{
				::FreeEnvironmentStringsW(p);
			}

			void operator()(char* p)
			{
				::FreeEnvironmentStringsA(p);
			}
		};

		template <typename CharType>
		std::unique_ptr<CharType, EnvironmentStringsDeleter> get_environment_strings();
		
		template <>
		std::unique_ptr<wchar_t, EnvironmentStringsDeleter> get_environment_strings<wchar_t>()
		{
			return std::unique_ptr<wchar_t, EnvironmentStringsDeleter>(::GetEnvironmentStringsW());
		}

#ifndef UNICODE
		template <>
		std::unique_ptr<char, EnvironmentStringsDeleter> get_environment_strings<char>()
		{
			return std::unique_ptr<char, EnvironmentStringsDeleter>(::GetEnvironmentStringsA());
		}
#endif

		template <typename CharType>
		std::map<std::basic_string<CharType>, std::basic_string<CharType>> get_current_environment()
		{
			typedef std::basic_string<CharType> string_type;

			std::map<string_type, string_type> result;

			const auto environment_strings = get_environment_strings<CharType>();
			auto ptr = environment_strings.get();

			if (ptr)
			{
				while (*ptr)
				{
					const string_type line(ptr);

					// Environment variables that start with an equal sign are private to the shell and should be ignored.
					if (*ptr != argument_helper<CharType>::EQUAL_CHARACTER)
					{
						const auto pos = line.find(argument_helper<CharType>::EQUAL_CHARACTER);

						if (pos == string_type::npos)
						{
							// Discard the result.
						}
						else
						{
							const string_type key = line.substr(0, pos);
							const string_type value = line.substr(pos + 1);
							result[key] = value;
						}
					}

					ptr += line.size() + 1;
				}
			}

			return result;
		}

		class handle_closer
		{
			public:
				handle_closer(HANDLE handle) : m_handle(handle) {}
				~handle_closer() { ::CloseHandle(m_handle); }

			private:
				HANDLE m_handle;
		};

		void output(const char* str)
		{
			std::cout << str << std::endl;
		}

		void output(const wchar_t* str)
		{
			std::wcout << str << std::endl;
		}

		bool has_escapable_characters(const std::string& str)
		{
			return (str.find_first_of(" \t\n\v\"") != std::string::npos);
		}

		bool has_escapable_characters(const std::wstring& str)
		{
			return (str.find_first_of(L" \t\n\v\"") != std::string::npos);
		}

		template <typename CharType>
		std::basic_string<CharType> escape_argument(const std::basic_string<CharType>& arg)
		{
			std::basic_string<CharType> result(1, argument_helper<CharType>::QUOTE_CHARACTER);

			for (auto it = arg.begin();; ++it)
			{
				unsigned int escapes_count = 0;

				while ((it != arg.end()) && (*it == argument_helper<CharType>::ESCAPE_CHARACTER))
				{
					++it;
					++escapes_count;
				}

				if (it == arg.end())
				{
					result.append(escapes_count * 2, argument_helper<CharType>::ESCAPE_CHARACTER);
					break;
				}
				else if (*it == argument_helper<CharType>::QUOTE_CHARACTER)
				{
					result.append(escapes_count * 2 + 1, argument_helper<CharType>::ESCAPE_CHARACTER);
					result.push_back(*it);
				}
				else
				{
					result.append(escapes_count, argument_helper<CharType>::ESCAPE_CHARACTER);
					result.push_back(*it);
				}
			}

			result.push_back(argument_helper<CharType>::QUOTE_CHARACTER);

			return result;
		}

		template <typename CharType>
		std::basic_string<CharType> escape_argument_if_needed(const std::basic_string<CharType>& arg)
		{
			if (!arg.empty() && !has_escapable_characters(arg))
			{
				return arg;
			}
			else
			{
				return escape_argument(arg);
			}
		}

		DWORD do_create_process(const char* application, char* command_line, char* env, STARTUPINFOA& si, PROCESS_INFORMATION& pi)
		{
			return ::CreateProcessA(application, command_line, NULL, NULL, FALSE, 0, env, NULL, &si, &pi);
		}

		DWORD do_create_process(const wchar_t* application, wchar_t* command_line, wchar_t* env, STARTUPINFOW& si, PROCESS_INFORMATION& pi)
		{
			return ::CreateProcessW(application, command_line, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, env, NULL, &si, &pi);
		}

		template <typename CharType>
		struct startupinfo;

		template <>
		struct startupinfo<char>
		{
			typedef STARTUPINFOA type;
		};

		template <>
		struct startupinfo<wchar_t>
		{
			typedef STARTUPINFOW type;
		};

		template <typename CharType>
		DWORD create_process(const CharType* application, CharType* command_line, CharType* env)
		{
			DWORD exit_status;

			typename startupinfo<CharType>::type si;
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
#if FREELAN_DEBUG
			si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
#else
			si.hStdOutput = INVALID_HANDLE_VALUE;
#endif
			si.hStdError = INVALID_HANDLE_VALUE;

			PROCESS_INFORMATION pi;

#if FREELAN_DEBUG
			output(command_line);
#endif

			if (!do_create_process(application, command_line, env, si, pi))
			{
				throw boost::system::system_error(::GetLastError(), boost::system::system_category());
			}

			handle_closer thread_closer(pi.hThread);
			handle_closer process_closer(pi.hProcess);

			DWORD wait_result = ::WaitForSingleObject(pi.hProcess, INFINITE);

			switch (wait_result)
			{
				case WAIT_OBJECT_0:
					{
						DWORD exit_code = 0;

						if (::GetExitCodeProcess(pi.hProcess, &exit_code))
						{
							exit_status = static_cast<int>(exit_code);
						}
						else
						{
							throw boost::system::system_error(::GetLastError(), boost::system::system_category());
						}

						break;
					}
				default:
					{
						throw boost::system::system_error(::GetLastError(), boost::system::system_category());
					}
			}

			return exit_status;
		}

		template <typename CharType>
		int do_execute(const std::vector<std::basic_string<CharType>>& args, const std::map<std::basic_string<CharType>, std::basic_string<CharType>>& env, boost::system::error_code& ec)
		{
			if (args.empty())
			{
				ec = make_error_code(executeplus_error::external_process_failed);

				return EXIT_FAILURE;
			}

			const std::basic_string<CharType> application = args.front();
			std::basic_ostringstream<CharType> command_line_buffer;
			std::basic_ostringstream<CharType> environment_string_buffer;

			for (auto it = args.begin(); it != args.end(); ++it)
			{
				if (it != args.begin())
				{
					command_line_buffer << " ";
				}

				command_line_buffer << escape_argument_if_needed(*it);
			}

			for (auto&& pair : env)
			{
				environment_string_buffer << pair.first << argument_helper<CharType>::EQUAL_CHARACTER << pair.second << argument_helper<CharType>::NULL_CHARACTER;
			}

			std::basic_string<CharType> command_line = command_line_buffer.str();
			std::basic_string<CharType> environment_string = environment_string_buffer.str();

			return create_process<CharType>(application.c_str(), command_line.empty() ? nullptr : &command_line[0], environment_string.empty() ? nullptr : &environment_string[0]);
		}

		template <typename CharType>
		int do_execute(const std::vector<std::basic_string<CharType>>& args, const std::map<std::basic_string<CharType>, std::basic_string<CharType>>& env)
		{
			boost::system::error_code ec;

			const auto result = do_execute(args, env, ec);

			if (result < 0)
			{
				throw boost::system::system_error(ec);
			}

			return result;
		}

		template <typename CharType>
		void do_checked_execute(const std::vector<std::basic_string<CharType>>& args, const std::map<std::basic_string<CharType>, std::basic_string<CharType>>& env)
		{
			if (do_execute(args, env) != 0)
			{
				throw boost::system::system_error(make_error_code(executeplus_error::external_process_failed));
			}
		}
	}

#ifdef UNICODE
	std::map<std::wstring, std::wstring> get_current_environment()
	{
		return get_current_environment<wchar_t>();
	}
#else
	std::map<std::string, std::string> get_current_environment()
	{
		return get_current_environment<char>();
	}
#endif

#ifdef UNICODE
	int execute(const std::vector<std::wstring>& args, const std::map<std::wstring, std::wstring>& env, boost::system::error_code& ec)
#else
	int execute(const std::vector<std::string>& args, const std::map<std::string, std::string>& env, boost::system::error_code& ec)
#endif
	{
		return do_execute(args, env, ec);
	}

#ifdef UNICODE
	int execute(const std::vector<std::wstring>& args, const std::map<std::wstring, std::wstring>& env)
#else
	int execute(const std::vector<std::string>& args, const std::map<std::string, std::string>& env)
#endif
	{
		return do_execute(args, env);
	}

#ifdef UNICODE
	void checked_execute(const std::vector<std::wstring>& args, const std::map<std::wstring, std::wstring>& env)
#else
	void checked_execute(const std::vector<std::string>& args, const std::map<std::string, std::string>& env)
#endif
	{
		do_checked_execute(args, env);
	}
}

#endif

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
 * \file windows_tap_adapter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The Windows tap adapter class.
 */

#include "windows/netsh.hpp"

#include <windows.h>

#include <executeplus/windows_system.hpp>
#include <executeplus/error.hpp>

namespace asiotap
{
	namespace
	{
		size_t get_system_directory(char* buf, size_t buf_len)
		{
			return static_cast<size_t>(::GetSystemDirectoryA(buf, static_cast<UINT>(buf_len)));
		}

		size_t get_system_directory(wchar_t* buf, size_t buf_len)
		{
			return static_cast<size_t>(::GetSystemDirectoryW(buf, static_cast<UINT>(buf_len)));
		}

		template <typename CharType>
		std::basic_string<CharType> get_system_directory()
		{
			const size_t required_size = get_system_directory(static_cast<CharType*>(NULL), 0);

			if (required_size == 0)
			{
				throw boost::system::system_error(::GetLastError(), boost::system::system_category());
			}

			std::basic_string<CharType> result;

			// We make room for the content and the trailing NULL character.
			result.resize(required_size + 1);

			const size_t new_size = get_system_directory(&result[0], result.size());

			if (new_size == 0)
			{
				throw boost::system::system_error(::GetLastError(), boost::system::system_category());
			}

			// Get rid of the trailing NULL character.
			result.resize(new_size);

			return result;
		}
	}

	std::wstring multi_byte_to_wide_char(const std::string& str)
	{
		if (str.empty())
		{
			return std::wstring();
		}

		size_t required_size = ::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, &str[0], str.size(), nullptr, 0);

		if (required_size == 0)
		{
			throw boost::system::system_error(::GetLastError(), boost::system::system_category());
		}

		std::wstring result(required_size, '\0');
		required_size = ::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, &str[0], str.size(), &result[0], result.capacity());

		if (required_size == 0)
		{
			throw boost::system::system_error(::GetLastError(), boost::system::system_category());
		}

		result.resize(required_size);
		return result;
	}

#ifdef UNICODE
	void netsh(const std::vector<std::wstring>& args)
#else
	void netsh(const std::vector<std::string>& args)
#endif
	{
#ifdef UNICODE
		std::vector<std::wstring> real_args = { get_system_directory<wchar_t>() + L"\\netsh.exe" };
#else
		std::vector<std::string> real_args = { get_system_directory<char>() + "\\netsh.exe" };
#endif

		real_args.insert(real_args.end(), args.begin(), args.end());
		const auto env = executeplus::get_current_environment();

		executeplus::checked_execute(real_args, env);
	}
}

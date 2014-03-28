/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file file.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A file class.
 */

#include <cstdio>

#include "file.hpp"

#include <cassert>

#ifdef WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#endif

namespace cryptoplus
{
	void secure_fclose(file::pointer ptr)
	{
		if (ptr)
		{
			fclose(ptr);
		}
	}

	template <>
	file::deleter_type pointer_wrapper<file::value_type>::deleter = secure_fclose;

	namespace
	{
#ifdef WINDOWS
		std::string get_last_error_string()
		{
			LPSTR msgbuf = NULL;

			FormatMessageA(
			    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			    NULL,
			    GetLastError(),
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			    (LPSTR)&msgbuf,
			    0,
			    NULL
			);

			boost::shared_ptr<CHAR> _msgbuf(msgbuf, LocalFree);

			std::string result(msgbuf, strlen(_msgbuf.get()));

			return result;
		}
#else
		std::string get_last_error_string()
		{
			return std::string(strerror(errno));
		}
#endif
	}

	file file::open(const std::string& filename, const std::string& mode)
	{
#ifdef MSV
		file::pointer ptr = 0;

		int err = fopen_s(&ptr, filename.c_str(), mode.c_str());

		if (err != 0)
		{
			ptr = 0;
		}
#else
		file::pointer ptr = fopen(filename.c_str(), mode.c_str());
#endif

		return take_ownership(ptr);
	}

#ifdef WINDOWS
	file file::open(const std::wstring& filename, const std::wstring& mode)
	{
#ifdef MSV
		file::pointer ptr = 0;

		int err = _wfopen_s(&ptr, filename.c_str(), mode.c_str());

		if (err != 0)
		{
			ptr = 0;
		}
#else
		file::pointer ptr = _wfopen(filename.c_str(), mode.c_str());
#endif

		return take_ownership(ptr);
	}
#endif

	file file::take_ownership(pointer _ptr)
	{
		if (!_ptr)
		{
			throw std::runtime_error(get_last_error_string());
		}

		return file(_ptr, deleter);
	}
}


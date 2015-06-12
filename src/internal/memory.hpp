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
 * \file memory.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Memory functions.
 */

#pragma once

#include <cstddef>

#include <freelan/memory.h>

#define FREELAN_NEW MarkPointer(__FILE__, __LINE__) * new
#define FREELAN_DELETE delete

class MarkPointer {
	public:
		MarkPointer(const char* _file, unsigned int line) :
			m_file(_file),
			m_line(line)
		{}

		template <typename T>
		T* operator*(T* ptr) {
			freelan_mark_pointer(ptr, m_file, m_line);

			return ptr;
		}

	private:
		const char* m_file;
		unsigned int m_line;
};

void* operator new(std::size_t n);

#if __cplusplus <= 199711L
#define DELETE_NOEXCEPT
#else
#define DELETE_NOEXCEPT noexcept
#endif

void operator delete(void* ptr) DELETE_NOEXCEPT;

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

#include "memory.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>

namespace freelan {

namespace {
	static void* default_malloc(size_t size) {
		return ::malloc(size);
	}

	static void* default_realloc(void* ptr, size_t size) {
		return ::realloc(ptr, size);
	}

	static void default_free(void* ptr) {
		return ::free(ptr);
	}

	static char* default_strdup(const char* str) {
#if _MSC_VER
		return ::_strdup(str);
#else
		return ::strdup(str);
#endif
	}

	static void* default_mark_pointer(void* ptr, const char*, unsigned int) {
		return ptr;
	}

	void* (*freelan_malloc_func)(size_t) = &default_malloc;
	void* (*freelan_realloc_func)(void*, size_t) = &default_realloc;
	void (*freelan_free_func)(void*) = &default_free;
	char* (*freelan_strdup_func)(const char*) = &default_strdup;
	void* (*freelan_mark_pointer_func)(void*, const char*, unsigned int) = &default_mark_pointer;
}

void* internal_malloc(size_t size) {
	return freelan_malloc_func(size);
}

void* internal_realloc(void* ptr, size_t size) {
	return freelan_realloc_func(ptr, size);
}

void internal_free(void* ptr) {
	return freelan_free_func(ptr);
}

char* internal_strdup(const char* str) {
	assert(str);

	const size_t len = ::strlen(str);
	char* const newstr = static_cast<char*>(internal_malloc(len + 1));
	::memcpy(newstr, str, len + 1);

	return newstr;
}

void internal_register_memory_functions(
	void* (*malloc_func)(size_t),
	void* (*realloc_func)(void*, size_t),
	void (*free_func)(void*),
	char* (*strdup_func)(const char*)
) {
	freelan_malloc_func = malloc_func ? malloc_func : &default_malloc;
	freelan_realloc_func = realloc_func ? realloc_func : &default_realloc;
	freelan_free_func = free_func ? free_func : &default_free;
	freelan_strdup_func = strdup_func ? strdup_func : &default_strdup;
}

void* internal_mark_pointer(void* ptr, const char* file, unsigned int line) {
	return freelan_mark_pointer_func(ptr, file, line);
}

void internal_register_memory_debug_functions(void* (*mark_pointer_func)(void*, const char*, unsigned int)) {
	freelan_mark_pointer_func = mark_pointer_func ? mark_pointer_func : &default_mark_pointer;
}

}

void* operator new(std::size_t n) {
	void* const result = freelan::internal_malloc(n);

	if (result == NULL) {
		throw std::bad_alloc();
	}

	return result;
}

void operator delete(void* ptr) DELETE_NOEXCEPT {
	freelan::internal_free(ptr);
}

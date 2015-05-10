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

#include <freelan/memory.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>

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
		return ::strdup(str);
	}

	void* (*freelan_malloc_func)(size_t) = &default_malloc;
	void* (*freelan_realloc_func)(void*, size_t) = &default_realloc;
	void (*freelan_free_func)(void*) = &default_free;
	char* (*freelan_strdup_func)(const char*) = &default_strdup;
}

FREELAN_API void* freelan_malloc(size_t size) {
	return freelan_malloc_func(size);
}

FREELAN_API void* freelan_realloc(void* ptr, size_t size) {
	return freelan_realloc_func(ptr, size);
}

FREELAN_API void freelan_free(void* ptr) {
	return freelan_free_func(ptr);
}

FREELAN_API char* freelan_strdup(const char* str) {
	assert(str);

	const size_t len = ::strlen(str);
	char* const newstr = static_cast<char*>(freelan_malloc(len + 1));
	::memcpy(newstr, str, len);

	return newstr;
}

FREELAN_API void freelan_register_memory_functions(
	void* (*malloc_func)(size_t),
	void* (*realloc_func)(void*, size_t),
	void (*free_func)(void*),
	char* (*strdup_func)(const char*)
) {
	freelan_malloc_func = malloc_func;
	freelan_realloc_func = realloc_func;
	freelan_free_func = free_func;
	freelan_strdup_func = strdup_func;
}

void* operator new(std::size_t n) throw(std::bad_alloc) {
	void* const result = freelan_malloc(n);

	if (result == nullptr) {
		throw std::bad_alloc();
	}

	return result;
}

void operator delete(void* p) throw() {
	freelan_free(p);
}

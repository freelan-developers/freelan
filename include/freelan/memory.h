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
 * \file memory.h
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Memory functions.
 */

#ifndef FREELAN_MEMORY_H
#define FREELAN_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FREELAN_API_NO_INCLUDES
#include "stddef.h"
#endif

#include "common.h"

#define FREELAN_MALLOC(size) freelan_mark_pointer(freelan_malloc(size), __FILE__, __LINE__)
#define FREELAN_REALLOC(ptr,size) freelan_mark_pointer(freelan_realloc(ptr, size), __FILE__, __LINE__)

/**
 * \brief Allocate a chunk of memory.
 * \param size The size of the memory chunk to allocate.
 * \return The memory chunk. If no memory can be allocated, a null pointer is
 * returned instead.
 */
FREELAN_API void* freelan_malloc(size_t size);

/**
 * \brief Reallocate a chunk of memory.
 * \param ptr The memory chunk to reallocate.
 * \param size The new size of the memory chunk to reallocate.
 * \return The memory chunk. If no memory can be reallocated, a null pointer is
 * returned and buf remains unchanged.
 */
FREELAN_API void* freelan_realloc(void* ptr, size_t size);

/**
 * \brief Free a chunk of memory.
 * \param ptr The memory chunk to free. If ptr is a null pointer, nothing is
 * done.
 */
FREELAN_API void freelan_free(void* ptr);

/**
 * \brief Duplicate a string.
 * \param str The NULL-terminated string to duplicate.
 * \return A duplicated string. If it the caller responsibility to free the
 * resulting string by calling \c freelan_free.
 */
FREELAN_API char* freelan_strdup(const char* str);

/**
 * \brief Override the memory functions.
 * \param malloc_func The allocation function. If NULL, the default
 * implementation is used.
 * \param realloc_func The reallocation function. If NULL, the default
 * implementation is used.
 * \param free_func The free function. If NULL, the default implementation is
 * used.
 * \param strdup_func The strdup function. If NULL, the default implementation
 * is used.
 *
 * \warning This function MUST be called once before using any other part of
 * the API and **NEVER** after that.
 */
FREELAN_API void freelan_register_memory_functions(
    void* (*malloc_func)(size_t),
    void* (*realloc_func)(void*, size_t),
    void (*free_func)(void*),
    char* (*strdup_func)(const char*)
);

/**
 * \brief Mark the origin of a memory allocation.
 * \param ptr The pointer to mark.
 * \param file The file at which the allocation took place. Can be NULL if that
 * information is not available.
 * \param line The line at which the allocation took place.
 * \return ptr, unchanged.
 */
FREELAN_API void* freelan_mark_pointer(void* ptr, const char* file, unsigned int line);

/**
 * \brief Override the memory debug functions.
 * \param malloc_callback_func The allocation callback. This function is called
 * whenever new memory is allocated. If NULL, the default implementation (which
 * does nothing) is used.
 * \param realloc_callback_func The reallocation callback. This function is
 * called whenever memory is reallocated. If NULL, the default implementation
 * (which does nothing) is used.
 * \param mark_pointer_func The marking pointer function. This function is
 * called whenever file:line information is available for recently-allocated
 * memory. If NULL, the default implementation (which does nothing) is used.
 * \param free_callback_func The free callback. This function is called
 * whenever memory is freed. If NULL, the default implementation (which does
 * nothing) is used.
 *
 * \warning These functions are observers and should not attempt to modify,
 * free or realloc any of the passed-in memory locations.
 */
FREELAN_API void freelan_register_memory_debug_functions(
    void (*malloc_callback_func)(void*, size_t),
    void (*realloc_callback_func)(void*, void*, size_t),
    void* (*mark_pointer_func)(void*, const char*, unsigned int),
    void (*free_callback_func)(void*)
);

#ifdef __cplusplus
}
#endif

#endif

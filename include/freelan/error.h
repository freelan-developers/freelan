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
 * \file error.h
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Error functions.
 */

#ifndef FREELAN_ERROR_H
#define FREELAN_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FREELAN_API_NO_INCLUDES
#include "stddef.h"
#endif

#include "common.h"

struct ErrorContext;

/**
 * \brief Acquire an error context.
 * \return The error context. The caller is responsible for calling \c
 * freelan_release_error_context when the error context is no longer
 * necesssary.
 *
 * \note It is recommended that you allocate one error context per thread.
 * Using the same error context simultaneously in multiple threads has
 * undefined behavior - don't do it.
 *
 * \warning If no error context can be allocated, a null pointer is returned
 * instead. This should only occur when running in very-low memory conditions.
 */
FREELAN_API struct ErrorContext* freelan_acquire_error_context(void);

/**
 * \brief Deallocate an error context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 */
FREELAN_API void freelan_release_error_context(struct ErrorContext* ectx);

/**
 * \brief Reset the error context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 *
 * It is recommended to reset the error context before making any use of it in a
 * function call.
 *
 * Resetting the error context invalidates all previous recovered values.
 */
FREELAN_API void freelan_error_context_reset(struct ErrorContext* ectx);

/**
 * \brief Get the error category associated to the context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 * \return The error category, as a string. If no error occured during the last
 * call, a null pointer is returned.
 *
 * The caller is *NOT* responsible for freeing the returned string. The value
 * is valid until the next use of the error context or until its destruction.
 */
FREELAN_API const char* freelan_error_context_get_error_category(const struct ErrorContext* ectx);

/**
 * \brief Get the error code associated to the context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 * \return The error code, as an integral value. If no error occured during the last
 * call, 0 is returned.
 */
FREELAN_API int freelan_error_context_get_error_code(const struct ErrorContext* ectx);

/**
 * \brief Get the error description associated to the context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 * \return The error description, as a string. If no error occured during the
 * last call, a null pointer is returned.
 *
 * The caller is *NOT* responsible for freeing the returned string. The value
 * is valid until the next use of the error context or until its destruction.
 */
FREELAN_API const char* freelan_error_context_get_error_description(const struct ErrorContext* ectx);

/**
 * \brief Get the filename in which the error occured from the specified
 * context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 * \return The error file, as a string. If no error occured during the last
 * call or if the file is not known, a null pointer is returned.
 *
 * The caller is *NOT* responsible for freeing the returned string. The value
 * is valid until the next use of the error context or until its destruction.
 */
FREELAN_API const char* freelan_error_context_get_error_file(const struct ErrorContext* ectx);

/**
 * \brief Get the line at which the error occured from the specified
 * context.
 * \param ectx The error context. As given by \c freelan_acquire_error_context.
 * \return The error line. If no error occured during the last call or if the
 * line is not known, 0 is returned.
 */
FREELAN_API unsigned int freelan_error_context_get_error_line(const struct ErrorContext* ectx);

#ifdef __cplusplus
}
#endif

#endif

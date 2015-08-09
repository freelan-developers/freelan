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
 * \file io.h
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Input/Output functions and types.
 */

#ifndef FREELAN_IO_H
#define FREELAN_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "error.h"

/* API BEGIN */

struct freelan_IOService;

/**
 * \brief The runnable callback type.
 */
typedef void (*freelan_RunnableCallback)(void* user_ctx);

/**
 * \brief Create a new I/O service instance that will handle I/O events.
 * \param ectx The error context.
 * \return The I/O service instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IOService_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IOService* freelan_IOService_new(struct freelan_ErrorContext* ectx);

/**
 * \brief Add an asynchronous task to the specified I/O service.
 * \param inst The freelan_IOService instance.
 * \param task The task to add. It MUST remain a valid pointer for as long as
 * it gets executed.
 * \param user_ctx The user context to pass to the runnable.
 *
 * \note You need to call (directly or indirectly) \c freelan_IOService_run for
 * the task to be effectively run.
 * \note This method is thread-safe and can be called in any thread for as long
 * as the underlying freelan_IOService instance exists.
 */
FREELAN_API void freelan_IOService_post(struct freelan_IOService* inst, freelan_RunnableCallback task, void* user_ctx);

/**
 * \brief Run the I/O service until all its tasks are complete.
 * \param inst The freelan_IOService instance.
 *
 * \warning This function blocks for as long as the I/O service has tasks to
 * run. Destroying the freelan_IOService instance while tasks are running has
 * an undefined behavior.
 * \note This method is thread-safe and can be called in any thread for as long
 * as the underlying freelan_IOService instance exists.
 */
FREELAN_API void freelan_IOService_run(struct freelan_IOService* inst);

/**
 * \brief Delete an freelan_IOService instance.
 * \param inst The instance.
 *
 * \warning Destroying an freelan_IOService instance that is currently running
 * (in another thread, for instance) has undefined behavior (you don't want
 * that - trust me).
 */
FREELAN_API void freelan_IOService_free(struct freelan_IOService* inst);

/* API END */

#ifdef __cplusplus
}
#endif

#endif

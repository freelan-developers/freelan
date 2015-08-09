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
 * \file log.h
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Structured logging functions.
 */

#ifndef FREELAN_LOG_H
#define FREELAN_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FREELAN_API_NO_INCLUDES
#include "stddef.h"
#include "stdint.h"
#endif

#include "common.h"

/**
 * \brief The log level type.
 */
typedef unsigned int freelan_LogLevel;

/**
 * \brief The log timestamp type.
 */
typedef double freelan_Timestamp;

/**
 * \brief The log entry payload types type.
 */
typedef unsigned int freelan_LogPayloadType;

/**
 * \brief A log entry payload value type.
 */
typedef union {
	void* as_null;
	const char* as_string;
	int64_t as_integer;
	double as_float;
	int as_boolean;
} freelan_LogPayloadValue;

/**
 * \brief A log entry payload structure.
 */
struct freelan_LogPayload {
	const char* key;
	freelan_LogPayloadType type;
	freelan_LogPayloadValue value;
};

/**
 * \brief A pending log entry.
 */
struct freelan_Logger;

/**
 * \brief The logging callback type.
 *
 * \note The entry is deleted automatically when the callback returns.
 */
typedef int (*freelan_LogFunctionCallback)(freelan_LogLevel level, freelan_Timestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct freelan_LogPayload* payload, const char* file, unsigned int line);

/**
 * The log levels.
 */
const freelan_LogLevel FREELAN_LOG_LEVEL_TRACE = 10;
const freelan_LogLevel FREELAN_LOG_LEVEL_DEBUG = 20;
const freelan_LogLevel FREELAN_LOG_LEVEL_INFORMATION = 30;
const freelan_LogLevel FREELAN_LOG_LEVEL_IMPORTANT = 40;
const freelan_LogLevel FREELAN_LOG_LEVEL_WARNING = 50;
const freelan_LogLevel FREELAN_LOG_LEVEL_ERROR = 60;
const freelan_LogLevel FREELAN_LOG_LEVEL_FATAL = 70;

/**
 * The log payload types.
 */
const freelan_LogPayloadType FREELAN_LOG_PAYLOAD_TYPE_NULL = 0;
const freelan_LogPayloadType FREELAN_LOG_PAYLOAD_TYPE_STRING = 1;
const freelan_LogPayloadType FREELAN_LOG_PAYLOAD_TYPE_INTEGER = 2;
const freelan_LogPayloadType FREELAN_LOG_PAYLOAD_TYPE_FLOAT = 3;
const freelan_LogPayloadType FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN = 4;

/**
 * \brief Set the log function callback.
 * \param cb The log function callback. If set to NULL, the log callback is
 * disabled.
 *
 * \warning The log function callback can (and will) be called simultaneously
 * from different threads in an unspecified order. It is the caller
 * responsibility to ensure that the log callback is thread-safe.
 *
 * \warning The log function callback will be called in a blocking-manner by
 * the library code meaning that calls must be as fast as possible. Performing
 * long-lived operations in the log callback will result in dramatically poor
 * performances.
 */
FREELAN_API void freelan_set_log_function(freelan_LogFunctionCallback cb);

/**
 * \brief Set the log level.
 * \param level The log level.
 *
 * \warning Pending logging operations are unaffected by a log level change.
 *
 * \note The default log level is FREELAN_LOG_LEVEL_INFORMATION.
 */
FREELAN_API void freelan_set_log_level(freelan_LogLevel level);

/**
 * \brief Get the log level.
 * \return The current log level.
 *
 * \note The default log level is FREELAN_LOG_LEVEL_INFORMATION.
 */
FREELAN_API freelan_LogLevel freelan_get_log_level(void);

/**
 * \brief Writes a log entry.
 * \param level The log level.
 * \param timestamp The time at which the log was emitted.
 * \param domain The log domain.
 * \param code The domain-specific code.
 * \param payload_size The number of element \c payload points to. If 0, \c
 * payload is ignored (and can thus be NULL).
 * \param payload A pointer to the first element of an array which is \c
 * payload_size long.
 * \param file The file in which the log was emitted. May be NULL if that
 * information is not available.
 * \param line The line at which the log was emitted. Ignored if \c file is
 * NULL.
 * \return A non-zero value if the log entry was handled.
 *
 * \note For a more user-friendly way of creating a log entry with payload
 * values, see \c freelan_log_start.
 */
FREELAN_API int freelan_log(freelan_LogLevel level, freelan_Timestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct freelan_LogPayload* payload, const char* file, unsigned int line);

/**
 * \brief Starts a log entry.
 * \param level The log level.
 * \param timestamp The time at which the log was emitted.
 * \param domain The log domain.
 * \param code The domain-specific code.
 * \param file The file in which the log was emitted. May be NULL if that
 * information is not available.
 * \param line The line at which the log was emitted. Ignored if \c file is
 * NULL.
 * \return A pending log entry instance.
 *
 * \note It is the caller responsibility to make sure \c freelan_log_complete()
 * is called on the returned value or memory leaks will occur.
 */
FREELAN_API struct freelan_Logger* freelan_log_start(freelan_LogLevel level, freelan_Timestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line);

/**
 * \brief Attach a payload to the the current log entry.
 * \param logger The log entry, as returned by \c freelan_log_start.
 * \param key The name of payload value to attach.
 * \param type The type of the payload value to attach.
 * \param value The value to attach.
 */
FREELAN_API void freelan_log_attach(struct freelan_Logger* logger, const char* key, freelan_LogPayloadType type, freelan_LogPayloadValue value);

/**
 * \brief Completes the current log entry.
 * \param logger The log entry, as returned by \c freelan_log_start. \c log is no
 * longer valid after the call.
 * \return A non-zero value if the log entry was handled.
 */
FREELAN_API int freelan_log_complete(struct freelan_Logger* logger);

#ifdef __cplusplus
}
#endif

#endif

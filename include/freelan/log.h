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

#include "stddef.h"
#include "stdint.h"

#include "common.h"

/**
 * \brief The log level type.
 */
typedef unsigned int FreeLANLogLevel;

/**
 * \brief The log timestamp type.
 */
typedef uint64_t FreeLANTimestamp;

/**
 * \brief The log entry payload types type.
 */
typedef unsigned int FreeLANLogPayloadType;

/**
 * \brief A log entry payload structure.
 */
struct FreeLANLogPayload {
	const char* key;
	FreeLANLogPayloadType type;
	union {
		const char* as_string;
		int64_t as_integer;
		double as_float;
		int as_boolean;
	} value;
};

/**
 * \brief A pending log entry.
 */
struct FreeLANLog;

/**
 * \brief The logging callback type.
 *
 * \note The entry is deleted automatically when the callback returns.
 */
typedef int (*FreeLANLoggingCallback)(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line);

/**
 * The log levels.
 */
const FreeLANLogLevel FREELAN_LOG_LEVEL_FATAL = 10;
const FreeLANLogLevel FREELAN_LOG_LEVEL_ERROR = 20;
const FreeLANLogLevel FREELAN_LOG_LEVEL_WARNING = 30;
const FreeLANLogLevel FREELAN_LOG_LEVEL_IMPORTANT = 40;
const FreeLANLogLevel FREELAN_LOG_LEVEL_INFORMATION = 50;
const FreeLANLogLevel FREELAN_LOG_LEVEL_DEBUG = 60;
const FreeLANLogLevel FREELAN_LOG_LEVEL_TRACE = 70;

/**
 * The log domains.
 */
const char* FREELAN_LOG_DOMAIN_GENERIC = "generic";

/**
 * The log payload types.
 */
const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_STRING = 1;
const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_INTEGER = 2;
const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_FLOAT = 3;
const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN = 4;

/**
 * \brief Set the logging function callback.
 * \param cb The logging function callback. If set to NULL, the logging
 * callback is disabled.
 *
 * \warning The logging callback can (and will) be called simultaneously from
 * different threads in an unspecified order. It is the caller responsibility
 * to ensure that the logging callback is thread-safe.
 *
 * \warning The logging callback will be called in a blocking-manner by the
 * library code meaning that calls must be as fast as possible. Performing
 * long-lived operations in the logging callback will result in dramatically
 * poor performances.
 */
FREELAN_API void freelan_set_logging_callback(FreeLANLoggingCallback cb);

/**
 * \brief Set the log level.
 * \param level The log level.
 *
 * \warning Pending logging operations are unaffected by a log level change.
 *
 * \note The default log level is FREELAN_LOG_LEVEL_INFORMATION.
 */
FREELAN_API void freelan_set_log_level(FreeLANLogLevel level);

/**
 * \brief Get the log level.
 * \return The current log level.
 *
 * \note The default log level is FREELAN_LOG_LEVEL_INFORMATION.
 */
FREELAN_API FreeLANLogLevel freelan_get_log_level(void);

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
FREELAN_API int freelan_log(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file = NULL, unsigned int line = 0);

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
FREELAN_API struct FreeLANLog* freelan_log_start(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file = NULL, unsigned int line = 0);

/**
 * \brief Attach a string to the the current log entry.
 * \param log The log entry, as returned by \c freelan_log_start.
 * \param key The name of payload value to attach.
 * \param value The value to attach.
 */
FREELAN_API void freelan_log_attach_string(struct FreeLANLog* log, const char* key, const char* value);

/**
 * \brief Attach a string to the the current log entry.
 * \param log The log entry, as returned by \c freelan_log_start.
 * \param key The name of payload value to attach.
 * \param value The value to attach.
 */
FREELAN_API void freelan_log_attach_integer(struct FreeLANLog* log, const char* key, uint64_t value);

/**
 * \brief Attach a string to the the current log entry.
 * \param log The log entry, as returned by \c freelan_log_start.
 * \param key The name of payload value to attach.
 * \param value The value to attach.
 */
FREELAN_API void freelan_log_attach_float(struct FreeLANLog* log, const char* key, double value);

/**
 * \brief Attach a string to the the current log entry.
 * \param log The log entry, as returned by \c freelan_log_start.
 * \param key The name of payload value to attach.
 * \param value The value to attach.
 */
FREELAN_API void freelan_log_attach_boolean(struct FreeLANLog* log, const char* key, int value);

/**
 * \brief Completes the current log entry.
 * \param log The log entry, as returned by \c freelan_log_start. \c log is no
 * longer valid after the call.
 * \return A non-zero value if the log entry was handled.
 */
FREELAN_API int freelan_log_complete(struct FreeLANLog* log);

#ifdef __cplusplus
}
#endif

#endif

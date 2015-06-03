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

#include <freelan/log.h>

#include <cassert>
#include <atomic>
#include <memory>

#include <vector>

#include "../internal/memory.hpp"
#include "../internal/log.hpp"

using freelan::LogLevel;

namespace {
	static std::atomic<FreeLANLoggingCallback> freelan_logging_func(nullptr);
	static std::atomic<FreeLANLogLevel> freelan_log_level(FREELAN_LOG_LEVEL_INFORMATION);

	struct LogEntry {
		FreeLANLogLevel level;
		FreeLANTimestamp timestamp;
		const char* domain;
		const char* code;
		const char* file;
		unsigned int line;
		std::vector<FreeLANLogPayload> payload;
	};
}

FREELAN_API void freelan_set_logging_callback(FreeLANLoggingCallback cb) {
	freelan_logging_func = cb;
}

FREELAN_API void freelan_set_log_level(FreeLANLogLevel level) {
	freelan_log_level = level;
}

FREELAN_API FreeLANLogLevel freelan_get_log_level(void) {
	return freelan_log_level;
}

FREELAN_API int freelan_log(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const FreeLANLogPayload* payload, const char* file, unsigned int line) {
	FreeLANLoggingCallback const func = freelan_logging_func;

	if (func && (level <= freelan_log_level)) {
		return func(level, timestamp, domain, code, payload_size, payload, file, line);
	}

	return 0;
}

FREELAN_API struct FreeLANLog* freelan_log_start(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line) {
	auto log = FREELAN_NEW LogEntry{ level, timestamp, domain, code, file, line };

	return reinterpret_cast<struct FreeLANLog*>(log);
}

FREELAN_API void freelan_log_attach_string(struct FreeLANLog* log, const char* key, const char* value) {
	assert(log);

	LogEntry& entry = *reinterpret_cast<LogEntry*>(log);
	FreeLANLogPayload item { key, FREELAN_LOG_PAYLOAD_TYPE_STRING, nullptr };
	item.value.as_string = value;

	entry.payload.push_back(item);
}

FREELAN_API void freelan_log_attach_integer(struct FreeLANLog* log, const char* key, uint64_t value) {
	assert(log);

	LogEntry& entry = *reinterpret_cast<LogEntry*>(log);
	FreeLANLogPayload item { key, FREELAN_LOG_PAYLOAD_TYPE_INTEGER, nullptr };
	item.value.as_integer = value;

	entry.payload.push_back(item);
}

FREELAN_API void freelan_log_attach_float(struct FreeLANLog* log, const char* key, double value) {
	assert(log);

	LogEntry& entry = *reinterpret_cast<LogEntry*>(log);
	FreeLANLogPayload item { key, FREELAN_LOG_PAYLOAD_TYPE_FLOAT, nullptr };
	item.value.as_float = value;

	entry.payload.push_back(item);
}

FREELAN_API void freelan_log_attach_boolean(struct FreeLANLog* log, const char* key, int value) {
	assert(log);

	LogEntry& entry = *reinterpret_cast<LogEntry*>(log);
	FreeLANLogPayload item { key, FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN, nullptr };
	item.value.as_boolean = value;

	entry.payload.push_back(item);
}

FREELAN_API int freelan_log_complete(struct FreeLANLog* log) {
	assert(log);

	std::unique_ptr<LogEntry> entry(reinterpret_cast<LogEntry*>(log));
	const FreeLANLogPayload* const payload = entry->payload.empty() ? nullptr : &entry->payload[0];

	return freelan_log(entry->level, entry->timestamp, entry->domain, entry->code, entry->payload.size(), payload, entry->file, entry->line);
}

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

#include "log.h"

#include <cassert>
#include <atomic>
#include <memory>

#include <vector>
#include <typeinfo>

#include "memory.hpp"
#include "log.hpp"

using boost::any_cast;

namespace {
	static std::atomic<FreeLANLogFunctionCallback> freelan_log_function(nullptr);
	static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

	static bool on_log_callback(freelan::LogLevel level, const boost::posix_time::ptime& timestamp, const std::string& domain, const std::string& code, const std::vector<freelan::LogPayload>& payload, const char* file, unsigned int line) {
		FreeLANLogFunctionCallback cb = freelan_log_function;

		if (cb) {
			const FreeLANTimestamp ts = (timestamp - epoch).total_microseconds() / 1000000.0f;
			const FreeLANLogPayload* p_payload = nullptr;
			std::vector<FreeLANLogPayload> raw_payload;

			if (payload.size()) {
				raw_payload.reserve(payload.size());

				for (const auto& p : payload) {
					FreeLANLogPayload item { p.key.c_str(), FREELAN_LOG_PAYLOAD_TYPE_NULL, { nullptr } };

					if (p.value.type() == typeid(std::string)) {
						item.type = FREELAN_LOG_PAYLOAD_TYPE_STRING;
						item.value.as_string = any_cast<const std::string&>(p.value).c_str();
					} else if (p.value.type() == typeid(int64_t)) {
						item.type = FREELAN_LOG_PAYLOAD_TYPE_INTEGER;
						item.value.as_integer = any_cast<int64_t>(p.value);
					} else if (p.value.type() == typeid(double)) {
						item.type = FREELAN_LOG_PAYLOAD_TYPE_FLOAT;
						item.value.as_float = any_cast<double>(p.value);
					} else if (p.value.type() == typeid(bool)) {
						item.type = FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN;
						item.value.as_boolean = any_cast<bool>(p.value) ? 1 : 0;
					}

					raw_payload.push_back(item);
				}

				p_payload = &raw_payload.front();
			}

			return cb(static_cast<FreeLANLogLevel>(level), ts, domain.c_str(), code.c_str(), raw_payload.size(), p_payload, file, line) != 0;
		}

		return false;
	}

freelan::Logger create_logger(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line) {
	const boost::posix_time::ptime ts = epoch + boost::posix_time::microseconds(static_cast<uint64_t>(timestamp * 1000000.0f));

	return freelan::Logger(static_cast<freelan::LogLevel>(level), domain, code, file, line, ts);
}

void attach_to_logger(freelan::Logger& logger, const FreeLANLogPayload& payload) {
	switch (payload.type) {
		case FREELAN_LOG_PAYLOAD_TYPE_NULL:
			logger.attach(payload.key);
			break;
		case FREELAN_LOG_PAYLOAD_TYPE_STRING:
			logger.attach(payload.key, payload.value.as_string);
			break;
		case FREELAN_LOG_PAYLOAD_TYPE_INTEGER:
			logger.attach(payload.key, payload.value.as_integer);
			break;
		case FREELAN_LOG_PAYLOAD_TYPE_FLOAT:
			logger.attach(payload.key, payload.value.as_float);
			break;
		case FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN:
			logger.attach(payload.key, payload.value.as_boolean != 0);
			break;
		default:
			assert(false);
			std::terminate();
	}
}

}

FREELAN_API void freelan_set_log_function(FreeLANLogFunctionCallback cb) {
	if (cb) {
		freelan_log_function = cb;
		freelan::set_log_function(&on_log_callback);
	} else {
		freelan::set_log_function(nullptr);
		freelan_log_function = cb;
	}
}

FREELAN_API void freelan_set_log_level(FreeLANLogLevel level) {
	freelan::set_log_level(static_cast<freelan::LogLevel>(level));
}

FREELAN_API FreeLANLogLevel freelan_get_log_level(void) {
	return static_cast<FreeLANLogLevel>(freelan::get_log_level());
}

FREELAN_API int freelan_log(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const FreeLANLogPayload* payload, const char* file, unsigned int line) {
	auto logger = create_logger(level, timestamp, domain, code, file, line);

	for (size_t i = 0; i < payload_size; ++i) {
		attach_to_logger(logger, payload[i]);
	}

	return logger.commit() ? 1 : 0;
}

FREELAN_API struct FreeLANLogger* freelan_log_start(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line) {
	auto logger = FREELAN_NEW freelan::Logger(create_logger(level, timestamp, domain, code, file, line));

	return reinterpret_cast<struct FreeLANLogger*>(logger);
}

FREELAN_API void freelan_log_attach(struct FreeLANLogger* _logger, const char* key, FreeLANLogPayloadType type, FreeLANLogPayloadValue value) {
	assert(_logger);

	freelan::Logger& logger = *reinterpret_cast<freelan::Logger*>(_logger);

	attach_to_logger(logger, FreeLANLogPayload { key, type, value });
}

FREELAN_API int freelan_log_complete(struct FreeLANLogger* _logger) {
	assert(_logger);

	std::unique_ptr<freelan::Logger> logger(reinterpret_cast<freelan::Logger*>(_logger));

	return logger->commit() ? 1 : 0;
}

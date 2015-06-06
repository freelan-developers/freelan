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
 * \file log.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Logging functions.
 */

#pragma once

#include <string>
#include <vector>
#include <functional>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <freelan/log.h>

#include "traits.hpp"


namespace freelan {

enum class LogLevel : ::FreeLANLogLevel {
	TRACE = FREELAN_LOG_LEVEL_TRACE,
	DEBUG = FREELAN_LOG_LEVEL_DEBUG,
	INFORMATION = FREELAN_LOG_LEVEL_INFORMATION,
	IMPORTANT = FREELAN_LOG_LEVEL_IMPORTANT,
	WARNING = FREELAN_LOG_LEVEL_WARNING,
	ERROR = FREELAN_LOG_LEVEL_ERROR,
	FATAL = FREELAN_LOG_LEVEL_FATAL
};

typedef std::function<bool (LogLevel level, const boost::posix_time::ptime& timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line)> LogFunction;

void set_log_function(LogFunction function);

inline void set_log_level(LogLevel level) {
	::freelan_set_log_level(static_cast<FreeLANLogLevel>(level));
}

inline LogLevel get_log_level() {
	return static_cast<LogLevel>(::freelan_get_log_level());
}

inline const char* to_raw_payload_key(const char* value) {
	return value;
}

inline const char* to_raw_payload_key(const std::string& value) {
	return value.c_str();
}

inline FreeLANLogPayload to_raw_payload(const char* key, const char* value) {
	return { key, FREELAN_LOG_PAYLOAD_TYPE_STRING, { value } };
}

inline FreeLANLogPayload to_raw_payload(const char* key, const std::string& value) {
	return { key, FREELAN_LOG_PAYLOAD_TYPE_STRING, { value.c_str() } };
}

inline FreeLANLogPayload to_raw_payload(const char* key, int value) {
	FreeLANLogPayload result { key, FREELAN_LOG_PAYLOAD_TYPE_INTEGER, { nullptr } };
	result.value.as_integer = value;
	return result;
}

inline FreeLANLogPayload to_raw_payload(const char* key, int64_t value) {
	FreeLANLogPayload result { key, FREELAN_LOG_PAYLOAD_TYPE_INTEGER, { nullptr } };
	result.value.as_integer = value;
	return result;
}

inline FreeLANLogPayload to_raw_payload(const char* key, double value) {
	FreeLANLogPayload result { key, FREELAN_LOG_PAYLOAD_TYPE_FLOAT, { nullptr } };
	result.value.as_float = value;
	return result;
}

inline FreeLANLogPayload to_raw_payload(const char* key, bool value) {
	FreeLANLogPayload result { key, FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN, { nullptr } };
	result.value.as_boolean = value ? 1 : 0;
	return result;
}

template <typename Type>
using reference_type_for = typename enable_if_else<
	std::is_array<Type>::value,
	const typename std::remove_all_extents<Type>::type*,
	Type
>::type;

template <typename KeyType, typename ValueType>
class Payload {
	public:
		Payload(const KeyType& _key, const ValueType& _value) :
			key(_key),
			value(_value)
		{}

		reference_type_for<KeyType> key;
		reference_type_for<ValueType> value;
};

template <typename KeyType, typename ValueType>
inline Payload<KeyType, ValueType> to_payload(const KeyType& key, const ValueType& value) {
	return { key, value };
}

template <typename KeyType, typename ValueType>
inline FreeLANLogPayload to_raw_payload(const Payload<KeyType, ValueType>& payload) {
	return to_raw_payload(to_raw_payload_key(payload.key), payload.value);
}

class Logger {
	public:
		Logger(LogLevel level, const char* domain, const char* code, const char* file = nullptr, unsigned int line = 0, const boost::posix_time::ptime timestamp = boost::posix_time::microsec_clock::universal_time()) :
			m_level(level),
			m_timestamp(timestamp),
			m_domain(domain),
			m_code(code),
			m_file(file),
			m_line(line)
		{}

		Logger(const Logger&) = delete;
		Logger(Logger&&) = default;

		template <typename KeyType, typename ValueType>
		Logger& operator<<(const Payload<KeyType, ValueType>& payload) {
			m_payload.push_back(to_raw_payload(payload));

			return *this;
		}

		bool commit() const {
			static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

			const FreeLANLogPayload* const p_payload = m_payload.empty() ? nullptr : &m_payload[0];

			return (
				::freelan_log(
					static_cast<FreeLANLogLevel>(m_level),
					(m_timestamp - epoch).total_microseconds() / 1000000.0f,
					m_domain,
					m_code,
					m_payload.size(),
					p_payload,
					m_file,
					m_line
				) != 0
			);
		}

	private:
		LogLevel m_level;
		boost::posix_time::ptime m_timestamp;
		const char* m_domain;
		const char* m_code;
		const char* m_file;
		unsigned int m_line;
		std::vector<FreeLANLogPayload> m_payload;
};

}

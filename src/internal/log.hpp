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
#include <typeinfo>

#include <boost/any.hpp>
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

class Payload {
	public:
		static Payload from_native_payload(const FreeLANLogPayload& native_payload) {
			switch (native_payload.type) {
				case FREELAN_LOG_PAYLOAD_TYPE_NULL:
					return { native_payload.key };
				case FREELAN_LOG_PAYLOAD_TYPE_STRING:
					return { native_payload.key, native_payload.value.as_string };
				case FREELAN_LOG_PAYLOAD_TYPE_INTEGER:
					return { native_payload.key, native_payload.value.as_integer };
				case FREELAN_LOG_PAYLOAD_TYPE_FLOAT:
					return { native_payload.key, native_payload.value.as_float };
				case FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN:
					return { native_payload.key, native_payload.value.as_boolean != 0 };
				default:
					assert(false);
					std::terminate();
			}
		}

		Payload(const std::string& _key) :
			key(_key),
			value()
		{}

		Payload(const std::string& _key, const std::string& _value) :
			key(_key),
			value(_value)
		{}

		Payload(const std::string& _key, const char* _value) :
			key(_key),
			value(std::string(_value))
		{}

		Payload(const std::string& _key, int _value) :
			key(_key),
			value(static_cast<int64_t>(_value))
		{}

		Payload(const std::string& _key, int64_t _value) :
			key(_key),
			value(_value)
		{}

		Payload(const std::string& _key, double _value) :
			key(_key),
			value(_value)
		{}

		Payload(const std::string& _key, bool _value) :
			key(_key),
			value(_value)
		{}

		FreeLANLogPayload to_native_payload() const {
			using boost::any_cast;

			FreeLANLogPayload result { key.c_str(), FREELAN_LOG_PAYLOAD_TYPE_NULL, { nullptr } };

			if (value.type() == typeid(std::string)) {
				result.type = FREELAN_LOG_PAYLOAD_TYPE_STRING;
				result.value.as_string = any_cast<std::string>(value).c_str();
			} else if (value.type() == typeid(int64_t)) {
				result.type = FREELAN_LOG_PAYLOAD_TYPE_INTEGER;
				result.value.as_integer = any_cast<int64_t>(value);
			} else if (value.type() == typeid(double)) {
				result.type = FREELAN_LOG_PAYLOAD_TYPE_FLOAT;
				result.value.as_float = any_cast<double>(value);
			} else if (value.type() == typeid(bool)) {
				result.type = FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN;
				result.value.as_boolean = any_cast<bool>(value) ? 1 : 0;
			}

			return result;
		}

		std::string key;
		boost::any value;
};

template <typename IteratorType>
inline std::vector<FreeLANLogPayload> to_native_payload(IteratorType begin, IteratorType end) {
	std::vector<FreeLANLogPayload> result;
	result.reserve(std::distance(begin, end));

	for (IteratorType it = begin; it != end; ++it) {
		result.push_back(it->to_native_payload());
	}

	return result;
}

class Logger {
	public:
		Logger(LogLevel level, const std::string& domain, const std::string& code, const char* file = nullptr, unsigned int line = 0, const boost::posix_time::ptime timestamp = boost::posix_time::microsec_clock::universal_time()) :
			m_ok(level <= get_log_level()),
			m_level(level),
			m_timestamp(timestamp),
			m_domain(domain),
			m_code(code),
			m_file(file),
			m_line(line)
		{}

		Logger(const Logger&) = delete;
		Logger(Logger&& other) :
			m_ok(std::move(other.m_ok)),
			m_level(std::move(other.m_level)),
			m_timestamp(std::move(other.m_timestamp)),
			m_domain(std::move(other.m_domain)),
			m_code(std::move(other.m_code)),
			m_file(std::move(other.m_file)),
			m_line(std::move(other.m_line))
		{
			other.m_ok = false;
		};

		template <typename ValueType>
		Logger& attach(const std::string& key, const ValueType& value) {
			if (m_ok) {
				m_payload.push_back(Payload(key, value));
			}

			return *this;
		}

		~Logger() {
			commit();
		}

		bool commit() {
			if (m_ok) {
				m_ok = false;

				static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
				const std::vector<FreeLANLogPayload> payload = to_native_payload(m_payload.begin(), m_payload.end());
				const FreeLANLogPayload* const p_payload = payload.empty() ? nullptr : &payload.front();

				return (
					::freelan_log(
						static_cast<FreeLANLogLevel>(m_level),
						(m_timestamp - epoch).total_microseconds() / 1000000.0f,
						m_domain.c_str(),
						m_code.c_str(),
						payload.size(),
						p_payload,
						m_file,
						m_line
					) != 0
				);
			}

			return false;
		}

	private:
		bool m_ok;
		LogLevel m_level;
		boost::posix_time::ptime m_timestamp;
		std::string m_domain;
		std::string m_code;
		const char* m_file;
		unsigned int m_line;
		std::vector<Payload> m_payload;
};

/**
 * \brief A logging macro.
 * \param level The log level.
 * \param domain The log domain.
 * \param code The log domain-specific code.
 */
#define LOG(level,domain,code) freelan::Logger(level, domain, code, __FILE__, __LINE__)
}

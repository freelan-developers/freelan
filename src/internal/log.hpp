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

#include <boost/any.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace freelan {

// Windows defines ERROR.
#ifdef ERROR
#undef ERROR
#endif

enum class LogLevel : unsigned int {
    TRACE = 10,
    DEBUG = 20,
    INFORMATION = 30,
    IMPORTANT = 40,
    WARNING = 50,
    ERROR = 60,
    FATAL = 70
};

inline std::ostream& operator<<(std::ostream& os, LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:
            return os << "trace";
        case LogLevel::DEBUG:
            return os << "debug";
        case LogLevel::INFORMATION:
            return os << "information";
        case LogLevel::IMPORTANT:
            return os << "important";
        case LogLevel::WARNING:
            return os << "warning";
        case LogLevel::ERROR:
            return os << "error";
        case LogLevel::FATAL:
            return os << "fatal";
        default:
            return os << "<unspecified log level " << static_cast<unsigned int>(level) << ">";
    }
}

class LogPayload {
    public:
        LogPayload(const std::string& _key) :
            key(_key),
            value()
        {}

        LogPayload(const std::string& _key, const std::string& _value) :
            key(_key),
            value(_value)
        {}

        LogPayload(const std::string& _key, const char* _value) :
            key(_key),
            value(std::string(_value))
        {}

        LogPayload(const std::string& _key, int _value) :
            key(_key),
            value(static_cast<int64_t>(_value))
        {}

        LogPayload(const std::string& _key, int64_t _value) :
            key(_key),
            value(_value)
        {}

        LogPayload(const std::string& _key, double _value) :
            key(_key),
            value(_value)
        {}

        LogPayload(const std::string& _key, bool _value) :
            key(_key),
            value(_value)
        {}

        template <typename Type>
        bool is() const {
            return as<Type>() != nullptr;
        }

        template <typename Type>
        const Type* as() const {
            return boost::any_cast<Type>(&value);
        }

        std::string key;
        boost::any value;

        friend std::ostream& operator<<(std::ostream& os, const LogPayload& _value) {
            os << _value.key;

            if (_value.is<std::string>()) {
                os << "=" << *_value.as<std::string>();
            } else if (_value.is<int64_t>()) {
                os << "=" << *_value.as<int64_t>();
            } else if (_value.is<double>()) {
                os << "=" << *_value.as<double>();
            } else if (_value.is<bool>()) {
                os << "=" << *_value.as<bool>();
            }

            return os;
        }
};

typedef std::function<bool (LogLevel level, const boost::posix_time::ptime& timestamp, const std::string& domain, const std::string& code, const std::vector<LogPayload>& payload, const char* file, unsigned int line)> LogFunction;

void set_log_function(LogFunction function = LogFunction());
LogFunction get_log_function();
void set_log_level(LogLevel level);
LogLevel get_log_level();

class Logger {
    public:
        Logger(LogLevel level, const std::string& domain, const std::string& code, const char* file = nullptr, unsigned int line = 0, const boost::posix_time::ptime timestamp = boost::posix_time::microsec_clock::universal_time()) :
            m_ok(level >= get_log_level()),
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

        Logger& attach(const std::string& key) {
            if (m_ok) {
                m_payload.push_back(LogPayload(key));
            }

            return *this;
        }

        template <typename ValueType>
        Logger& attach(const std::string& key, const ValueType& value) {
            if (m_ok) {
                m_payload.push_back(LogPayload(key, value));
            }

            return *this;
        }

        ~Logger() {
            commit();
        }

        bool commit();

    private:
        bool m_ok;
        LogLevel m_level;
        boost::posix_time::ptime m_timestamp;
        std::string m_domain;
        std::string m_code;
        const char* m_file;
        unsigned int m_line;
        std::vector<LogPayload> m_payload;
};

/**
 * \brief A logging macro.
 * \param level The log level.
 * \param domain The log domain.
 * \param code The log domain-specific code.
 */
#define LOG(level,domain,code) freelan::Logger(level, domain, code, __FILE__, __LINE__)

}

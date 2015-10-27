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
 * \file common.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Common classes and functions for tests.
 */

#pragma once

#include <gtest/gtest.h>

#include "../internal/log.hpp"

#include <boost/lexical_cast.hpp>

#include <functional>
#include <sstream>
#include <cstdlib>


class LoggedTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            using namespace std::placeholders;

            freelan::set_log_function(std::bind(&LoggedTest::on_log, this, _1, _2, _3, _4, _5, _6, _7));
            freelan::set_log_level(freelan::LogLevel::TRACE);
        }

        virtual void TearDown() {
            freelan::set_log_function();

            m_log_output.str(std::string());
            m_log_output.clear();
        }

        bool on_log(freelan::LogLevel level, const boost::posix_time::ptime& timestamp, const std::string& domain, const std::string& code, const std::vector<freelan::LogPayload>& payload, const char* file, unsigned int line) {
            m_log_output << '\n' << timestamp << " [" << level << "] " << domain << " (" << file << ":" << line << "): " << code;

            for (auto&& pl: payload) {
                m_log_output << ", " << pl;
            }

            return true;
        }

        std::string pop_log_output() {
            const auto result = m_log_output.str();
            m_log_output.str(std::string());
            m_log_output.clear();

            return result;
        }

        std::ostringstream m_log_output;
};

#define SCOPED_LOGS() SCOPED_TRACE(pop_log_output())

inline bool system_tests_enabled() {
    const char* value = std::getenv("FREELAN_ENABLE_SYSTEM_TESTS");

    return (value != nullptr) && (boost::lexical_cast<int>(value) > 0);
}

#define IS_SYSTEM_TEST() if (!system_tests_enabled()) { SUCCEED() << "System tests disabled"; return; }

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
 * \file error.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Error functions.
 */

#pragma once

#include <boost/system/system_error.hpp>

#define FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx) reinterpret_cast<freelan::ErrorContext*>(ectx)->reset(); try {
#define FREELAN_END_USE_ERROR_CONTEXT(ectx) } catch (const boost::system::system_error& ex) { reinterpret_cast<freelan::ErrorContext*>(ectx)->assign_from_exception(ex, __FILE__, __LINE__); }

namespace freelan {

class ErrorContext {
    public:
        ErrorContext() :
            m_error_code(),
            m_description(),
            m_file(),
            m_line()
        {}

        const boost::system::error_code& error_code() const {
            return m_error_code;
        }

        const std::string& description() const {
            return m_description;
        }

        const char* file() const {
            return m_file;
        }

        unsigned int line() const {
            return m_line;
        }

        void reset() {
            m_error_code = boost::system::error_code();
            m_description.clear();
            m_file = nullptr;
            m_line = 0;
        }

        void assign(const boost::system::error_code& ec, const char* _file = nullptr, unsigned int _line = 0) {
            m_error_code = ec;
            m_description = ec.message();
            m_file = _file;
            m_line = _line;
        }

        void assign_from_exception(const boost::system::system_error& ex, const char* _file = nullptr, unsigned int _line = 0) {
            assign(ex.code(), _file, _line);
        }

    private:
        boost::system::error_code m_error_code;
        std::string m_description;
        const char* m_file;
        unsigned int m_line;
};

}

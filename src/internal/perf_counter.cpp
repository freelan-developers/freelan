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

#include "perf_counter.hpp"

#include <boost/thread/tss.hpp>

namespace freelan {
    CallStats& CallStats::operator+=(const CallStats& other) {
        if (m_in_call_time_total.is_special()) {
            return operator=(other);
        }

        m_call_count += other.m_call_count;
        m_in_call_time_total += other.m_in_call_time_total;

        if (m_in_call_time_max < other.m_in_call_time_max) {
            m_in_call_time_max = other.m_in_call_time_max;
        }

        if (m_in_call_time_min > other.m_in_call_time_min) {
            m_in_call_time_min = other.m_in_call_time_min;
        }

        return *this;
    }

    void CallStats::increment_call_count(const boost::posix_time::time_duration& duration) {
        m_call_count++;

        if (m_in_call_time_total.is_special()) {
            m_in_call_time_total = duration;
            m_in_call_time_min = duration;
            m_in_call_time_max = duration;
        } else {
            m_in_call_time_total += duration;

            if (m_in_call_time_max < duration) {
                m_in_call_time_max = duration;
            }

            if (m_in_call_time_min.is_special() || (m_in_call_time_min > duration)) {
                m_in_call_time_min = duration;
            }
        }
    }

    boost::posix_time::time_duration CallStats::in_call_time_average() const {
        if (m_call_count > 0) {
            return m_in_call_time_total / m_call_count;
        }

        return boost::posix_time::not_a_date_time;
    }

    std::ostream& CallStats::write_to(std::ostream& os) const {
        os << "Call count: " << call_count() << std::endl;
        os << "In call total time: " << in_call_time_total() << std::endl;
        os << "In call max time: " << in_call_time_max() << std::endl;
        os << "In call min time: " << in_call_time_min() << std::endl;
        os << "In call average time: " << in_call_time_average() << std::endl;

        return os;
    }

    void ScopedMeasurement::report() {
        if (m_perf_counter != nullptr) {
            m_perf_counter->record_call(m_label, now() - m_start_time);
            m_perf_counter = nullptr;
        }
    }

    PerfCounter& PerfCounter::get_instance() {
        // This should be a thread_local but OSX's runtime does not support it
        // so this is the next best thing (tm).
        static boost::thread_specific_ptr<PerfCounter> instance;

        if (!instance.get()) {
            instance.reset(new PerfCounter());
        }

        return *instance;
    }

    std::ostream& PerfCounter::write_to(std::ostream& os) const {
        os << "# Function calls" << std::endl;

        for (auto&& item : m_call_stats) {
            os << "- " << item.first << std::endl;
            os << item.second << std::endl;
        }

        return os;
    }
}

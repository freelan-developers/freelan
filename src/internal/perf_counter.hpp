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
 * \brief Performance counter.
 */

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <mutex>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace freelan {

class CallStats {
    public:
        CallStats() :
            m_call_count(0),
            m_in_call_time_total(boost::posix_time::not_a_date_time),
            m_in_call_time_max(boost::posix_time::not_a_date_time),
            m_in_call_time_min(boost::posix_time::not_a_date_time)
        {}

        CallStats& operator+=(const CallStats& other);

        void increment_call_count(const boost::posix_time::time_duration& duration);

        unsigned int call_count() const { return m_call_count; }
        boost::posix_time::time_duration in_call_time_total() const { return m_in_call_time_total; }
        boost::posix_time::time_duration in_call_time_max() const { return m_in_call_time_max; }
        boost::posix_time::time_duration in_call_time_min() const { return m_in_call_time_min; }
        boost::posix_time::time_duration in_call_time_average() const;

        std::ostream& write_to(std::ostream& os) const;

    private:
        unsigned int m_call_count;
        boost::posix_time::time_duration m_in_call_time_total;
        boost::posix_time::time_duration m_in_call_time_max;
        boost::posix_time::time_duration m_in_call_time_min;

        friend std::ostream& operator<<(std::ostream& os, const CallStats& stats) {
            return stats.write_to(os);
        }

        friend CallStats operator+(const CallStats& lhs, const CallStats& rhs) {
            CallStats result = lhs;

            return result += rhs;
        }
};

class PerfCounter;

class ScopedMeasurement {
    public:
        static boost::posix_time::ptime now() {
            return boost::posix_time::microsec_clock::universal_time();
        }

        ScopedMeasurement(PerfCounter& perf_counter, const std::string& label) :
            m_perf_counter(&perf_counter),
            m_label(label),
            m_start_time(now())
        {}

        ScopedMeasurement(const ScopedMeasurement&) = delete;
        ScopedMeasurement(ScopedMeasurement&& other) :
            m_perf_counter(std::move(other.m_perf_counter)),
            m_label(std::move(m_label)),
            m_start_time(std::move(m_start_time))
        {}

        ~ScopedMeasurement() {
            report();
        }

        void report();

    private:
        PerfCounter* m_perf_counter;
        std::string m_label;
        boost::posix_time::ptime m_start_time;
};

class PerfCounter {
    public:
        /**
         * \brief Get the thread-local instance of PerfCounter.
         */
        static PerfCounter& get_instance();

        PerfCounter& operator+=(const PerfCounter& other) {
            for(auto&& item : other.m_call_stats) {
                m_call_stats[item.first] += item.second;
            }

            return *this;
        }

        void clear() {
            m_call_stats.clear();
        }

        ScopedMeasurement scoped_measurement(const std::string& label) {
            return ScopedMeasurement(*this, label);
        }

        void record_call(const std::string& label, const boost::posix_time::time_duration& duration) {
            m_call_stats[label].increment_call_count(duration);
        }

        const CallStats& get_call_stats(const std::string& label) const {
            const auto it = m_call_stats.find(label);

            if (it == m_call_stats.end()) {
                throw std::out_of_range(label);
            }

            return it->second;
        }

        std::ostream& write_to(std::ostream& os) const;

    private:
        std::map<std::string, CallStats> m_call_stats;

        friend std::ostream& operator<<(std::ostream& os, const PerfCounter& perf_counter) {
            return perf_counter.write_to(os);
        }

        friend PerfCounter operator+(const PerfCounter& lhs, const PerfCounter& rhs) {
            PerfCounter result = lhs;

            return result += rhs;
        }
};

class VampirePerfCounter : public PerfCounter {
    public:
        VampirePerfCounter(PerfCounter& target, std::mutex& mutex) :
            m_target(target),
            m_source(PerfCounter::get_instance()),
            m_mutex(mutex)
        {}

        ~VampirePerfCounter();

    private:
        PerfCounter& m_target;
        PerfCounter& m_source;
        std::mutex& m_mutex;
};

#define MEASURE_SCOPE(label) auto&& scoped_measurement_instance__UNUSED__ = freelan::PerfCounter::get_instance().scoped_measurement(label); static_cast<void>(scoped_measurement_instance__UNUSED__)
#define DELEGATE_TO_PERFCOUNTER(perf_counter,mutex) auto&& vampire_perf_counter__UNUSED__ = freelan::VampirePerfCounter(perf_counter, mutex); static_cast<void>(vampire_perf_counter__UNUSED__)
}

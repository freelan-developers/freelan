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

#include <gtest/gtest.h>

#include <thread>

#include "../internal/perf_counter.hpp"

using freelan::CallStats;
using freelan::PerfCounter;
using boost::posix_time::seconds;

TEST(PerfCounterTest, call_stats_increment_call_count) {
    CallStats stats;

    ASSERT_EQ(static_cast<unsigned int>(0), stats.call_count());
    ASSERT_TRUE(stats.in_call_time_total().is_special());
    ASSERT_TRUE(stats.in_call_time_min().is_special());
    ASSERT_TRUE(stats.in_call_time_max().is_special());
    ASSERT_TRUE(stats.in_call_time_average().is_special());

    stats.increment_call_count(seconds(5));

    ASSERT_EQ(static_cast<unsigned int>(1), stats.call_count());
    ASSERT_EQ(seconds(5), stats.in_call_time_total());
    ASSERT_EQ(seconds(5), stats.in_call_time_min());
    ASSERT_EQ(seconds(5), stats.in_call_time_max());
    ASSERT_EQ(seconds(5), stats.in_call_time_average());

    stats.increment_call_count(seconds(5));

    ASSERT_EQ(static_cast<unsigned int>(2), stats.call_count());
    ASSERT_EQ(seconds(10), stats.in_call_time_total());
    ASSERT_EQ(seconds(5), stats.in_call_time_min());
    ASSERT_EQ(seconds(5), stats.in_call_time_max());
    ASSERT_EQ(seconds(5), stats.in_call_time_average());

    stats.increment_call_count(seconds(11));

    ASSERT_EQ(static_cast<unsigned int>(3), stats.call_count());
    ASSERT_EQ(seconds(21), stats.in_call_time_total());
    ASSERT_EQ(seconds(5), stats.in_call_time_min());
    ASSERT_EQ(seconds(11), stats.in_call_time_max());
    ASSERT_EQ(seconds(7), stats.in_call_time_average());
}

TEST(PerfCounterTest, call_stats_addition) {
    CallStats accumulator;

    ASSERT_EQ(static_cast<unsigned int>(0), accumulator.call_count());
    ASSERT_TRUE(accumulator.in_call_time_total().is_special());
    ASSERT_TRUE(accumulator.in_call_time_min().is_special());
    ASSERT_TRUE(accumulator.in_call_time_max().is_special());
    ASSERT_TRUE(accumulator.in_call_time_average().is_special());

    CallStats stats;

    // Test addition of uninitialized CallStats.
    accumulator += stats;

    ASSERT_EQ(static_cast<unsigned int>(0), accumulator.call_count());
    ASSERT_TRUE(accumulator.in_call_time_total().is_special());
    ASSERT_TRUE(accumulator.in_call_time_min().is_special());
    ASSERT_TRUE(accumulator.in_call_time_max().is_special());
    ASSERT_TRUE(accumulator.in_call_time_average().is_special());

    stats.increment_call_count(seconds(5));

    // Test addition of an initialized CallStats on top of an uninitialized.
    accumulator += stats;

    ASSERT_EQ(static_cast<unsigned int>(1), accumulator.call_count());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_total());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_min());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_max());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_average());

    // Test addition of initialized CallStats.
    accumulator += stats;

    ASSERT_EQ(static_cast<unsigned int>(2), accumulator.call_count());
    ASSERT_EQ(seconds(10), accumulator.in_call_time_total());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_min());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_max());
    ASSERT_EQ(seconds(5), accumulator.in_call_time_average());
}

TEST(PerfCounterTest, perf_counter_record_call) {
    PerfCounter perf_counter;

    perf_counter.record_call("foo", seconds(4));
    perf_counter.record_call("foo", seconds(5));
    perf_counter.record_call("bar", seconds(1));
    perf_counter.record_call("foo", seconds(1));

    const auto& foo_stats = perf_counter.get_call_stats("foo");

    ASSERT_EQ(static_cast<unsigned int>(3), foo_stats.call_count());
    ASSERT_EQ(seconds(10), foo_stats.in_call_time_total());

    const auto& bar_stats = perf_counter.get_call_stats("bar");

    ASSERT_EQ(static_cast<unsigned int>(1), bar_stats.call_count());
    ASSERT_EQ(seconds(1), bar_stats.in_call_time_total());
}

TEST(PerfCounterTest, perf_counter_scoped_measurement) {
    const auto loop_cycles = 100;
    PerfCounter perf_counter;

    {
        auto&& outer_loop_measurement = perf_counter.scoped_measurement("outer_loop");

        for (auto i = loop_cycles; i > 0; --i) {
            auto&& inner_loop_measurement = perf_counter.scoped_measurement("inner_loop");

            static_cast<void>(inner_loop_measurement);
        }

        static_cast<void>(outer_loop_measurement);
    }

    const auto& inner_loop_stats = perf_counter.get_call_stats("inner_loop");
    const auto& outer_loop_stats = perf_counter.get_call_stats("outer_loop");

    ASSERT_EQ(static_cast<unsigned int>(1), outer_loop_stats.call_count());
    ASSERT_EQ(static_cast<unsigned int>(loop_cycles), inner_loop_stats.call_count());
    ASSERT_LE(inner_loop_stats.in_call_time_total(), outer_loop_stats.in_call_time_total());
}

TEST(PerfCounterTest, perf_counter_unknown_call) {
    PerfCounter perf_counter;

    try {
        perf_counter.get_call_stats("unexisting");
    }
    catch (std::out_of_range& ex) {
        ASSERT_EQ("unexisting", std::string(ex.what()));
    }
    catch (...) {
        FAIL() << "expected std::out_of_range()";
    }
}

TEST(PerfCounterTest, perf_counter_steal_from_local_thread) {
    const auto thread_count = 16;
    PerfCounter perf_counter;
    std::mutex perf_counter_mutex;

    auto&& my_func = [&perf_counter, &perf_counter_mutex](int x) {
        DELEGATE_TO_PERFCOUNTER(perf_counter, perf_counter_mutex);
        MEASURE_SCOPE("my_func");

        for (int i = 0; i < 1 << x; ++i) {
            x += x;
        }

        return x;
    };

    std::vector<std::thread> threads;

    for (auto i = 0; i < thread_count; ++i) {
        threads.push_back(std::thread(my_func, i));
    }

    for (auto&& thread : threads) {
        thread.join();
    }

    const auto& my_func_stats = perf_counter.get_call_stats("my_func");

    ASSERT_EQ(static_cast<unsigned int>(thread_count), my_func_stats.call_count());
}

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

#include <boost/date_time/posix_time/posix_time.hpp>

#include "../internal/types/x509_certificate.hpp"

using freelan::X509Certificate;

TEST(X509Certificate, default_instantiation) {
    const X509Certificate certificate;
}

TEST(X509Certificate, move_instantiation) {
    X509Certificate certificate;
    const auto now = boost::posix_time::second_clock::universal_time();
    certificate.set_version(2);
    certificate.set_serial_number(1234);
    certificate.set_not_after(now);
    certificate.set_not_before(now);
    const X509Certificate other = std::move(certificate);

    ASSERT_EQ(static_cast<unsigned int>(2), other.version());
    ASSERT_EQ(static_cast<unsigned int>(1234), other.serial_number());
    ASSERT_EQ(now, other.not_after());
    ASSERT_EQ(now, other.not_before());
}

TEST(X509Certificate, get_version_default) {
    const X509Certificate certificate;
    const auto value = certificate.version();

    ASSERT_EQ(static_cast<unsigned int>(0), value);
}

TEST(X509Certificate, get_version) {
    X509Certificate certificate;
    certificate.set_version(2);
    const auto value = certificate.version();

    ASSERT_EQ(static_cast<unsigned int>(2), value);
}

TEST(X509Certificate, get_serial_number_default) {
    const X509Certificate certificate;
    const auto value = certificate.serial_number();

    ASSERT_EQ(static_cast<unsigned long int>(0), value);
}

TEST(X509Certificate, get_serial_number) {
    X509Certificate certificate;
    certificate.set_serial_number(1234);
    const auto value = certificate.serial_number();

    ASSERT_EQ(static_cast<unsigned long int>(1234), value);
}

TEST(X509Certificate, get_not_after_default) {
    const X509Certificate certificate;
    const auto value = certificate.not_after();

    ASSERT_TRUE(value.is_special());
}

TEST(X509Certificate, get_not_after) {
    X509Certificate certificate;
    const auto now = boost::posix_time::second_clock::universal_time();
    certificate.set_not_after(now);
    const auto value = certificate.not_after();

    ASSERT_EQ(now, value);
}

TEST(X509Certificate, get_not_before_default) {
    const X509Certificate certificate;
    const auto value = certificate.not_before();

    ASSERT_TRUE(value.is_special());
}

TEST(X509Certificate, get_not_before) {
    X509Certificate certificate;
    const auto now = boost::posix_time::second_clock::universal_time();
    certificate.set_not_before(now);
    const auto value = certificate.not_before();

    ASSERT_EQ(now, value);
}

TEST(X509Certificate, write_as_der) {
    X509Certificate certificate;
    const auto value = certificate.write_as_der();
}

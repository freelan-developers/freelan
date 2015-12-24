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

#include "x509_certificate.hpp"

#include <cassert>
#include <memory>

#include <boost/lexical_cast.hpp>

#include <openssl/asn1.h>

namespace freelan
{
    namespace {
        static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

        typedef std::unique_ptr<ASN1_UTCTIME, decltype(&ASN1_UTCTIME_free)> ASN1_UTCTIME_PTR;

        ASN1_UTCTIME_PTR from_ptime(const boost::posix_time::ptime& value) {
            const time_t time = static_cast<time_t>((value - epoch).total_seconds());

            ASN1_UTCTIME_PTR asn1_utctime(::ASN1_UTCTIME_new(), ASN1_UTCTIME_free);
            const auto result = ::ASN1_UTCTIME_set(asn1_utctime.get(), time);

            if (!result) {
                error::check_openssl_error();
            }

            return asn1_utctime;
        }

        boost::posix_time::ptime to_ptime(ASN1_UTCTIME* ptr) {
            using boost::lexical_cast;

            assert(ptr);

            const auto size = ::ASN1_STRING_length(ptr);
            const auto data = ::ASN1_STRING_data(ptr);

            if (!size) {
                return boost::posix_time::not_a_date_time;
            }

            std::string str = std::string(reinterpret_cast<const char*>(data), size);

            // str format can either be:
            // YYMMDDhhmmssZ
            // YYMMDDhhmmss+hh'mm'
            // YYMMDDhhmmss-hh'mm'

            assert(str.size() >= 13);

            int year = lexical_cast<int>(str.substr(0, 2));

            if (year < 50) {
                year += 2000;
            } else {
                year += 1900;
            }

            const int month = lexical_cast<int>(str.substr(2, 2));
            const int day = lexical_cast<int>(str.substr(4, 2));

            boost::gregorian::date date(year, month, day);

            const int hour = lexical_cast<int>(str.substr(6, 2));
            const int minute = lexical_cast<int>(str.substr(8, 2));
            const int second = lexical_cast<int>(str.substr(10, 2));

            boost::posix_time::time_duration time_duration(hour, minute, second);

            const char separator = str[12];

            if ((separator == '+') || (separator == '-'))
            {
                //const int offset_hour = lexical_cast<int>(str.substr(13, 2));
                //const int offset_minute = lexical_cast<int>(str.substr(16, 2));
            }

            //TODO: Use the separator and offset information

            return boost::posix_time::ptime(date, time_duration);
        }
    }

    boost::posix_time::ptime X509Certificate::not_after() const {
        return to_ptime(X509_get_notAfter(m_ptr));
    }

    boost::posix_time::ptime X509Certificate::not_before() const {
        return to_ptime(X509_get_notBefore(m_ptr));
    }

    void X509Certificate::set_not_after(const boost::posix_time::ptime& date) {
        const auto asn1_utctime = from_ptime(date);
        const auto result = ::X509_set_notAfter(m_ptr, asn1_utctime.get());

        if (result == 0) {
            error::check_openssl_error();
        }
    }

    void X509Certificate::set_not_before(const boost::posix_time::ptime& date) {
        const auto asn1_utctime = from_ptime(date);
        const auto result = ::X509_set_notBefore(m_ptr, asn1_utctime.get());

        if (result == 0) {
            error::check_openssl_error();
        }
    }
}

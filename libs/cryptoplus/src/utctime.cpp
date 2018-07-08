/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file utctime.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_UTCTIME pointer class.
 */

#include "asn1/utctime.hpp"
#include "asn1/string.hpp"

#include <boost/lexical_cast.hpp>

#include <cassert>

namespace cryptoplus
{
#ifdef __clang__
	template <> asn1::utctime::deleter_type pointer_wrapper<asn1::utctime::value_type>::deleter;
#endif

	namespace asn1
	{
		namespace
		{
			const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
		}

		// We don't need this, because ASN1_UTCTIME is an ASN1_STRING.
#if 0
		template <>
		utctime::deleter_type pointer_wrapper<utctime::value_type>::deleter = ASN1_UTCTIME_free;
#endif

		utctime utctime::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return utctime(_ptr, deleter);
		}

		void utctime::set_time(const boost::posix_time::ptime& time) const
		{
			set_time(static_cast<time_t>((time - epoch).total_seconds()));
		}

		boost::posix_time::ptime utctime::to_ptime() const
		{
			using boost::lexical_cast;

			std::string str = string(ptr().get()).str();

			// str format can either be:
			// YYMMDDhhmmssZ
			// YYMMDDhhmmss+hh'mm'
			// YYMMDDhhmmss-hh'mm'

			assert(str.size() >= 13);

			int year = lexical_cast<int>(str.substr(0, 2));

			if (year < 50)
			{
				year += 2000;
			}
			else
			{
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
}


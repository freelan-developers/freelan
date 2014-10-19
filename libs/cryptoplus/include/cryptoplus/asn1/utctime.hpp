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
 * \file utctime.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_UTCTIME pointer class.
 */

#ifndef CRYPTOPLUS_ASN1_UTCTIME_HPP
#define CRYPTOPLUS_ASN1_UTCTIME_HPP

#include "../pointer_wrapper.hpp"
#include "../error/helpers.hpp"
#include "../bio/bio_ptr.hpp"

#include <openssl/crypto.h>
#include <openssl/asn1.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>

namespace cryptoplus
{
	namespace asn1
	{
		/**
		 * \brief An OpenSSL ASN1_UTCTIME pointer.
		 *
		 * The utctime class is a wrapper for an OpenSSL ASN1_UTCTIME* pointer.
		 *
		 * A utctime instance has the same semantic as a ASN1_UTCTIME* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * \warning Always check for the utctime not to be NULL before calling any of its method. Calling any method (except raw()) on a null utctime has undefined behavior.
		 */
		class utctime : public pointer_wrapper<ASN1_UTCTIME>
		{
			public:

				/**
				 * \brief Create a new utctime.
				 * \return The utctime.
				 *
				 * If allocation fails, an exception is thrown.
				 */
				static utctime create();

				/**
				 * \brief Take ownership of a specified ASN1_UTCTIME pointer.
				 * \param ptr The pointer. Cannot be NULL.
				 * \return An utctime.
				 */
				static utctime take_ownership(pointer ptr);

				/**
				 * \brief Create an utctime from a time_t.
				 * \param time The time_t value.
				 * \return The utctime.
				 */
				static utctime from_time_t(time_t time);

				/**
				 * \brief Create an utctime from a string.
				 * \param str The time string. It must respect one of these formats: YYMMDDhhmmssZ, YYMMDDhhmmss+hh'mm', YYMMDDhhmmss-hh'mm'.
				 *
				 * Z indicates that local time is GMT.
				 * + indicates that local time is later than GMT.
				 * - indicates that local time is earlier than GMT.
				 * hh' is the absolute value of the offset from GMT in hours.
				 * mm' is the absolute value of the offset from GMT in minutes.
				 *
				 * \return The utctime.
				 */
				static utctime from_string(const std::string& str);

				/**
				 * \brief Create an utctime by taking its value from a boost::posix_time::ptime.
				 * \param time The time.
				 * \return The utctime.
				 */
				static utctime from_ptime(const boost::posix_time::ptime& time);

				/**
				 * \brief Create a new empty utctime.
				 */
				utctime();

				/**
				 * \brief Create an utctime by *NOT* taking ownership of an existing ASN1_UTCTIME* pointer.
				 * \param ptr The ASN1_UTCTIME* pointer.
				 * \warning The caller is still responsible for freeing the memory.
				 */
				utctime(pointer ptr);

				/**
				 * \brief Set the time.
				 * \param time The time to set.
				 */
				void set_time(time_t time) const;

				/**
				 * \brief Set the time from a string.
				 * \param str The time string. It must respect one of these formats: YYMMDDhhmmssZ, YYMMDDhhmmss+hh'mm', YYMMDDhhmmss-hh'mm'.
				 *
				 * Z indicates that local time is GMT.
				 * + indicates that local time is later than GMT.
				 * - indicates that local time is earlier than GMT.
				 * hh' is the absolute value of the offset from GMT in hours.
				 * mm' is the absolute value of the offset from GMT in minutes.
				 */
				void set_time(const std::string& str) const;

				/**
				 * \brief Set the time from a ptime structure.
				 * \param time The time.
				 */
				void set_time(const boost::posix_time::ptime& time) const;

				/**
				 * \brief Get a ptime from the ASN1_UTCTIME.
				 * \return A ptime if check() returns true, boost::posix_time::not_a_date_time otherwise.
				 */
				boost::posix_time::ptime to_ptime() const;

				/**
				 * \brief Check if the structure is valid.
				 * \return true if the structure is valid, false otherwise.
				 */
				bool check() const;

				/**
				 * \brief Print a ASN1_UTCTIME to a BIO.
				 * \param bio The BIO.
				 */
				void print(bio::bio_ptr bio) const;

			private:

				explicit utctime(pointer _ptr, deleter_type _del);
		};

		/**
		 * \brief Compare two ASN1 utctime pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two utctime instance share the same underlying pointer.
		 */
		bool operator==(const utctime& lhs, const utctime& rhs);

		/**
		 * \brief Compare two ASN1 utctime pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two utctime instance do not share the same underlying pointer.
		 */
		bool operator!=(const utctime& lhs, const utctime& rhs);

		inline utctime utctime::create()
		{
			return take_ownership(ASN1_UTCTIME_new());
		}
		inline utctime utctime::from_time_t(time_t time)
		{
			utctime result = create();

			result.set_time(time);

			return result;
		}
		inline utctime utctime::from_string(const std::string& str)
		{
			utctime result = create();

			result.set_time(str);

			return result;
		}
		inline utctime utctime::from_ptime(const boost::posix_time::ptime& time)
		{
			utctime result = create();

			result.set_time(time);

			return result;
		}
		inline utctime::utctime()
		{
		}
		inline utctime::utctime(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
		{
		}
		inline void utctime::set_time(time_t time) const
		{
			throw_error_if_not(ASN1_UTCTIME_set(ptr().get(), time));
		}
		inline void utctime::set_time(const std::string& str) const
		{
			throw_error_if_not(ASN1_UTCTIME_set_string(ptr().get(), str.c_str()) != 0);
		}
		inline bool utctime::check() const
		{
			return (ASN1_UTCTIME_check(ptr().get()) != 0);
		}
		inline void utctime::print(bio::bio_ptr bio) const
		{
			throw_error_if_not(ASN1_UTCTIME_print(bio.raw(), ptr().get()) != 0);
		}
		inline utctime::utctime(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
		{
		}
		inline bool operator==(const utctime& lhs, const utctime& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const utctime& lhs, const utctime& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPLUS_ASN1_UTCTIME_HPP */

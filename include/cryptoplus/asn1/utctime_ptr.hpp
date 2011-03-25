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
 * \file utctime_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ASN1_UTCTIME pointer class.
 */

#ifndef CRYPTOPEN_ASN1_UTCTIME_PTR_HPP
#define CRYPTOPEN_ASN1_UTCTIME_PTR_HPP

#include "../error/cryptographic_exception.hpp"
#include "../nullable.hpp"

#include <openssl/crypto.h>
#include <openssl/asn1.h>

#include <vector>

namespace cryptoplus
{
	namespace asn1
	{
		/**
		 * \brief An OpenSSL ASN1_UTCTIME pointer.
		 *
		 * The utctime_ptr class is a wrapper for an OpenSSL ASN1_UTCTIME* pointer.
		 *
		 * A utctime_ptr instance has the same semantic as a ASN1_UTCTIME* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A utctime_ptr *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a utctime_ptr always points to a valid ASN1_UTCTIME structure.
		 *
		 * \warning Always check for the utctime_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset_ptr()) on a null utctime_ptr has undefined behavior.
		 */
		class utctime_ptr : public nullable<utctime_ptr>
		{
			public:

				/**
				 * \brief Create a new utctime_ptr.
				 * \param utctime The ASN1_UTCTIME to point to.
				 */
				utctime_ptr(ASN1_UTCTIME* utctime = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param utctime The ASN1_UTCTIME to point to.
				 */
				void reset_ptr(ASN1_UTCTIME* utctime = NULL);

				/**
				 * \brief Get the raw ASN1_utctime pointer.
				 * \return The raw ASN1_UTCTIME pointer.
				 */
				ASN1_UTCTIME* raw();

				/**
				 * \brief Get the raw ASN1_utctime pointer.
				 * \return The raw ASN1_UTCTIME pointer.
				 */
				const ASN1_UTCTIME* raw() const;

				/**
				 * \brief Set the time.
				 * \param time The time to set.
				 */
				void set_time(time_t time);

			private:

				bool boolean_test() const;

				ASN1_UTCTIME* m_utctime;
		};

		/**
		 * \brief Compare two ASN1 utctime pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two utctime_ptr instance share the same underlying pointer.
		 */
		bool operator==(const utctime_ptr& lhs, const utctime_ptr& rhs);

		/**
		 * \brief Compare two ASN1 utctime pointers.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two utctime_ptr instance do not share the same underlying pointer.
		 */
		bool operator!=(const utctime_ptr& lhs, const utctime_ptr& rhs);

		inline utctime_ptr::utctime_ptr(ASN1_UTCTIME* _utctime) : m_utctime(_utctime)
		{
		}
		inline void utctime_ptr::reset_ptr(ASN1_UTCTIME* _utctime)
		{
			m_utctime = _utctime;
		}
		inline ASN1_UTCTIME* utctime_ptr::raw()
		{
			return m_utctime;
		}
		inline const ASN1_UTCTIME* utctime_ptr::raw() const
		{
			return m_utctime;
		}
		inline void utctime_ptr::set_time(time_t time)
		{
			error::throw_error_if_not(ASN1_UTCTIME_set(m_utctime, time));
		}
		inline bool utctime_ptr::boolean_test() const
		{
			return (m_utctime != NULL);
		}
		inline bool operator==(const utctime_ptr& lhs, const utctime_ptr& rhs)
		{
			return lhs.raw() == rhs.raw();
		}
		inline bool operator!=(const utctime_ptr& lhs, const utctime_ptr& rhs)
		{
			return lhs.raw() != rhs.raw();
		}
	}
}

#endif /* CRYPTOPEN_ASN1_UTCTIME_PTR_HPP */


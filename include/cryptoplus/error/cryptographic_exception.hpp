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
 * \file cryptographic_exception.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Cryptographic exception class.
 */

#ifndef CRYPTOPLUS_ERROR_CRYPTOGRAPHIC_EXCEPTION_HPP
#define CRYPTOPLUS_ERROR_CRYPTOGRAPHIC_EXCEPTION_HPP

#include "error.hpp"

#include <stdexcept>

namespace cryptoplus
{
	namespace error
	{
		/**
		 * \brief Throw a cryptographic_exception for the first available cryptographic error in the error queue.
		 */
		void throw_error();

		/**
		 * \brief Throw a cryptographic_exception for the first available cryptographic error in the error queue if the condition succeeds.
		 * \param condition The condition.
		 */
		void throw_error_if(bool condition);

		/**
		 * \brief Throw a cryptographic_exception for the first available cryptographic error in the error queue if the condition fails.
		 * \param condition The condition.
		 */
		void throw_error_if_not(bool condition);
		
		/**
		 * \brief Throw a cryptographic_exception for the first available cryptographic error in the error queue if the specified pointer is NULL.
		 * \param ptr The pointer to test.
		 */
		void throw_error_if_not(const void* ptr);

		/**
		 * \brief A cryptographic exception class.
		 *
		 * Instances of cryptographic_exception are thrown whenever a cryptographic function fails.
		 */
		class cryptographic_exception : public std::runtime_error
		{
			public:

				/**
				 * \brief Create a cryptographic_exception from the first available cryptographic error in the error queue.
				 * \return A cryptographic_exception.
				 */
				static cryptographic_exception from_error();

				/**
				 * \brief Create a new cryptographic_exception from the specified error code.
				 * \param err The error code.
				 */
				cryptographic_exception(error_type err);

				/**
				 * \brief Get the associated error code.
				 * \return The associated error code.
				 */
				error_type err() const;

			private:

				error_type m_err;
		};

		inline void throw_error()
		{
			throw cryptographic_exception::from_error();
		}
		inline void throw_error_if(bool condition)
		{
			if (condition) throw_error();
		}
		inline void throw_error_if_not(bool condition)
		{
			if (!condition) throw_error();
		}
		inline void throw_error_if_not(const void* ptr)
		{
			if (ptr == NULL) throw_error();
		}
		inline cryptographic_exception cryptographic_exception::from_error()
		{
			return cryptographic_exception(get_error());
		}
		inline error_type cryptographic_exception::err() const
		{
			return m_err;
		}
	}
}

#endif /* CRYPTOPLUS_ERROR_CRYPTOGRAPHIC_EXCEPTION_HPP */

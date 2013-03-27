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
 * \file nullable.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A nullable interface.
 *
 * See http://www.artima.com/cppsource/safebool.html for more explanations.
 */

#ifndef CRYPTOPLUS_NULLABLE_HPP
#define CRYPTOPLUS_NULLABLE_HPP

namespace cryptoplus
{
	/**
	 * \brief The base nullable class.
	 */
	class nullable_base
	{
		protected:

			/**
			 * \brief The boolean type.
			 */
			typedef void (nullable_base::*bool_type)() const;

			/**
			 * \brief A dummy function.
			 */
			void this_type_does_not_support_comparisons() const {}

			/**
			 * \brief Default constructor.
			 */
			nullable_base() {}

			/**
			 * \brief Protected copy constructor.
			 */
			nullable_base(const nullable_base&) {}

			/**
			 * \brief Protected assignation operator.
			 */
			nullable_base& operator=(const nullable_base&)
			{
				return *this;
			}

			/**
			 * \brief Default destructor.
			 */
			~nullable_base() {}
	};

	/**
	 * \brief A templated version of the nullable class.
	 */
	template <typename T=void> class nullable : public nullable_base
	{
		public:

			/**
			 * \brief The bool_type conversion operator.
			 */
			operator bool_type() const
			{
				return (static_cast<const T*>(this))->boolean_test() ? &nullable<T>::this_type_does_not_support_comparisons : 0;
			}

			/**
			 * \brief Explicit test.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return !static_cast<const T*>(this)->boolean_test();
			}

		protected:

			~nullable() {}
	};

	/**
	 * \brief A specialized version of the template nullable class.
	 */
	template <> class nullable<void> : public nullable_base
	{
		public:

			/**
			 * \brief The bool_type conversion operator.
			 */
			operator bool_type() const
			{
				return (boolean_test() == true) ? &nullable<void>::this_type_does_not_support_comparisons : 0;
			}

			/**
			 * \brief Explicit test.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return boolean_test();
			}

		protected:

			/**
			 * \brief The boolean_test() method.
			 * \return true or false, it's up to you.
			 *
			 * Must be defined in the subclasses to determine booleaness of the derived type.
			 *
			 */
			virtual bool boolean_test() const = 0;
			virtual ~nullable() {}
	};

	/**
	 * \brief Compare two nullables, in a boolean context.
	 * \return false, always.
	 */
	template <typename T, typename U> inline bool operator==(const nullable<T>&, const nullable<U>&)
	{
		return false;
	}

	/**
	 * \brief Compare two nullables, in a boolean context.
	 * \return false, always.
	 */
	template <typename T, typename U> bool operator!=(const nullable<T>&, const nullable<U>&)
	{
		return false;
	}
}

#endif /* CRYPTOPLUS_NULLABLE_HPP */

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
 * \file pointer_wrapper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A pointer wrapper template class.
 */

#ifndef CRYPTOPEN_POINTER_WRAPPER_HPP
#define CRYPTOPEN_POINTER_WRAPPER_HPP

#include "nullable.hpp"

#include <boost/shared_ptr.hpp>

namespace cryptoplus
{
	/**
	 * \brief A pointer wrapper template class.
	 *
	 * A pointer_wrapper holds a shared reference to a pointer.
	 */
	template <typename T>
	class pointer_wrapper : public nullable<pointer_wrapper<T> >
	{
		public:

			/**
			 * \brief The value type.
			 */
			typedef T value_type;

			/**
			 * \brief The pointer type.
			 */
			typedef value_type* pointer;

			/**
			 * \brief The const pointer type.
			 */
			typedef const value_type* const_pointer;

			/**
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 * \warning The instance has ownership of the return pointer. Do not free the returned pointer.
			 */
			const_pointer raw() const;

			/**
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 * \warning The instance has ownership of the return pointer. Do not free the returned pointer.
			 */
			pointer raw();

		protected:

			static void null_deleter(pointer);

			explicit pointer_wrapper(pointer ptr);
			explicit pointer_wrapper(boost::shared_ptr<value_type> ptr);

			bool boolean_test() const;

			boost::shared_ptr<value_type>& ptr();
			const boost::shared_ptr<value_type>& ptr() const;

		private:

			boost::shared_ptr<value_type> m_pointer;
	};

	template <typename T>
	inline typename pointer_wrapper<T>::const_pointer pointer_wrapper<T>::raw() const
	{
		return m_pointer.get();
	}
	template <typename T>
	inline typename pointer_wrapper<T>::pointer pointer_wrapper<T>::raw()
	{
		return m_pointer.get();
	}
	template <typename T>
	inline void pointer_wrapper<T>::null_deleter(pointer)
	{
	}
	template <typename T>
	inline pointer_wrapper<T>::pointer_wrapper(typename pointer_wrapper<T>::pointer _ptr) : m_pointer(_ptr, null_deleter)
	{
	}
	template <typename T>
	inline pointer_wrapper<T>::pointer_wrapper(boost::shared_ptr<pointer_wrapper<T>::value_type> _ptr) : m_pointer(_ptr)
	{
	}
	template <typename T>
	inline bool pointer_wrapper<T>::boolean_test() const
	{
		return static_cast<bool>(m_pointer);
	}
	template <typename T>
	inline boost::shared_ptr<typename pointer_wrapper<T>::value_type>& pointer_wrapper<T>::ptr()
	{
		return m_pointer;
	}
	template <typename T>
	inline const boost::shared_ptr<typename pointer_wrapper<T>::value_type>& pointer_wrapper<T>::ptr() const
	{
		return m_pointer;
	}
}

#endif /* CRYPTOPEN_POINTER_WRAPPER_HPP */


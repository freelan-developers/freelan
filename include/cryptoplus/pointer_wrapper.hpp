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
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 * \warning The instance has ownership of the return pointer. Do not free the returned pointer.
			 */
			const T* raw() const;

			/**
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 * \warning The instance has ownership of the return pointer. Do not free the returned pointer.
			 */
			T* raw();

		protected:

			static void null_deleter(T*);

			bool boolean_test() const;

			explicit pointer_wrapper(boost::shared_ptr<T> pointer);

			boost::shared_ptr<T> m_pointer;
	};

	template <typename T>
	inline const T* pointer_wrapper<T>::raw() const
	{
		return m_pointer.get();
	}
	template <typename T>
	inline T* pointer_wrapper<T>::raw()
	{
		return m_pointer.get();
	}
	template <typename T>
	inline void pointer_wrapper<T>::null_deleter(T*)
	{
	}
	template <typename T>
	inline bool pointer_wrapper<T>::boolean_test() const
	{
		return static_cast<bool>(m_pointer);
	}
	template <typename T>
	inline pointer_wrapper<T>::pointer_wrapper(boost::shared_ptr<T> pointer) : m_pointer(pointer)
	{
	}
}

#endif /* CRYPTOPEN_POINTER_WRAPPER_HPP */


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
 * \file file.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A file class.
 */

#ifndef CRYPTOPLUS_FILE_HPP
#define CRYPTOPLUS_FILE_HPP

#include "os.hpp"
#include "pointer_wrapper.hpp"

#include <string>
#include <stdexcept>

namespace cryptoplus
{
	/**
	 * \brief A file class.
	 *
	 * The file class represents a FILE*.
	 * file is a low level structure.
	 *
	 * A file instance has the same semantic as a FILE* pointer, thus two copies of the same instance share the same underlying pointer.
	 */
	class file : public pointer_wrapper<FILE>
	{
		public:

			/**
			 * \brief Open a file.
			 * \param filename The filename.
			 * \param mode The mode, as specified in fopen().
			 * \return The file.
			 */
			static file open(const std::string& filename, const std::string& mode = "r");

#ifdef WINDOWS
			/**
			 * \brief Open a file, unicode aware version.
			 * \param filename The filename.
			 * \param mode The mode, as specified in fopen().
			 * \return The file.
			 */
			static file open(const std::wstring& filename, const std::wstring& mode = L"r");
#endif

			/**
			 * \brief Take ownership of a specified EVP_file pointer.
			 * \param ptr The pointer. Cannot be NULL.
			 * \return An file.
			 */
			static file take_ownership(pointer ptr);

			/**
			 * \brief Create a new empty file.
			 */
			file();

			/**
			 * \brief Create an file by *NOT* taking ownership of an existing EVP_file pointer.
			 * \param ptr The EVP_file pointer.
			 * \warning The caller is still responsible for freeing the memory.
			 */
			file(pointer ptr);

		private:

			explicit file(pointer _ptr, deleter_type _del);
	};

	/**
	 * \brief Compare two file instances.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two file instances share the same underlying pointer.
	 */
	bool operator==(const file& lhs, const file& rhs);

	/**
	 * \brief Compare two file instances.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two file instances do not share the same underlying pointer.
	 */
	bool operator!=(const file& lhs, const file& rhs);

	inline file::file()
	{
	}
	inline file::file(pointer _ptr) : pointer_wrapper<value_type>(_ptr, null_deleter)
	{
	}
	inline file::file(pointer _ptr, deleter_type _del) : pointer_wrapper<value_type>(_ptr, _del)
	{
	}
	inline bool operator==(const file& lhs, const file& rhs)
	{
		return lhs.raw() == rhs.raw();
	}
	inline bool operator!=(const file& lhs, const file& rhs)
	{
		return lhs.raw() != rhs.raw();
	}
}

#endif /* CRYPTOPLUS_FILE_HPP */


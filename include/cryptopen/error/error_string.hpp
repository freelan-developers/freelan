/*
 * libcryptopen - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptopen.
 *
 * libcryptopen is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptopen is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptopen in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file error_string.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Error string helper functions.
 */

#ifndef CRYPTOPEN_ERROR_STRING_HPP
#define CRYPTOPEN_ERROR_STRING_HPP

#include "error.hpp"

#include <string>

namespace cryptopen
{
	namespace error
	{
		/**
		 * \brief Get the error string associated with a specified error.
		 * \param err The error.
		 */
		std::string get_error_string(error_type err);

		/**
		 * \brief Get the error string associated with a specified error, with a limited count of characters.
		 * \param err The error.
		 * \param len The maximum characters count to get.
		 */
		std::string get_error_string_n(error_type err, size_t len);

		/**
		 * \brief Get the library error string associated with a specified error.
		 * \param err The error.
		 */
		std::string get_library_error_string(error_type err);

		/**
		 * \brief Get the function error string associated with a specified error.
		 * \param err The error.
		 */
		std::string get_function_error_string(error_type err);

		/**
		 * \brief Get the reason error string associated with a specified error.
		 * \param err The error.
		 */
		std::string get_reason_error_string(error_type err);

		inline std::string get_library_error_string(error_type err)
		{
			return ERR_lib_error_string(err);
		}
		inline std::string get_function_error_string(error_type err)
		{
			return ERR_func_error_string(err);
		}
		inline std::string get_reason_error_string(error_type err)
		{
			return ERR_reason_error_string(err);
		}
	}
}

#endif /* CRYPTOPEN_ERROR_STRING_HPP */

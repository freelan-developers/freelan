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
 * \file error_strings.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Error strings helper functions.
 */

#ifndef CRYPTOPLUS_ERROR_STRINGS_HPP
#define CRYPTOPLUS_ERROR_STRINGS_HPP

#include "error.hpp"
#include "../initializer.hpp"

#include <string>

namespace cryptoplus
{
	namespace error
	{
		/**
		 * \brief A error string initializer.
		 *
		 * Only one instance of this class should be created. When an instance exists, the library can provide more informative error strings.
		 */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
		inline void _null_function()
		{
		}

		typedef initializer<_null_function, _null_function> error_strings_initializer;
#else
		typedef initializer<ERR_load_crypto_strings, ERR_free_strings> error_strings_initializer;
#endif

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

		/**
		 * \brief Print the errors to a BIO.
		 * \param bp The BIO.
		 *
		 * The error queue will be empty after this call.
		 */
		void print_errors(BIO* bp);

		/**
		 * \brief Print the errors to a file.
		 * \param fp The file.
		 *
		 * The error queue will be empty after this call.
		 */
		void print_errors(FILE* fp);

		inline std::string get_library_error_string(error_type err)
		{
			return ERR_lib_error_string(err.error_code);
		}
		inline std::string get_function_error_string(error_type err)
		{
			return ERR_func_error_string(err.error_code);
		}
		inline std::string get_reason_error_string(error_type err)
		{
			return ERR_reason_error_string(err.error_code);
		}
		inline void print_errors(BIO* bp)
		{
			ERR_print_errors(bp);
		}
		inline void print_errors(FILE* fp)
		{
			ERR_print_errors_fp(fp);
		}
	}
}

#endif /* CRYPTOPLUS_ERROR_STRINGS_HPP */

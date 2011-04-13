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
 * \file error.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Error helper functions.
 */

#ifndef CRYPTOPLUS_ERROR_ERROR_HPP
#define CRYPTOPLUS_ERROR_ERROR_HPP

#include <openssl/err.h>

namespace cryptoplus
{
	namespace error
	{
		/**
		 * \brief The error type.
		 */
		typedef unsigned long error_type;

		/**
		 * \brief The error info structure.
		 */
		struct error_info
		{
			/**
			 * \brief The file.
			 */
			const char* file;

			/**
			 * \brief The line.
			 */
			int line;
		};

		/**
		 * \brief The error data structure.
		 */
		struct error_data
		{
			/**
			 * \brief Check if the data is a text string.
			 * \return true if the data is a text string.
			 */
			bool is_text_string() const;

			/**
			 * \brief Check if the data was allocated using OPENSSL_malloc().
			 * \return true if the data was allocated using OPENSSL_malloc().
			 */
			bool is_text_malloced() const;

			/**
			 * \brief The data.
			 */
			const char* data;

			/**
			 * \brief The flags.
			 */
			int flags;
		};

		/**
		 * \brief Get the earliest crypto error that occured in this thread and removes it from the error queue.
		 * \return The last error.
		 */
		error_type get_error();

		/**
		 * \brief Get the earliest crypto error that occured in this thread and leaves it in the error queue.
		 * \return The last error.
		 */
		error_type peek_error();

		/**
		 * \brief Get the last crypto error that occured in this thread and leaves it in the error queue.
		 * \return The last error.
		 */
		error_type peek_last_error();

		/**
		 * \brief Get the earliest crypto error that occured in this thread and removes it from the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type get_error_line(error_info& info);

		/**
		 * \brief Get the earliest crypto error that occured in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type peek_error_line(error_info& info);

		/**
		 * \brief Get the last crypto error that occured in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type peek_last_error_line(error_info& info);

		/**
		 * \brief Get the earliest crypto error that occured in this thread and removes it from the error queue.
		 * \param info The error information.
		 * \param data The error data.
		 * \return The last error.
		 */
		error_type get_error_line_data(error_info& info, error_data& data);

		/**
		 * \brief Get the earliest crypto error that occured in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \param data The error data.
		 * \return The last error.
		 */
		error_type peek_error_line_data(error_info& info, error_data& data);

		/**
		 * \brief Get the last crypto error that occured in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \param data The error data.
		 * \return The last error.
		 */
		error_type peek_last_error_line_data(error_info& info, error_data& data);

		/**
		 * \brief Get the library component of the specified error code.
		 * \param err The error code.
		 * \return The library component.
		 */
		int get_library_error(error_type err);

		/**
		 * \brief Get the function component of the specified error code.
		 * \param err The error code.
		 * \return The function component.
		 */
		int get_function_error(error_type err);

		/**
		 * \brief Get the reasib component of the specified error code.
		 * \param err The error code.
		 * \return The reason component.
		 */
		int get_reason_error(error_type err);

		inline bool error_data::is_text_string() const
		{
			return (flags & ERR_TXT_STRING) == ERR_TXT_STRING;
		}
		inline bool error_data::is_text_malloced() const
		{
			return (flags & ERR_TXT_MALLOCED) == ERR_TXT_MALLOCED;
		}
		inline error_type get_error()
		{
			return ERR_get_error();
		}
		inline error_type peek_error()
		{
			return ERR_peek_error();
		}
		inline error_type peek_last_error()
		{
			return ERR_peek_last_error();
		}
		inline error_type get_error_line(error_info& info)
		{
			return ERR_get_error_line(&info.file, &info.line);
		}
		inline error_type peek_error_line(error_info& info)
		{
			return ERR_peek_error_line(&info.file, &info.line);
		}
		inline error_type peek_last_error_line(error_info& info)
		{
			return ERR_peek_last_error_line(&info.file, &info.line);
		}
		inline error_type get_error_line_data(error_info& info, error_data& data)
		{
			return ERR_get_error_line_data(&info.file, &info.line, &data.data, &data.flags);
		}
		inline error_type peek_error_line_data(error_info& info, error_data& data)
		{
			return ERR_peek_error_line_data(&info.file, &info.line, &data.data, &data.flags);
		}
		inline error_type peek_last_error_line_data(error_info& info, error_data& data)
		{
			return ERR_peek_last_error_line_data(&info.file, &info.line, &data.data, &data.flags);
		}
		inline int get_library_error(error_type err)
		{
			return ERR_GET_LIB(err);
		}
		inline int get_function_error(error_type err)
		{
			return ERR_GET_FUNC(err);
		}
		inline int get_reason_error(error_type err)
		{
			return ERR_GET_REASON(err);
		}
	}
}

#endif /* CRYPTOPLUS_ERROR_ERROR_HPP */

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

#pragma once

#include <openssl/err.h>

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace cryptoplus
{
	namespace error
	{
		/**
		 * \brief The error type.
		 */
		struct error_type
		{
			unsigned long error_code;
		};

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
		 * \brief Get the earliest crypto error that occurred in this thread and removes it from the error queue.
		 * \return The last error.
		 */
		error_type get_error();

		/**
		 * \brief Get the earliest crypto error that occurred in this thread and leaves it in the error queue.
		 * \return The last error.
		 */
		error_type peek_error();

		/**
		 * \brief Get the last crypto error that occurred in this thread and leaves it in the error queue.
		 * \return The last error.
		 */
		error_type peek_last_error();

		/**
		 * \brief Get the earliest crypto error that occurred in this thread and removes it from the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type get_error_line(error_info& info);

		/**
		 * \brief Get the earliest crypto error that occurred in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type peek_error_line(error_info& info);

		/**
		 * \brief Get the last crypto error that occurred in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \return The last error.
		 */
		error_type peek_last_error_line(error_info& info);

		/**
		 * \brief Get the earliest crypto error that occurred in this thread and removes it from the error queue.
		 * \param info The error information.
		 * \param data The error data.
		 * \return The last error.
		 */
		error_type get_error_line_data(error_info& info, error_data& data);

		/**
		 * \brief Get the earliest crypto error that occurred in this thread and leaves it in the error queue.
		 * \param info The error information.
		 * \param data The error data.
		 * \return The last error.
		 */
		error_type peek_error_line_data(error_info& info, error_data& data);

		/**
		 * \brief Get the last crypto error that occurred in this thread and leaves it in the error queue.
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
			return error_type{ ERR_get_error() };
		}
		inline error_type peek_error()
		{
			return error_type{ ERR_peek_error() };
		}
		inline error_type peek_last_error()
		{
			return error_type{ ERR_peek_last_error() };
		}
		inline error_type get_error_line(error_info& info)
		{
			return error_type{ ERR_get_error_line(&info.file, &info.line) };
		}
		inline error_type peek_error_line(error_info& info)
		{
			return error_type{ ERR_peek_error_line(&info.file, &info.line) };
		}
		inline error_type peek_last_error_line(error_info& info)
		{
			return error_type{ ERR_peek_last_error_line(&info.file, &info.line) };
		}
		inline error_type get_error_line_data(error_info& info, error_data& data)
		{
			return error_type{ ERR_get_error_line_data(&info.file, &info.line, &data.data, &data.flags) };
		}
		inline error_type peek_error_line_data(error_info& info, error_data& data)
		{
			return error_type{ ERR_peek_error_line_data(&info.file, &info.line, &data.data, &data.flags) };
		}
		inline error_type peek_last_error_line_data(error_info& info, error_data& data)
		{
			return error_type{ ERR_peek_last_error_line_data(&info.file, &info.line, &data.data, &data.flags) };
		}
		inline int get_library_error(error_type err)
		{
			return ERR_GET_LIB(err.error_code);
		}
		inline int get_function_error(error_type err)
		{
			return ERR_GET_FUNC(err.error_code);
		}
		inline int get_reason_error(error_type err)
		{
			return ERR_GET_REASON(err.error_code);
		}
	}

	/**
	 * @brief Get the default cryptoplus error category.
	 * @return The default cryptoplus error category instance.
	 *
	 * @warning The first call to this function is thread-safe only starting with C++11.
	 */
	const boost::system::error_category& cryptoplus_category();

	/**
	 * @brief Create an error_code instance for the given error.
	 * @param error The error.
	 * @return The error_code instance.
	 */
	inline boost::system::error_code make_error_code(error::error_type error)
	{
		return boost::system::error_code(static_cast<int>(error.error_code), cryptoplus_category());
	}

	/**
	 * @brief Create an error_condition instance for the given error.
	 * @param error The error.
	 * @return The error_condition instance.
	 */
	inline boost::system::error_condition make_error_condition(error::error_type error)
	{
		return boost::system::error_condition(static_cast<int>(error.error_code), cryptoplus_category());
	}

	/**
	 * @brief A cryptoplus error category.
	 */
	class cryptoplus_category_impl : public boost::system::error_category
	{
		public:
			/**
			 * @brief Get the name of the category.
			 * @return The name of the category.
			 */
			virtual const char* name() const throw();

			/**
			 * @brief Get the error message for a given error.
			 * @param ev The error numeric value.
			 * @return The message associated to the error.
			 */
			virtual std::string message(int ev) const;
	};
}

namespace boost
{
	namespace system
	{
		template <>
		struct is_error_code_enum<cryptoplus::error::error_type> : public boost::true_type {};
	}
}

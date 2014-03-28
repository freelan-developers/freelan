/*
 * libiconvplus - A C++ lightweight wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libiconvplus.
 *
 * libiconvplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libiconvplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libiconvplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file iconv_error_category.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv error category.
 */

#ifndef ICONVPLUS_ICONV_ERROR_CATEGORY_HPP
#define ICONVPLUS_ICONV_ERROR_CATEGORY_HPP

#include <boost/system/error_code.hpp>

namespace iconvplus
{
	/**
	 * \brief The iconv error category.
	 */
	class iconv_error_category : public boost::system::error_category
	{
		public:
			/**
			 * \brief Constructor.
			 */
			iconv_error_category()
			{
			}

			/**
			 * \brief Get the name of the category.
			 * \return The name of the category.
			 */
			const char* name() const throw()
			{
				return "iconv";
			}

			/**
			 * \brief Get the error message for the specified event.
			 * \param ev The event.
			 * \return error message.
			 */
			std::string message(int ev) const;
	};

	/**
		* \brief The iconv error category.
		*/
	const iconv_error_category& iconv_category();
}

#endif /* ICONVPLUS_ICONV_ERROR_CATEGORY_HPP */

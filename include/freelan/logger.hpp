/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file logger.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A logger class.
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace freelan
{
	/**
	 * \brief Log level type.
	 */
	enum log_level
	{
		LOG_DEBUG, /**< \brief The debug log level. */
		LOG_INFORMATION, /**< \brief The information log level. */
		LOG_WARNING, /**< \brief The warning log level. */
		LOG_ERROR, /**< \brief The error log level. */
		LOG_FATAL /**< \brief The fatal log level. */
	};

	class logger_stream;

	/**
	 * \brief A logger class.
	 */
	class logger
	{
		public:

			/**
			 * \brief The log callback function type.
			 */
			typedef boost::function<void (log_level, const std::string&)> log_callback_type;

			/**
			 * \brief Create a new logger.
			 * \param callback The callback to use for logging.
			 * \param level The desired log level.
			 */
			logger(log_callback_type callback = log_callback_type(0), log_level level = LOG_INFORMATION);

			/**
			 * \brief Get the appropriate logger stream for the specified log level.
			 * \param level The log level.
			 * \return The appropriate logger stream.
			 */
			logger_stream operator()(log_level level);

			/**
			 * \brief Get the logger's level.
			 * \return The logger's level.
			 */
			log_level level() const;

		private:

			void flush(log_level);

		private:

			std::ostream& oss();

			log_callback_type m_callback;
			log_level m_level;
			boost::shared_ptr<std::ostream> m_oss;

			friend class logger_stream;
	};

	inline log_level logger::level() const
	{
		return m_level;
	}
}

#endif /* LOGGER_HPP */


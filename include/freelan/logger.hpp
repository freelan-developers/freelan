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

namespace freelan
{
	/**
	 * \brief A logger class.
	 */
	class logger
	{
		public:

			/**
			 * \brief The output stream type.
			 */
			typedef std::ostream stream_type;

			/**
			 * \brief The manipulator type.
			 */
			typedef logger& (manipulator_type)(logger&);

			/**
			 * \brief The ostream manipulator type.
			 */
			typedef stream_type& (ostream_manipulator_type)(stream_type&);

			/**
			 * \brief Create a new logger that uses the specified output stream.
			 * \param os The output stream to use.
			 *
			 * \warning os must remain valid during the whole lifetime of the logger instance.
			 */
			logger(stream_type& os);

			/**
			 * \brief Write something to the logger.
			 * \param val The value to write.
			 */
			template <typename T>
			logger& operator<<(const T& val);

			/**
			 * \brief Execute an ostream manipulator on the logger.
			 * \param manipulator The ostream manipulator to execute.
			 */
			logger& operator<<(ostream_manipulator_type manipulator);

			/**
			 * \brief Execute an manipulator on the logger.
			 * \param manipulator The manipulator to execute.
			 */
			logger& operator<<(manipulator_type manipulator);

		private:

			std::ostream& m_os;
	};
	
	/**
	 * \brief The timestamp manipulator.
	 * \param lg The logger instance to manipulate.
	 * \return lg.
	 *
	 * Add a timestamp to the log.
	 */
	logger& timestamp(logger& lg);
	
	/**
	 * \brief The prefix manipulator.
	 * \param lg The logger instance to manipulate.
	 * \return lg.
	 *
	 * Add a typical prefix to the log.
	 */
	logger& prefix(logger& lg);
	
	/**
	 * \brief The flush manipulator.
	 * \param lg The logger instance to manipulate.
	 * \return lg.
	 */
	logger& flush(logger& lg);
	
	/**
	 * \brief The end-line manipulator.
	 * \param lg The logger instance to manipulate.
	 * \return lg.
	 */
	logger& endl(logger& lg);
	
	inline logger::logger(stream_type& os) : m_os(os)
	{
	}
	
	template <typename T>
	inline logger& logger::operator<<(const T& val)
	{
		m_os << val;

		return *this;
	}
	
	logger& logger::operator<<(ostream_manipulator_type manipulator)
	{
		m_os << manipulator;

		return *this;
	}

	logger& logger::operator<<(manipulator_type manipulator)
	{
		return manipulator(*this);
	}
	
	logger& prefix(logger& lg)
	{
		return lg << "[" << timestamp << "] ";
	}

	logger& flush(logger& lg)
	{
		return lg << std::flush;
	}

	logger& endl(logger& lg)
	{
		return lg << std::endl;
	}
}

#endif /* LOGGER_HPP */


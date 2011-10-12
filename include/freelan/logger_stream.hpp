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
 * \file logger_stream.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A logger stream class.
 */

#ifndef LOGGER_STREAM_HPP
#define LOGGER_STREAM_HPP

#include <iostream>

#include <boost/optional.hpp>

namespace freelan
{
	/**
	 * \brief A logger stream class.
	 */
	class logger_stream
	{
		public:

			/**
			 * \brief The output stream type.
			 */
			typedef std::ostream stream_type;

			/**
			 * \brief The manipulator type.
			 */
			typedef logger_stream& (manipulator_type)(logger_stream&);

			/**
			 * \brief The ostream manipulator type.
			 */
			typedef stream_type& (ostream_manipulator_type)(stream_type&);

			/**
			 * \brief Create a new logger stream that uses the specified output stream.
			 * \param os The output stream to use.
			 *
			 * \warning os must remain valid during the whole lifetime of the logger instance.
			 */
			logger_stream(boost::optional<stream_type&> os = boost::none);

			/**
			 * \brief Write something to the logger stream.
			 * \param val The value to write.
			 */
			template <typename T>
			logger_stream& operator<<(const T& val);

			/**
			 * \brief Execute an ostream manipulator on the logger stream.
			 * \param manipulator The ostream manipulator to execute.
			 */
			logger_stream& operator<<(ostream_manipulator_type manipulator);

			/**
			 * \brief Execute an manipulator on the logger stream.
			 * \param manipulator The manipulator to execute.
			 */
			logger_stream& operator<<(manipulator_type manipulator);

		private:

			boost::optional<std::ostream&> m_os;
	};

	/**
	 * \brief The timestamp manipulator.
	 * \param ls The logger_stream instance to manipulate.
	 * \return ls.
	 *
	 * Add a timestamp to the log.
	 */
	logger_stream& timestamp(logger_stream& ls);
	
	/**
	 * \brief The prefix manipulator.
	 * \param ls The logger_stream instance to manipulate.
	 * \return ls.
	 *
	 * Add a typical prefix to the log.
	 */
	logger_stream& prefix(logger_stream& ls);
	
	/**
	 * \brief The flush manipulator.
	 * \param ls The logger_stream instance to manipulate.
	 * \return ls.
	 */
	logger_stream& flush(logger_stream& ls);
	
	/**
	 * \brief The end-line manipulator.
	 * \param ls The logger_stream instance to manipulate.
	 * \return ls.
	 */
	logger_stream& endl(logger_stream& ls);
	
	inline logger_stream::logger_stream(boost::optional<stream_type&> os) : m_os(os)
	{
	}
	
	template <typename T>
	inline logger_stream& logger_stream::operator<<(const T& val)
	{
		if (m_os)
		{
			*m_os << val;
		}

		return *this;
	}
	
	inline logger_stream& logger_stream::operator<<(ostream_manipulator_type manipulator)
	{
		if (m_os)
		{
			*m_os << manipulator;
		}

		return *this;
	}

	inline logger_stream& logger_stream::operator<<(manipulator_type manipulator)
	{
		return manipulator(*this);
	}
	
	inline logger_stream& prefix(logger_stream& ls)
	{
		return ls << "[" << timestamp << "] ";
	}

	inline logger_stream& flush(logger_stream& ls)
	{
		return ls << std::flush;
	}

	inline logger_stream& endl(logger_stream& ls)
	{
		return ls << std::endl;
	}
}

#endif /* LOGGER_HPP */


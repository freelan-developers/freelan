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
 * \file logger.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A logger class.
 */

#include "logger.hpp"

#include <sstream>

#include "logger_stream.hpp"

namespace freelan
{
	namespace
	{
		struct logger_pimpl
		{
			logger_pimpl(logger& logger) :
				null_stream(),
				log_stream(logger)
			{
			}

			logger_stream null_stream;
			logger_stream log_stream;
			std::ostringstream ostream;
		};
	}

	logger::logger(log_callback_type callback, log_level _level) :
		m_callback(callback),
		m_level(_level),
		m_pimpl(new logger_pimpl(*this))
	{
	}

	logger_stream& logger::operator()(log_level _level)
	{
		logger_stream& ls = 
			(_level >= m_level) ?
			log_stream() :
			null_stream();

		return ls;
	}
	
	logger_stream& logger::null_stream()
	{
		return boost::static_pointer_cast<logger_pimpl>(m_pimpl)->null_stream;
	}

	logger_stream& logger::log_stream()
	{
		return boost::static_pointer_cast<logger_pimpl>(m_pimpl)->log_stream;
	}

	std::ostream& logger::ostream()
	{
		return boost::static_pointer_cast<logger_pimpl>(m_pimpl)->ostream;
	}

	void logger::flush()
	{
		ostream() << std::flush;

		//TODO: Implement
	}
}

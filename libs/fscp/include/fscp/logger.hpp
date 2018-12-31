/*
 * libfscp - A C++ library to establish peer-to-peer virtual private networks.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file logger.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A logger class.
 */

#pragma once

#include <iostream>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/variant.hpp>

namespace fscp
{
	class logger;

	/**
	 * \brief Log level type.
	 */
	enum class log_level
	{
		trace, /**< \brief The trace log level. */
		debug, /**< \brief The debug log level. */
		information, /**< \brief The information log level. */
		important, /**< \brief The important log level. */
		warning, /**< \brief The warning log level. */
		error, /**< \brief The error log level. */
		fatal /**< \brief The fatal log level. */
	};

	/**
	 * \brief Output a log level as its string representation.
	 * \param os The output stream.
	 * \param lvl The log level.
	 * \return os.
	 */
	inline std::ostream& operator<<(std::ostream& os, log_level lvl)
	{
		switch (lvl)
		{
			case log_level::trace:
				return os << "trace";
			case log_level::debug:
				return os << "debug";
			case log_level::information:
				return os << "information";
			case log_level::important:
				return os << "important";
			case log_level::warning:
				return os << "warning";
			case log_level::error:
				return os << "error";
			case log_level::fatal:
				return os << "fatal";
			default:
				return os << "<unspecified log level>";
		}
	}

	/**
	 * \brief A null logger stream.
	 */
	class null_logger_stream {};

	/**
	 * \brief A string logger stream.
	 */
	class string_logger_stream
	{
		public:

			/**
			 * \brief Create a string logger stream.
			 * \param logger_ The logger to attach to.
			 * \param level_ The level of the logger stream.
			 */
			string_logger_stream(const logger& logger_, log_level level_) :
				m_logger(logger_),
				m_level(level_)
			{}

			/**
			 * \brief Destroy the string logger.
			 */
			~string_logger_stream();

			/**
			 * \brief Output a value to the stream.
			 * \return *this.
			 */
			template <typename Type>
			string_logger_stream& operator<<(const Type& value)
			{
				if (!m_oss)
				{
					m_oss = boost::make_shared<std::ostringstream>();
				}

				(*m_oss) << value;

				return *this;
			}

		private:

			const logger& m_logger;
			log_level m_level;
			boost::shared_ptr<std::ostringstream> m_oss;
	};

	/**
	 * \brief The logger stream type implementation.
	 */
	typedef boost::variant<null_logger_stream, string_logger_stream> logger_stream_impl;

	/**
	 * \brief A visitor that outputs value to the logger stream.
	 */
	template <typename Type>
	class output_visitor : public boost::static_visitor<>
	{
		public:

			/**
			 * \brief Create an output visitor.
			 * \param value The value to output.
			 */
			output_visitor(const Type& value) :
				m_value(value)
			{
			}

			/**
			 * \brief Does nothing.
			 */
			void operator()(null_logger_stream&) const
			{
			}

			/**
			 * \brief Output the value to the internal ostringstream.
			 * \param ls The logger stream.
			 * \return The logger stream.
			 */
			void operator()(string_logger_stream& ls) const
			{
				ls << m_value;
			}

		private:

			const Type& m_value;
	};

	/**
	* \brief A visitor that applies a manipulator to the logger stream.
	*/
	template <typename StreamType>
	class manipulator_visitor : public boost::static_visitor<>
	{
	public:

		/**
		 * \brief The manipulator type.
		 */
		typedef StreamType& (*manipulator_type)(StreamType&);

		/**
		* \brief Create a manipulator visitor.
		* \param manipulator The manipulator to apply.
		*/
		manipulator_visitor(manipulator_type manipulator) :
			m_manipulator(manipulator)
		{
		}

		/**
		* \brief Does nothing.
		*/
		void operator()(null_logger_stream&) const
		{
		}

		/**
		* \brief Output the value to the internal ostringstream.
		* \param ls The logger stream.
		* \return The logger stream.
		*/
		void operator()(string_logger_stream& ls) const
		{
			ls << m_manipulator;
		}

	private:

		manipulator_type m_manipulator;
	};

	/**
	 * \brief The logger stream type.
	 */
	class logger_stream
	{
		public:

			/**
			 * \brief The ostream manipulator type.
			 */
			typedef std::ostream& (ostream_manipulator_type)(std::ostream&);

			/**
			 * \brief The manipulator type.
			 */
			typedef logger_stream& (manipulator_type)(logger_stream&);

			logger_stream() :
				m_impl()
			{}

			logger_stream(const logger_stream_impl& impl) :
				m_impl(impl)
			{}

			/**
			 * \brief Output a value to the logger stream.
			 * \param value The value.
			 * \return *this.
			 */
			template <typename Type>
			logger_stream& operator<<(const Type& value)
			{
				boost::apply_visitor(output_visitor<Type>(value), m_impl);

				return *this;
			}

			/**
			 * \brief Output a standard manipulator to the logger stream.
			 * \param manipulator The manipulator.
			 * \return *this.
			 */
			logger_stream& operator<<(ostream_manipulator_type manipulator)
			{
				boost::apply_visitor(manipulator_visitor<std::ostream>(manipulator), m_impl);

				return *this;
			}

			/**
			 * \brief Output a standard manipulator to the logger stream.
			 * \param manipulator The manipulator.
			 * \return *this.
			 */
			logger_stream& operator<<(manipulator_type manipulator)
			{
				return manipulator(*this);
			}

		private:

			logger_stream_impl m_impl;
	};

	/**
	 * \brief A logger class.
	 */
	class logger
	{
		public:

			/**
			 * \brief The stream type.
			 */
			typedef logger_stream stream_type;

			/**
			 * \brief The timestamp type.
			 */
			typedef boost::posix_time::ptime timestamp_type;

			/**
			 * \brief The log handler type.
			 */
			typedef boost::function<void (log_level, const std::string&, const timestamp_type&)> log_handler_type;

			/**
			 * \brief Create a new logger.
			 * \param handler The function to call whenever a log entry must be written.
			 * \param _level The desired log level of the logger. Any logging below that level will be silently ignored.
			 */
			logger(log_handler_type handler = log_handler_type(), log_level _level = log_level::information) :
				m_handler(handler),
				m_level(_level)
			{
			}

			/**
			 * \brief Set the logger's callback.
			 * \param _callback The callback.
			 * \warning This method is NOT thread-safe.
			 */
			void set_callback(log_handler_type _callback)
			{
				m_handler = _callback;
			}

			/**
			 * \brief Get the logger's callback.
			 * \return The logger's callback.
			 */
			log_handler_type callback() const
			{
				return m_handler;
			}

			/**
			 * \brief Set the logger's level.
			 * \param _level The log level.
			 * \warning This method is NOT thread-safe.
			 */
			void set_level(log_level _level)
			{
				m_level = _level;
			}

			/**
			 * \brief Get the logger's level.
			 * \return The logger's level.
			 */
			log_level level() const
			{
				return m_level;
			}

			/**
			 * \brief Get a logger stream.
			 * \param level_ The log level.
			 * \return The appropriate logger stream.
			 */
			stream_type operator()(log_level level_) const
			{
				if (level_ >= m_level)
				{
					return logger_stream_impl(string_logger_stream(*this, level_));
				}
				else
				{
					return logger_stream_impl(null_logger_stream());
				}
			}

			/**
			 * \brief Log the specified message.
			 * \param level_ The log level.
			 * \param msg The message to log.
			 * \param timestamp The timestamp.
			 */
			void log(log_level level_, const std::string& msg, timestamp_type timestamp = boost::posix_time::microsec_clock::local_time()) const
			{
				if (m_handler && (level_ >= m_level))
				{
					m_handler(level_, msg, timestamp);
				}
			}

		private:

			log_handler_type m_handler;
			log_level m_level;
	};

	inline string_logger_stream::~string_logger_stream()
	{
		if (m_oss)
		{
			(*m_oss) << std::flush;

			const std::string msg = (*m_oss).str();

			m_logger.log(m_level, msg);
		}
	}
}

/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
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
 * \file hello_request.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A basic hello request class.
 */

#ifndef FSCP_HELLO_REQUEST_HPP
#define FSCP_HELLO_REQUEST_HPP

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <stdint.h>

namespace fscp
{
	/**
	 * \brief A basic hello request class.
	 */
	class hello_request
	{
		public:

			/**
			 * \brief A request callback function.
			 */
			typedef boost::function<void (boost::asio::ip::udp::endpoint, boost::posix_time::time_duration)> callback_type;

			/**
			 * \brief Create a new request.
			 * \param unique_number The unique number.
			 * \param target The target host.
			 * \param callback The callback.
			 */
			hello_request(uint32_t unique_number, const boost::asio::ip::udp::endpoint& target, callback_type callback);

			/**
			 * \brief Get the unique number.
			 * \return The unique number.
			 */
			uint32_t unique_number() const;

			/**
			 * \brief Get the target.
			 * \return The target.
			 */
			const boost::asio::ip::udp::endpoint& target() const;

			/**
			 * \brief Get the callback.
			 * \return The callback.
			 */
			callback_type callback() const;

			/**
			 * \brief Get the age of the request.
			 * \return The age of the request.
			 */
			boost::posix_time::time_duration age() const;

			/**
			 * \brief Trigger the callback function.
			 *
			 * Note: this function automatically cancels any started timeout.
			 */
			void trigger();

			/**
			 * \brief Trigger the callback function but signal a timeout.
			 */
			void trigger_timeout();

			/**
			 * \brief Set and start the timeout.
			 * \param io_service The io_service to use.
			 * \param timeout The timeout value.
			 * \warning Any existing timeout for this instance will be first cancelled.
			 */
			void start_timeout(boost::asio::io_service& io_service, boost::posix_time::time_duration timeout);

			/**
			 * \brief Cancel the timeout.
			 */
			void cancel_timeout();

		private:

			uint32_t m_unique_number;
			boost::asio::ip::udp::endpoint m_target;
			callback_type m_callback;
			boost::posix_time::ptime m_birthdate; 
			boost::shared_ptr<boost::asio::deadline_timer> m_timeout_timer;
	};

	/**
	 * \brief A hello_request list type.
	 */
	typedef std::list<hello_request> hello_request_list;

	/**
	 * \brief Find a request that matches the specified attributes.
	 * \param hello_request_list The hello_request_list.
	 * \param unique_number The unique number.
	 * \param target The target host.
	 * \return An iterator to the request, if one is found.
	 */
	hello_request_list::iterator find_hello_request(hello_request_list& hello_request_list, uint32_t unique_number, const boost::asio::ip::udp::endpoint& target);
	
	inline hello_request::hello_request(uint32_t _unique_number, const boost::asio::ip::udp::endpoint& _target, callback_type _callback) :
		m_unique_number(_unique_number),
		m_target(_target),
		m_callback(_callback),
		m_birthdate(boost::posix_time::microsec_clock::universal_time())
	{
	}

	inline uint32_t hello_request::unique_number() const
	{
		return m_unique_number;
	}
	
	inline const boost::asio::ip::udp::endpoint& hello_request::target() const
	{
		return m_target;
	}
	
	inline hello_request::callback_type hello_request::callback() const
	{
		return m_callback;
	}
	
	inline boost::posix_time::time_duration hello_request::age() const
	{
		return boost::posix_time::microsec_clock::universal_time() - m_birthdate;
	}
	
	inline void hello_request::trigger()
	{
		cancel_timeout();
		m_callback(m_target, age());
	}
	
	inline void hello_request::trigger_timeout()
	{
		m_callback(m_target, boost::posix_time::not_a_date_time);
	}

	inline void hello_request::cancel_timeout()
	{
		m_timeout_timer.reset();
	}
}

#endif /* FSCP_HELLO_REQUEST_HPP */

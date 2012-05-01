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

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <list>

#include <stdint.h>

namespace fscp
{
	/**
	 * \brief A basic hello request class.
	 */
	class hello_request : public boost::noncopyable
	{
		public:

			/**
			 * \brief The underlying endpoint type.
			 */
			typedef boost::asio::ip::udp::endpoint ep_type;

			/**
			 * \brief A request callback function.
			 */
			typedef boost::function<void (const ep_type&, const boost::posix_time::time_duration&, bool)> callback_type;

			/**
			 * \brief Create a new request.
			 * \param io_service The io_service to use.
			 * \param unique_number The unique number.
			 * \param target The target host.
			 * \param callback The callback.
			 * \param timeout The timeout value.
			 */
			hello_request(boost::asio::io_service& io_service, uint32_t unique_number, const ep_type& target, callback_type callback, boost::posix_time::time_duration timeout);

			/**
			 * \brief Destroy the request.
			 *
			 * Any associated timeout is cancelled upon destruction.
			 */
			~hello_request();

			/**
			 * \brief Get the unique number.
			 * \return The unique number.
			 */
			uint32_t unique_number() const;

			/**
			 * \brief Get the target.
			 * \return The target.
			 */
			const ep_type& target() const;

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
			 * \brief Cancel the timeout.
			 * \param success The cancel reason. true for success, false otherwise. true is the default.
			 */
			void cancel_timeout(bool success = true);

			/**
			 * \brief Check if the request is expired.
			 * \return true if the request was triggered.
			 */
			bool expired() const;

		private:

			void handle_timeout(const boost::system::error_code&);
			void trigger();

			uint32_t m_unique_number;
			ep_type m_target;
			callback_type m_callback;
			boost::posix_time::ptime m_birthdate;
			boost::asio::deadline_timer m_timeout_timer;
			bool m_cancel_status;
			bool m_triggered;
	};

	/**
	 * \brief A hello_request list type.
	 */
	typedef std::list<boost::shared_ptr<hello_request> > hello_request_list;

	/**
	 * \brief Find a request that matches the specified attributes.
	 * \param hello_request_list The hello_request_list.
	 * \param unique_number The unique number.
	 * \param target The target host.
	 * \return An iterator to the request, if one is found.
	 */
	hello_request_list::iterator find_hello_request(hello_request_list& hello_request_list, uint32_t unique_number, const hello_request::ep_type& target);

	/**
	 * \brief Erase any expired request from the specified hello_request_list.
	 * \param hello_request_list The hello_request_list.
	 */
	void erase_expired_hello_requests(hello_request_list& hello_request_list);

	inline hello_request::~hello_request()
	{
		trigger();
	}

	inline uint32_t hello_request::unique_number() const
	{
		return m_unique_number;
	}

	inline const hello_request::ep_type& hello_request::target() const
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

	inline void hello_request::cancel_timeout(bool status)
	{
		m_cancel_status = status;
		m_timeout_timer.expires_from_now(boost::posix_time::seconds(0));
	}

	inline bool hello_request::expired() const
	{
		return m_triggered;
	}

	inline void hello_request::trigger()
	{
		if (!m_triggered)
		{
			m_triggered = true;
			m_callback(m_target, age(), m_cancel_status);
		}
	}
}

#endif /* FSCP_HELLO_REQUEST_HPP */

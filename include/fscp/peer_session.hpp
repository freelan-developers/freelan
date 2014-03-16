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
 * \file session.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session class.
 */

#ifndef FSCP_PEER_SESSION_HPP
#define FSCP_PEER_SESSION_HPP

#include "session.hpp"

#include <cryptoplus/random/random.hpp>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fscp
{
	/**
	 * \brief A peer session storage class.
	 */
	class peer_session
	{
		public:

			peer_session() :
				m_current_session(),
				m_next_session(),
				m_host_identifier(),
				m_remote_host_identifier(),
				m_last_sign_of_life(boost::posix_time::microsec_clock::local_time())
			{
				// Generate a random host identifier.
				cryptoplus::random::get_random_bytes(m_host_identifier.data(), m_host_identifier.size());
			}

			bool clear();

			session_number_type next_session_number() const;
			bool has_current_session() const { return !!m_current_session; }
			session& current_session() { return *m_current_session; }
			const session& current_session() const { return *m_current_session; }
			void clear_current_session() { m_current_session = boost::none; }
			bool has_next_session() const { return !!m_next_session; }
			session& next_session() { return *m_next_session; }
			const session& next_session() const { return *m_next_session; }
			void clear_next_session() { m_next_session = boost::none; }
			session& set_next_session(session&& session) { m_next_session = std::move(session); return *m_next_session; }
			void activate_next_session() { swap(m_current_session, m_next_session); clear_next_session(); }

			const host_identifier_type& host_identifier() const { return m_host_identifier; }
			bool has_remote_host_identifier() const { return !!m_remote_host_identifier; }
			const host_identifier_type& remote_host_identifier() const { return *m_remote_host_identifier; }
			bool set_first_remote_host_identifier(const host_identifier_type& _host_identifier);
			void set_remote_host_identifier(const host_identifier_type& _host_identifier) { m_remote_host_identifier = _host_identifier; }
			void clear_remote_host_identifier() { m_remote_host_identifier = boost::none; }

			/**
			 * \brief Check if the session has timed out.
			 * \param timeout The timeout value.
			 * \return true if the session has timed out, false otherwise.
			 */
			bool has_timed_out(const boost::posix_time::time_duration& timeout) const
			{
				return (boost::posix_time::microsec_clock::local_time() > m_last_sign_of_life + timeout);
			}

			/**
			 * \brief Keep the session alive.
			 */
			void keep_alive()
			{
				m_last_sign_of_life = boost::posix_time::microsec_clock::local_time();
			}

		private:

			boost::optional<session> m_current_session;
			boost::optional<session> m_next_session;
			host_identifier_type m_host_identifier;
			boost::optional<host_identifier_type> m_remote_host_identifier;
			boost::posix_time::ptime m_last_sign_of_life;
	};
}

#endif /* FSCP_PEER_SESSION_HPP */

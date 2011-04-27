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
 * \file session_pair.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session pair class.
 */

#ifndef FSCP_SESSION_PAIR_HPP
#define FSCP_SESSION_PAIR_HPP

#include "session_store.hpp"

#include <boost/shared_ptr.hpp>

namespace fscp
{
	/**
	 * \brief An session pair class.
	 */
	class session_pair
	{
		public:

			/**
			 * \brief Create an empty session_pair.
			 */
			session_pair();

			/**
			 * \brief Check if the session_pair has a local session.
			 * \return true if the session_pair has a local session.
			 */
			bool has_local_session() const;

			/**
			 * \brief Check if the session_pair has a remote session.
			 * \return true if the session_pair has a remote session.
			 */
			bool has_remote_session() const;

			/**
			 * \brief Get the local session.
			 * \return The local session.
			 * \warning If has_local_session() is false, the behavior is undefined.
			 */
			const session_store& local_session() const;

			/**
			 * \brief Get the remote session.
			 * \return The remote session.
			 * \warning If has_remote_session() is false, the behavior is undefined.
			 */
			const session_store& remote_session() const;

			/**
			 * \brief Renew the local session, if it is old or if the renewal is forced.
			 * \param force Set to true to force renewal. Default is false.
			 * \return true if the session was effectively renewed. Should be true if force is set to true.
			 */
			bool renew_local_session(bool force = false);

			/**
			 * \brief Set the remote session.
			 * \param session The remote session.
			 */
			void set_remote_session(const session_store& session);

			/**
			 * \brief Clear the remote session.
			 */
			void clear_remote_session();

		private:

			boost::shared_ptr<session_store> m_local_session;
			boost::shared_ptr<session_store> m_remote_session;
	};

	inline session_pair::session_pair()
	{
	}

	inline bool session_pair::has_local_session() const
	{
		return static_cast<bool>(m_local_session);
	}

	inline bool session_pair::has_remote_session() const
	{
		return static_cast<bool>(m_remote_session);
	}

	inline const session_store& session_pair::local_session() const
	{
		return *m_local_session;
	}

	inline const session_store& session_pair::remote_session() const
	{
		return *m_remote_session;
	}

	inline void session_pair::clear_remote_session()
	{
		m_remote_session.reset();
	}
}

#endif /* FSCP_SESSION_PAIR_HPP */

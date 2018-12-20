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
 * \file session.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session class.
 */

#ifndef FSCP_PEER_SESSION_HPP
#define FSCP_PEER_SESSION_HPP

#include "constants.hpp"

#include <cryptoplus/buffer.hpp>
#include <cryptoplus/random/random.hpp>
#include <cryptoplus/pkey/ecdhe.hpp>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

namespace fscp
{
	/**
	 * \brief A peer session storage class.
	 */
	class peer_session
	{
		public:

			struct session_parameters
			{
				session_parameters(session_number_type _session_number, cipher_suite_type _cipher_suite, elliptic_curve_type _elliptic_curve, const cryptoplus::buffer& _public_key) :
					session_number(_session_number),
					cipher_suite(_cipher_suite),
					elliptic_curve(_elliptic_curve),
					public_key(_public_key)
				{}

				session_number_type session_number;
				cipher_suite_type cipher_suite;
				elliptic_curve_type elliptic_curve;
				cryptoplus::buffer public_key;
			};

			struct next_session_type
			{
				next_session_type(session_number_type _session_number, cipher_suite_type _cipher_suite, elliptic_curve_type _elliptic_curve) :
					ecdhe_context(_elliptic_curve.to_elliptic_curve_nid()),
					parameters(_session_number, _cipher_suite, _elliptic_curve, ecdhe_context.get_public_key())
				{}

				cryptoplus::pkey::ecdhe_context ecdhe_context;
				session_parameters parameters;
			};

			struct current_session_type
			{
				explicit current_session_type(const session_parameters& _parameters) :
					parameters(_parameters),
					local_sequence_number(),
					remote_sequence_number()
				{}

				bool is_old() const;

				session_parameters parameters;
				sequence_number_type local_sequence_number;
				sequence_number_type remote_sequence_number;
				cryptoplus::buffer local_session_key;
				cryptoplus::buffer remote_session_key;
				cryptoplus::buffer local_nonce_prefix;
				cryptoplus::buffer remote_nonce_prefix;
			};

			peer_session() :
				m_local_host_identifier(),
				m_remote_host_identifier(),
				m_last_sign_of_life(boost::posix_time::microsec_clock::local_time())
			{
				// Generate a random host identifier.
				cryptoplus::random::get_random_bytes(m_local_host_identifier.data.data(), m_local_host_identifier.data.size());
			}

			/**
			 * \brief Get the local host identifier.
			 * \return Return the local host identifier.
			 */
			const host_identifier_type& local_host_identifier() const { return m_local_host_identifier; }

			/**
			 * \brief Set the remote host identifier.
			 * \param _host_identifier The host identifier to set.
			 * \return Return true if there was no host identifier or if _host_identifier matches the current one.
			 */
			bool set_first_remote_host_identifier(const host_identifier_type& _host_identifier);

			/**
			 * \brief Clear the remote host identifier.
			 */
			void clear_remote_host_identifier() { m_remote_host_identifier = boost::none; }

			/**
			 * \brief Get the remote host identifier, if any.
			 * \return The remote host identifier.
			 */
			boost::optional<host_identifier_type> remote_host_identifier() const { return m_remote_host_identifier; }

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

			/**
			 * \brief Prepare the next session.
			 * \param _session_number The next session number.
			 * \param _cipher_suite The next cipher suite.
			 * \param _elliptic_curve The next elliptic curve.
			 * \return true if a new session was created.
			 */
			bool prepare_session(session_number_type _session_number, cipher_suite_type _cipher_suite, elliptic_curve_type _elliptic_curve);

			/**
			 * \brief Complete the next session.
			 * \param remote_public_key The remote public key.
			 * \param remote_public_key_size The remote public key size.
			 * \return true if the session was completed.
			 */
			bool complete_session(const void* remote_public_key, size_t remote_public_key_size);

			/**
			 * \brief Get the next session number.
			 * \return The next session number.
			 */
			session_number_type next_session_number() const;

			/**
			 * \brief Get the next session parameters.
			 * \return The next session parameters.
			 */
			const session_parameters& next_session_parameters() const;

			/**
			 * \brief Get the current session parameters.
			 * \return The current session parameters.
			 */
			const session_parameters& current_session_parameters() const;

			/**
			 * \brief Check if an active session exists.
			 * \return true if an active session exists.
			 */
			bool has_current_session() const { return static_cast<bool>(m_current_session); }

			/**
			 * \brief Get the current session.
			 * \return The current session, if there is one. If there is no current session, the behavior is undefined.
			 */
			const current_session_type& current_session() const { return *m_current_session; }

			/**
			 * \brief Increment the local sequence number.
			 * \return Return the current sequence number and increment it afterwards.
			 */
			sequence_number_type increment_local_sequence_number() { return ++m_current_session->local_sequence_number; }

			/**
			 * \brief Set the remote sequence number.
			 * \param sequence_number The remote sequence number.
			 * \return true if the sequence number was incremented with the new value, false is the current sequence number is greater than sequence_number.
			 */
			bool set_remote_sequence_number(sequence_number_type sequence_number);

			/**
			 * \brief Clear the current session.
			 * \return True if the session was cleared. False is there was no active session.
			 */
			bool clear();

		private:

			host_identifier_type m_local_host_identifier;
			boost::optional<host_identifier_type> m_remote_host_identifier;

			boost::posix_time::ptime m_last_sign_of_life;

			boost::shared_ptr<next_session_type> m_next_session;
			boost::shared_ptr<current_session_type> m_current_session;
	};
}

#endif /* FSCP_PEER_SESSION_HPP */

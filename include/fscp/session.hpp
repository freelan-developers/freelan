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

#ifndef FSCP_SESSION_HPP
#define FSCP_SESSION_HPP

#include "constants.hpp"

#include <cryptoplus/pkey/ecdhe.hpp>
#include <cryptoplus/buffer.hpp>
#include <cryptoplus/random/random.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>

#include <boost/optional.hpp>

namespace fscp
{
	/**
	 * \brief An session class.
	 */
	class session
	{
		public:

			class parameters
			{
				public:
					explicit parameters(const cryptoplus::buffer& _public_key, const cryptoplus::buffer& _shared_secret, const cryptoplus::buffer& _nonce_prefix) :
						m_public_key(_public_key),
						m_shared_secret(_shared_secret),
						m_nonce_prefix(_nonce_prefix),
						m_sequence_number()
					{}

					const cryptoplus::buffer& public_key() const { return m_public_key; }
					const cryptoplus::buffer& shared_secret() const { return m_shared_secret; }
					const cryptoplus::buffer& nonce_prefix() const { return m_nonce_prefix; }
					sequence_number_type sequence_number() const { return m_sequence_number; }
					void set_sequence_number(sequence_number_type _sequence_number) { m_sequence_number = _sequence_number; }

				private:
					cryptoplus::buffer m_public_key;
					cryptoplus::buffer m_shared_secret;
					cryptoplus::buffer m_nonce_prefix;
					sequence_number_type m_sequence_number;
			};

			session(session_number_type _session_number, cipher_suite_type _cipher_suite) :
				m_session_number(_session_number),
				m_cipher_suite(_cipher_suite),
				m_ecdhe_context(m_cipher_suite.to_elliptic_curve_nid()),
				m_public_key(m_ecdhe_context.get_public_key()),
				m_sequence_number()
			{}

			session_number_type session_number() const { return m_session_number; }
			cipher_suite_type cipher_suite() const { return m_cipher_suite; }
			const cryptoplus::buffer& public_key() const { return m_public_key; }
			sequence_number_type sequence_number() const { return m_sequence_number; }
			sequence_number_type increment_sequence_number() { return m_sequence_number++; }
			bool has_remote_parameters() const { return !!m_remote_parameters; }
			const parameters& remote_parameters() const { return *m_remote_parameters; }
			bool has_shared_secret() const { return !!m_shared_secret; }
			const cryptoplus::buffer& shared_secret() const { return *m_shared_secret; }
			bool has_nonce_prefix() const { return !!m_nonce_prefix; }
			const cryptoplus::buffer& nonce_prefix() const { return *m_nonce_prefix; }

			void set_remote_parameters(const void* remote_public_key, size_t remote_public_key_size);
			bool match_parameters(cipher_suite_type _cipher_suite, const void* remote_public_key, size_t remote_public_key_size);
			bool is_old() const;

		private:

			session_number_type m_session_number;
			cipher_suite_type m_cipher_suite;
			cryptoplus::pkey::ecdhe_context m_ecdhe_context;
			cryptoplus::buffer m_public_key;
			sequence_number_type m_sequence_number;
			boost::optional<cryptoplus::buffer> m_secret_key;
			boost::optional<parameters> m_remote_parameters;
			boost::optional<cryptoplus::buffer> m_shared_secret;
			boost::optional<cryptoplus::buffer> m_nonce_prefix;
	};
}

#endif /* FSCP_SESSION_HPP */

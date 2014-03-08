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
#include <cryptoplus/hash/pbkdf2.hpp>

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
					parameters(const cryptoplus::buffer& public_key, const cryptoplus::buffer& salt, const cryptoplus::buffer& nonce_prefix, const cryptoplus::buffer& shared_secret) :
						m_public_key(public_key),
						m_salt(salt),
						m_nonce_prefix(nonce_prefix),
						m_shared_secret(shared_secret),
						m_sequence_number()
					{}

					const cryptoplus::buffer& public_key() const { return m_public_key; }
					const cryptoplus::buffer& salt() const { return m_salt; }
					const cryptoplus::buffer& nonce_prefix() const { return m_nonce_prefix; }
					const cryptoplus::buffer& shared_secret() const { return m_shared_secret; }
					sequence_number_type sequence_number() const { return m_sequence_number; }

				private:
					cryptoplus::buffer m_public_key;
					cryptoplus::buffer m_salt;
					cryptoplus::buffer m_nonce_prefix;
					cryptoplus::buffer m_shared_secret;
					sequence_number_type m_sequence_number;
			};

			session(session_number_type _session_number, elliptic_curve_type ec, key_derivation_algorithm_type kd, cipher_algorithm_type calg) :
				m_session_number(_session_number),
				m_elliptic_curve(ec),
				m_key_derivation_algorithm(kd),
				m_cipher_algorithm(calg),
				m_ecdhe_context(m_elliptic_curve.to_elliptic_curve_nid()),
				m_public_key(m_ecdhe_context.get_public_key()),
				m_salt(cryptoplus::random::get_random_bytes(DEFAULT_SALT_SIZE)),
				m_nonce_prefix(cryptoplus::random::get_random_bytes(DEFAULT_NONCE_PREFIX_SIZE)),
				m_sequence_number()
			{}

			session_number_type session_number() const { return m_session_number; }
			elliptic_curve_type elliptic_curve() const { return m_elliptic_curve; }
			key_derivation_algorithm_type key_derivation_algorithm() const { return m_key_derivation_algorithm; }
			cipher_algorithm_type cipher_algorithm() const { return m_cipher_algorithm; }
			const cryptoplus::buffer& public_key() { return m_public_key; }
			const cryptoplus::buffer& salt() { return m_salt; }
			const cryptoplus::buffer& nonce_prefix() const { return m_nonce_prefix; }
			sequence_number_type sequence_number() const { return m_sequence_number; }
			bool has_remote_parameters() const { return !!m_remote_parameters; }
			const parameters& remote_parameters() const { return *m_remote_parameters; }
			bool has_shared_secret() const { return !!m_shared_secret; }
			const cryptoplus::buffer& shared_secret() const { return *m_shared_secret; }

			template <typename ConstBufferType, typename ConstBufferType2, typename ConstBufferType3>
			void set_remote_parameters(const ConstBufferType& remote_public_key, const ConstBufferType2& salt, const ConstBufferType3& remote_nonce_prefix)
			{
				assert(!m_remote_parameters);

				if (m_key_derivation_algorithm != key_derivation_algorithm_type::pbkdf2_sha256)
				{
					throw std::runtime_error("Unsupported key derivation algorithm");
				}

				const auto dh_shared_secret = m_ecdhe_context.derive_secret_key(buffer_cast<const void*>(remote_public_key), buffer_cast(remote_public_key));

				m_shared_secret = cryptoplus::hash::pbkdf2(
					buffer_cast<const void*>(dh_shared_secret),
					buffer_size(dh_shared_secret),
					buffer_cast<const void*>(m_salt),
					buffer_size(m_salt),
					m_key_derivation_algorithm.to_message_digest_algorithm(),
					m_key_derivation_algorithm.to_iterations_count()
				);

				const auto shared_secret = cryptoplus::hash::pbkdf2(
					buffer_cast<const void*>(dh_shared_secret),
					buffer_size(dh_shared_secret),
					buffer_cast<const void*>(salt),
					buffer_size(salt),
					m_key_derivation_algorithm.to_message_digest_algorithm(),
					m_key_derivation_algorithm.to_iterations_count()
				);

				m_remote_parameters = parameters(
					cryptoplus::buffer(buffer_cast<const void*>(remote_public_key), buffer_size(remote_public_key)),
					cryptoplus::buffer(buffer_cast<const void*>(salt), buffer_size(salt)),
					cryptoplus::buffer(buffer_cast<const void*>(remote_nonce_prefix), buffer_size(remote_nonce_prefix)),
					shared_secret
				);
			}

		private:

			session_number_type m_session_number;
			elliptic_curve_type m_elliptic_curve;
			key_derivation_algorithm_type m_key_derivation_algorithm;
			cipher_algorithm_type m_cipher_algorithm;
			cryptoplus::pkey::ecdhe_context m_ecdhe_context;
			cryptoplus::buffer m_public_key;
			cryptoplus::buffer m_salt;
			cryptoplus::buffer m_nonce_prefix;
			sequence_number_type m_sequence_number;
			boost::optional<parameters> m_remote_parameters;
			boost::optional<cryptoplus::buffer> m_shared_secret;
	};
}

#endif /* FSCP_SESSION_HPP */

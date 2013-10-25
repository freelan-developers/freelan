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
 * \file session_store.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session store class.
 */

#ifndef FSCP_SESSION_STORE_HPP
#define FSCP_SESSION_STORE_HPP

#include "constants.hpp"

#include <vector>

#include <boost/optional.hpp>

#include <stdint.h>

namespace fscp
{
	/**
	 * \brief An session store class.
	 */
	class session_store
	{
		public:

			/**
			 * \brief The session number type.
			 */
			typedef uint32_t session_number_type;

			/**
			 * \brief Create a new random session store.
			 * \param session_number The session number.
			 * \param cipher_algorithm The cipher algorithm.
			 */
			session_store(session_number_type session_number, const cipher_algorithm_type& cipher_algorithm);

			/**
			 * \brief Create a new session store.
			 * \param session_number The session number.
			 * \param cipher_algorithm The cipher algorithm.
			 * \param enc_key The encryption key.
			 * \param enc_key_len The encryption key length.
			 * \param nonce_prefix The nonce prefix.
			 * \param nonce_prefix_len The nonce prefix length.
			 */
			session_store(session_number_type session_number, const cipher_algorithm_type& cipher_algorithm, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len);

			/**
			 * \brief Get the session number.
			 * \return The session number.
			 */
			session_number_type session_number() const;

			/**
			 * \brief Get the cipher algorithm.
			 * \return The cipher algorithm.
			 */
			const cipher_algorithm_type& cipher_algorithm() const;

			/**
			 * \brief Get the encryption key.
			 * \return The encryption key.
			 */
			const uint8_t* encryption_key() const;

			/**
			 * \brief Get the encryption key size.
			 * \return The encryption key size.
			 */
			size_t encryption_key_size() const;

			/**
			 * \brief Get the nonce prefix.
			 * \return The nonce prefix.
			 */
			const uint8_t* nonce_prefix() const;

			/**
			 * \brief Get the nonce prefix size.
			 * \return The nonce prefix size.
			 */
			size_t nonce_prefix_size() const;

			/**
			 * \brief Get the sequence number.
			 * \return The sequence number.
			 */
			sequence_number_type sequence_number() const;

			/**
			 * \brief Set the sequence number.
			 * \param sequence_number The new sequence number.
			 */
			void set_sequence_number(sequence_number_type sequence_number);

			/**
			 * \brief Increment the sequence number by a certain amount.
			 * \param cnt The number to add to the current sequence number. Default is 1.
			 */
			void increment_sequence_number(size_t cnt = 1);

			/**
			 * \brief Check if the session is old.
			 * \return true if the function is old.
			 */
			bool is_old() const;

		private:

			/**
			 * \brief The key type.
			 */
			typedef std::vector<uint8_t> key_type;

			/**
			 * \brief The nonce prefix type.
			 */
			typedef std::vector<uint8_t> nonce_prefix_type;

			session_number_type m_session_number;
			cipher_algorithm_type m_cipher_algorithm;
			key_type m_enc_key;
			nonce_prefix_type m_nonce_prefix;
			sequence_number_type m_sequence_number;
	};

	inline session_store::session_number_type session_store::session_number() const
	{
		return m_session_number;
	}

	inline const cipher_algorithm_type& session_store::cipher_algorithm() const
	{
		return m_cipher_algorithm;
	}

	inline const uint8_t* session_store::encryption_key() const
	{
		return &m_enc_key[0];
	}

	inline size_t session_store::encryption_key_size() const
	{
		return m_enc_key.size();
	}

	inline const uint8_t* session_store::nonce_prefix() const
	{
		return &m_nonce_prefix[0];
	}

	inline size_t session_store::nonce_prefix_size() const
	{
		return m_nonce_prefix.size();
	}

	inline sequence_number_type session_store::sequence_number() const
	{
		return m_sequence_number;
	}

	inline void session_store::set_sequence_number(sequence_number_type _sequence_number)
	{
		m_sequence_number = _sequence_number;
	}

	inline void session_store::increment_sequence_number(size_t cnt)
	{
		if (m_sequence_number + cnt < m_sequence_number)
		{
			throw std::runtime_error("sequence_number overflow");
		}

		m_sequence_number += static_cast<fscp::sequence_number_type>(cnt);
	}
}

#endif /* FSCP_SESSION_STORE_HPP */

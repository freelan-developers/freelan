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

#include <boost/array.hpp>

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
			 * \brief The sequence number type.
			 */
			typedef uint32_t sequence_number_type;

			/**
			 * \brief The key length.
			 */
			static const size_t KEY_LENGTH = 32;

			/**
			 * \brief The iv length.
			 */
			static const size_t IV_LENGTH = 16;

			/**
			 * \brief The key type.
			 */
			typedef boost::array<uint8_t, KEY_LENGTH> key_type;

			/**
			 * \brief The iv type.
			 */
			typedef boost::array<uint8_t, IV_LENGTH> iv_type;

			/**
			 * \brief Create a new random session store.
				 * \param session_number The session number.
			 */
			explicit session_store(session_number_type session_number);

			/**
			 * \brief Create a new session store.
			 * \param session_number The session number.
			 * \param sig_key The signature key.
			 * \param enc_key The encryption key.
			 * \param iv The initialization vector.
			 */
			session_store(session_number_type session_number, key_type sig_key, key_type enc_key, iv_type iv);

			/**
			 * \brief Get the session number.
			 * \return The session number.
			 */
			session_number_type session_number() const;

			/**
			 * \brief Get the signature key.
			 * \return The signature key.
			 */
			const uint8_t* signature_key() const;

			/**
			 * \brief Get the signature key size.
			 * \return The signature key size.
			 */
			size_t signature_key_size() const;

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
			 * \brief Get the initialization vector.
			 * \return The initialization vector.
			 */
			const uint8_t* initialization_vector() const;

			/**
			 * \brief Get the initialization vector size.
			 * \return The initialization vector size.
			 */
			size_t initialization_vector_size() const;

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
			 * \brief Get the sequence initialization vector.
			 * \param sequence_number The sequence number to use to compute the IV.
			 * \return The sequence initialization vector.
			 */
			iv_type sequence_initialization_vector(sequence_number_type sequence_number) const;

			/**
			 * \brief Get the sequence initialization vector, using the current sequence number.
			 * \return The sequence initialization vector.
			 */
			iv_type sequence_initialization_vector() const;

			/**
			 * \brief Check if the session is old.
			 * \return true if the function is old.
			 */
			bool is_old() const;

		private:

			session_number_type m_session_number;
			key_type m_sig_key;
			key_type m_enc_key;
			iv_type m_iv;
			sequence_number_type m_sequence_number;
	};

	inline session_store::session_store(session_number_type _session_number, key_type sig_key, key_type enc_key, iv_type iv) :
		m_session_number(_session_number),
		m_sig_key(sig_key),
		m_enc_key(enc_key),
		m_iv(iv),
		m_sequence_number(0)
	{
	}

	inline session_store::session_number_type session_store::session_number() const
	{
		return m_session_number;
	}

	inline const uint8_t* session_store::signature_key() const
	{
		return m_sig_key.data();
	}

	inline size_t session_store::signature_key_size() const
	{
		return m_sig_key.size();
	}

	inline const uint8_t* session_store::encryption_key() const
	{
		return m_enc_key.data();
	}

	inline size_t session_store::encryption_key_size() const
	{
		return m_enc_key.size();
	}

	inline const uint8_t* session_store::initialization_vector() const
	{
		return m_iv.data();
	}

	inline size_t session_store::initialization_vector_size() const
	{
		return m_iv.size();
	}

	inline session_store::sequence_number_type session_store::sequence_number() const
	{
		return m_sequence_number;
	}

	inline void session_store::set_sequence_number(sequence_number_type _sequence_number)
	{
		m_sequence_number = _sequence_number;
	}

	inline session_store::iv_type session_store::sequence_initialization_vector() const
	{
		return sequence_initialization_vector(m_sequence_number);
	}
}

#endif /* FSCP_SESSION_STORE_HPP */

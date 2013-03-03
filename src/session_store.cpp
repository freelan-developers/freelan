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
 * \file session_store.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session store class.
 */

#include "session_store.hpp"

#include <cryptoplus/random/random.hpp>

#include <cstring>

namespace fscp
{
	session_store::session_store(session_number_type _session_number, const cryptoplus::cipher::cipher_algorithm& _cipher_algorithm, boost::optional<cryptoplus::hash::message_digest_algorithm> _message_digest_algorithm, size_t _message_digest_algorithm_hmac_size) :
		m_session_number(_session_number),
		m_cipher_algorithm(_cipher_algorithm),
		m_message_digest_algorithm(_message_digest_algorithm),
		m_message_digest_algorithm_hmac_size(_message_digest_algorithm_hmac_size),
		m_seal_key(cryptoplus::random::get_random_bytes(m_cipher_algorithm.key_length()).data()),
		m_enc_key(cryptoplus::random::get_random_bytes(m_cipher_algorithm.key_length()).data()),
		m_sequence_number(0)
	{
	}

	session_store::session_store(session_number_type _session_number, const cryptoplus::cipher::cipher_algorithm& _cipher_algorithm, boost::optional<cryptoplus::hash::message_digest_algorithm> _message_digest_algorithm, size_t _message_digest_algorithm_hmac_size, const void* _seal_key, size_t _seal_key_len, const void* _enc_key, size_t _enc_key_len) :
		m_session_number(_session_number),
		m_cipher_algorithm(_cipher_algorithm),
		m_message_digest_algorithm(_message_digest_algorithm),
		m_message_digest_algorithm_hmac_size(_message_digest_algorithm_hmac_size),
		m_seal_key(static_cast<const uint8_t*>(_seal_key), static_cast<const uint8_t*>(_seal_key) + _seal_key_len),
		m_enc_key(static_cast<const uint8_t*>(_enc_key), static_cast<const uint8_t*>(_enc_key) + _enc_key_len),
		m_sequence_number(1)
	{
		if (_seal_key_len != m_cipher_algorithm.key_length())
		{
			throw std::runtime_error("seal_key_len");
		}

		if (_enc_key_len != m_cipher_algorithm.key_length())
		{
			throw std::runtime_error("enc_key_len");
		}
	}

	bool session_store::is_old() const
	{
		return (m_sequence_number > (static_cast<sequence_number_type>(1) << (sizeof(sequence_number_type) * 8 - 1)));
	}
}

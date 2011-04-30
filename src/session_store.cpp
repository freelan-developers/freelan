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

#include <boost/asio.hpp>

namespace fscp
{
	session_store::session_store(session_number_type _session_number) :
		m_session_number(_session_number),
		m_sequence_number(0)
	{
		cryptoplus::random::get_random_bytes(m_sig_key.data(), m_sig_key.size());
		cryptoplus::random::get_random_bytes(m_enc_key.data(), m_enc_key.size());
		cryptoplus::random::get_random_bytes(m_iv.data(), m_iv.size());
		m_current_iv = m_iv;
	}

	session_store::session_store(session_number_type _session_number, const void* _sig_key, size_t _sig_key_len, const void* _enc_key, size_t _enc_key_len, const void* _iv, size_t _iv_len) :
		m_session_number(_session_number),
		m_sequence_number(0)
	{
		if (_sig_key_len != m_sig_key.size())
		{
			throw std::runtime_error("sig_key_len");
		}

		if (_enc_key_len != m_enc_key.size())
		{
			throw std::runtime_error("enc_key_len");
		}

		if (_iv_len != m_iv.size())
		{
			throw std::runtime_error("iv_len");
		}

		std::memcpy(m_sig_key.c_array(), _sig_key, _sig_key_len);
		std::memcpy(m_enc_key.c_array(), _enc_key, _enc_key_len);
		std::memcpy(m_iv.c_array(), _iv, _iv_len);
		m_current_iv = m_iv;
	}

	const uint8_t* session_store::sequence_initialization_vector(sequence_number_type _sequence_number) const
	{
		_sequence_number = htonl(_sequence_number);

		m_current_iv[m_current_iv.size() - 4] ^= (_sequence_number >> 24) & 0xFF;
		m_current_iv[m_current_iv.size() - 3] ^= (_sequence_number >> 16) & 0xFF;
		m_current_iv[m_current_iv.size() - 2] ^= (_sequence_number >> 8) & 0xFF;
		m_current_iv[m_current_iv.size() - 1] ^= (_sequence_number >> 0) & 0xFF;

		return m_current_iv.data();
	}

	bool session_store::is_old() const
	{
		return m_sequence_number > (sequence_number_type(1) << 24);
	}
}

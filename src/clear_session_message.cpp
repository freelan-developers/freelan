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
 * \file clear_session_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A clear session message class.
 */

#include "clear_session_message.hpp"

#include <cassert>
#include <stdexcept>

namespace fscp
{
	size_t clear_session_message::write(void* buf, size_t buf_len, session_number_type _session_number, const challenge_type& _challenge, cipher_algorithm_type calg, const void* enc_key, size_t enc_key_len, const void* nonce_prefix, size_t nonce_prefix_len)
	{
		const size_t result_size = MIN_BODY_LENGTH + enc_key_len + nonce_prefix_len;

		if (buf_len < result_size)
		{
			throw std::runtime_error("buf_len");
		}

		buffer_tools::set<session_number_type>(buf, 0, htonl(_session_number));
		std::copy(_challenge.begin(), _challenge.end(), static_cast<char*>(buf) + sizeof(_session_number));
		buffer_tools::set<uint8_t>(buf, sizeof(session_number_type) + challenge_type::static_size, calg.value());
		buffer_tools::set<uint8_t>(buf, sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t), 0x00);
		buffer_tools::set<uint16_t>(buf, sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 2, htons(0x0000));
		buffer_tools::set<uint16_t>(buf, sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 2 + 2, htons(static_cast<uint16_t>(enc_key_len)));
		std::memcpy(static_cast<uint8_t*>(buf) + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 2 + 2 + sizeof(uint16_t), enc_key, enc_key_len);
		buffer_tools::set<uint16_t>(buf, sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 2 + 2 + sizeof(uint16_t) + enc_key_len, htons(static_cast<uint16_t>(nonce_prefix_len)));
		std::memcpy(static_cast<uint8_t*>(buf) + sizeof(session_number_type) + challenge_type::static_size + sizeof(uint8_t) * 2 + 2 + sizeof(uint16_t) + enc_key_len + sizeof(uint16_t), nonce_prefix, nonce_prefix_len);

		return result_size;
	}

	clear_session_message::clear_session_message(const void* buf, size_t buf_len) :
		m_data(buf)
	{
		if (buf_len < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		if (buf_len < MIN_BODY_LENGTH + encryption_key_size())
		{
			throw std::runtime_error("buf_len");
		}

		if (buf_len < MIN_BODY_LENGTH + encryption_key_size() + nonce_prefix_size())
		{
			throw std::runtime_error("buf_len");
		}
	}
}

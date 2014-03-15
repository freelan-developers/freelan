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
 * \file session.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An session class.
 */

#include "session.hpp"

#include <cassert>
#include <cstring>

namespace fscp
{
	void session::set_remote_parameters(const void* remote_public_key, size_t remote_public_key_size)
	{
		assert(!m_remote_parameters);

		m_remote_parameters = parameters(
			cryptoplus::buffer(remote_public_key, remote_public_key_size),
		);

		const auto dh_shared_secret = m_ecdhe_context.derive_secret_key(m_remote_parameters->public_key());

		// We derive the shared secret so it looks random.
		message_digest_context md_ctx;
		md_ctx.initialize(m_cipher_suite.to_message_digest_algorithm());
		md_ctx.update(dh_shared_secret);
		m_shared_secret = md_ctx.finalize();

		// We resize the shared secret to match the key size of the used cipher algorithm.
		assert(buffer_size(*m_shared_secret) >= m_cipher_suite.to_cipher_algorithm().key_length());

		m_shared_secret->data().resize(m_cipher_suite.to_cipher_algorithm().key_length());
	}

	bool session::match_parameters(cipher_suite_type _cipher_suite, const void* remote_public_key, size_t remote_public_key_size)
	{
		assert(remote_public_key);

		if (_cipher_suite != cipher_suite())
		{
			return false;
		}

		if (!has_remote_parameters())
		{
			return false;
		}

		const size_t buf_len = buffer_size(remote_parameters().public_key());

		if (remote_public_key_size != buf_len)
		{
			return false;
		}

		const uint8_t* const buf = buffer_cast<const uint8_t*>(remote_parameters().public_key());

		if (std::memcmp(buf, static_cast<const uint8_t*>(remote_public_key), buf_len) != 0)
		{
			return false;
		}

		return true;
	}

	bool session::is_old() const
	{
		const auto max = std::numeric_limits<sequence_number_type>::max() / 2;
		return (sequence_number() > max) || (has_remote_parameters() && remote_parameters().sequence_number() > max);
	}
}

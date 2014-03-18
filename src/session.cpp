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

#include <cryptoplus/tls/tls.hpp>

namespace fscp
{
	using cryptoplus::buffer;
	using cryptoplus::buffer_cast;

	void session::set_remote_parameters(const void* _remote_public_key, size_t remote_public_key_size, const host_identifier_type& local_host_identifier, const host_identifier_type& remote_host_identifier)
	{
		assert(!m_remote_parameters);

		const size_t key_length = m_cipher_suite.to_cipher_algorithm().key_length();

		const auto remote_public_key = cryptoplus::buffer(_remote_public_key, remote_public_key_size);

		// We get the derived secret key.
		m_secret_key = boost::make_shared<buffer>(m_ecdhe_context->derive_secret_key(remote_public_key));

		m_shared_secret = boost::make_shared<buffer>(cryptoplus::tls::prf(
			key_length,
			buffer_cast<const void*>(*m_secret_key),
			buffer_size(*m_secret_key),
			"session key",
			local_host_identifier.data.data(),
			local_host_identifier.data.size(),
			get_default_digest_algorithm()
		));

		const auto remote_shared_secret = cryptoplus::tls::prf(
			key_length,
			buffer_cast<const void*>(*m_secret_key),
			buffer_size(*m_secret_key),
			"session key",
			remote_host_identifier.data.data(),
			remote_host_identifier.data.size(),
			get_default_digest_algorithm()
		);

		m_nonce_prefix = boost::make_shared<buffer>(cryptoplus::tls::prf(
			DEFAULT_NONCE_PREFIX_SIZE,
			buffer_cast<const void*>(*m_secret_key),
			buffer_size(*m_secret_key),
			"nonce prefix",
			local_host_identifier.data.data(),
			local_host_identifier.data.size(),
			get_default_digest_algorithm()
		));

		const auto remote_nonce_prefix = cryptoplus::tls::prf(
			DEFAULT_NONCE_PREFIX_SIZE,
			buffer_cast<const void*>(*m_secret_key),
			buffer_size(*m_secret_key),
			"nonce prefix",
			remote_host_identifier.data.data(),
			remote_host_identifier.data.size(),
			get_default_digest_algorithm()
		);

		m_remote_parameters = boost::make_shared<parameters>(
			remote_shared_secret,
			remote_nonce_prefix
		);
	}

	bool session::is_old() const
	{
		const auto max = std::numeric_limits<sequence_number_type>::max() / 2;
		return (sequence_number() > max) || (has_remote_parameters() && remote_parameters().sequence_number() > max);
	}
}

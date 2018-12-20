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
 * \file peer_session.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An peer session storage class.
 */

#include "peer_session.hpp"

#include <cryptoplus/tls/tls.hpp>

namespace fscp
{
	bool peer_session::current_session_type::is_old() const
	{
		const auto max = std::numeric_limits<sequence_number_type>::max() / 2;
		return ((local_sequence_number > max) || (remote_sequence_number > max));
	}

	bool peer_session::set_first_remote_host_identifier(const host_identifier_type& _host_identifier)
	{
		if (!m_remote_host_identifier)
		{
			m_remote_host_identifier = _host_identifier;

			return true;
		}

		return (_host_identifier == *m_remote_host_identifier);
	}

	bool peer_session::prepare_session(session_number_type _session_number, cipher_suite_type _cipher_suite, elliptic_curve_type _elliptic_curve)
	{
		if (m_next_session)
		{
			if ((m_next_session->parameters.session_number == _session_number) && (m_next_session->parameters.cipher_suite == _cipher_suite) && (m_next_session->parameters.elliptic_curve == _elliptic_curve))
			{
				// The session in preparation matches the requested one: not creating one to ensure the private DH key stays the same.
				return false;
			}
		}

		m_next_session = boost::make_shared<next_session_type>(_session_number, _cipher_suite, _elliptic_curve);

		return true;
	}

	bool peer_session::complete_session(const void* _remote_public_key, size_t remote_public_key_size)
	{
		using cryptoplus::buffer_cast;

		if (!m_next_session || !m_remote_host_identifier)
		{
			return false;
		}

		boost::shared_ptr<current_session_type> _current_session = boost::make_shared<current_session_type>(m_next_session->parameters);

		const size_t key_length = m_next_session->parameters.cipher_suite.to_cipher_algorithm().key_length();
		const auto remote_public_key = cryptoplus::buffer(_remote_public_key, remote_public_key_size);

		// We get the derived secret key.
		const auto secret_key = m_next_session->ecdhe_context.derive_secret_key(remote_public_key);

		_current_session->local_session_key = cryptoplus::tls::prf(
			key_length,
			buffer_cast<const void*>(secret_key),
			buffer_size(secret_key),
			"session key",
			m_local_host_identifier.data.data(),
			m_local_host_identifier.data.size(),
			get_default_digest_algorithm()
		);

		_current_session->remote_session_key = cryptoplus::tls::prf(
			key_length,
			buffer_cast<const void*>(secret_key),
			buffer_size(secret_key),
			"session key",
			m_remote_host_identifier->data.data(),
			m_remote_host_identifier->data.size(),
			get_default_digest_algorithm()
		);

		_current_session->local_nonce_prefix = cryptoplus::tls::prf(
			DEFAULT_NONCE_PREFIX_SIZE,
			buffer_cast<const void*>(secret_key),
			buffer_size(secret_key),
			"nonce prefix",
			m_local_host_identifier.data.data(),
			m_local_host_identifier.data.size(),
			get_default_digest_algorithm()
		);

		_current_session->remote_nonce_prefix = cryptoplus::tls::prf(
			DEFAULT_NONCE_PREFIX_SIZE,
			buffer_cast<const void*>(secret_key),
			buffer_size(secret_key),
			"nonce prefix",
			m_remote_host_identifier->data.data(),
			m_remote_host_identifier->data.size(),
			get_default_digest_algorithm()
		);

		m_next_session.reset();
		swap(m_current_session, _current_session);

		keep_alive();

		return true;
	}

	session_number_type peer_session::next_session_number() const
	{
		if (!has_current_session())
		{
			return 0;
		}
		else if (!m_next_session)
		{
			return current_session().parameters.session_number + 1;
		}
		else
		{
			return m_next_session->parameters.session_number;
		}
	}

	const peer_session::session_parameters& peer_session::next_session_parameters() const
	{
		assert(m_next_session);

		return m_next_session->parameters;
	}

	const peer_session::session_parameters& peer_session::current_session_parameters() const
	{
		assert(m_current_session);

		return m_current_session->parameters;
	}

	bool peer_session::set_remote_sequence_number(sequence_number_type sequence_number)
	{
		if (sequence_number > m_current_session->remote_sequence_number)
		{
			m_current_session->remote_sequence_number = sequence_number;

			return true;
		}

		return false;
	}

	bool peer_session::clear()
	{
		clear_remote_host_identifier();

		const bool result = has_current_session();

		m_current_session.reset();
		m_next_session.reset();

		return result;
	}
}

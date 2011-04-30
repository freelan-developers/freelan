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
 * \file identity_store.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An identity store class.
 */

#include "identity_store.hpp"

#include <cassert>
#include <stdexcept>

namespace fscp
{
	identity_store::identity_store(identity_store::cert_type sig_cert, identity_store::key_type sig_key, identity_store::cert_type enc_cert, identity_store::key_type enc_key) :
		m_sig_cert(sig_cert),
		m_sig_key(sig_key),
		m_enc_cert(enc_cert),
		m_enc_key(enc_key)
	{
		assert(m_sig_cert);
		assert(m_sig_key);

		if (!m_enc_cert)
		{
			m_enc_cert = m_sig_cert;
		}

		if (!m_enc_key)
		{
			m_enc_key = m_sig_key;
		}

		if (!m_sig_cert.verify_private_key(m_sig_key))
		{
			throw std::runtime_error("sig_key mismatch");
		}

		if (!m_enc_cert.verify_private_key(m_enc_key))
		{
			throw std::runtime_error("enc_key mismatch");
		}

		if (cryptoplus::x509::compare(m_sig_cert.subject(), m_enc_cert.subject()) != 0)
		{
			throw std::runtime_error("subject name mistmatch");
		}

		if (cryptoplus::x509::compare(m_sig_cert.issuer(), m_enc_cert.issuer()) != 0)
		{
			throw std::runtime_error("issuer name mistmatch");
		}
	}
}

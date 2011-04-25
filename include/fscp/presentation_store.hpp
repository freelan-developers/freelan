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
 * \file presentation_store.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An presentation store class.
 */

#ifndef FSCP_PRESENTATION_STORE_HPP
#define FSCP_PRESENTATION_STORE_HPP

#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/pkey/pkey.hpp>

namespace fscp
{
	/**
	 * \brief An presentation store class.
	 */
	class presentation_store
	{
		public:

			/**
			 * \brief The certificate type.
			 */
			typedef cryptoplus::x509::certificate cert_type;

			/**
			 * \brief Create a new presentation store.
			 * \param sig_cert The signature certificate. Cannot be null.
			 * \param enc_cert The encryption certificate. If enc_cert is null, sig_cert is taken instead.
			 * \warning Both certificate must have the same subject and issuer names or a std::runtime_error is thrown.
			 */
			presentation_store(cert_type sig_cert, cert_type enc_cert = cert_type());

			/**
			 * \brief Get the signature certificate.
			 * \return The signature certificate.
			 */
			cert_type signature_certificate() const;

			/**
			 * \brief Get the encryption certificate.
			 * \return The encryption certificate.
			 */
			cert_type encryption_certificate() const;

		private:

			cert_type m_sig_cert;
			cert_type m_enc_cert;
	};

	inline presentation_store::cert_type presentation_store::signature_certificate() const
	{
		return m_sig_cert;
	}

	inline presentation_store::cert_type presentation_store::encryption_certificate() const
	{
		return m_enc_cert;
	}
}

#endif /* FSCP_PRESENTATION_STORE_HPP */

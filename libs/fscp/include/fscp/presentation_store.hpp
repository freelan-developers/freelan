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
 * \file presentation_store.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An presentation store class.
 */

#ifndef FSCP_PRESENTATION_STORE_HPP
#define FSCP_PRESENTATION_STORE_HPP

#include "constants.hpp"

#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/pkey/pkey.hpp>

#include <list>

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
			 * \brief Create an empty presentation_store.
			 */
			presentation_store() {};

			/**
			 * \brief Create a new presentation store.
			 * \param sig_cert The signature certificate.
			 * \param pre_shared_key The pre-shared key.
			 */
			explicit presentation_store(cert_type sig_cert, const cryptoplus::buffer& pre_shared_key);

			/**
			 * \brief Check if the presentation store is empty.
			 * \return true if the presentation store is empty.
			 */
			bool empty() const
			{
				return !m_sig_cert && !m_pre_shared_key;
			}

			/**
			 * \brief Get the signature certificate.
			 * \return The signature certificate.
			 */
			cert_type signature_certificate() const
			{
				return m_sig_cert;
			}

			/**
			 * \brief Get the signature certificate hash.
			 * \return The signature certificate hash.
			 */
			const boost::optional<hash_type>& signature_certificate_hash() const
			{
				return m_sig_hash;
			}

			/**
			* \brief Get the pre-shared key.
			* \return The pre-shared key.
			*/
			const cryptoplus::buffer& pre_shared_key() const
			{
				return m_pre_shared_key;
			}

		private:

			cert_type m_sig_cert;
			boost::optional<hash_type> m_sig_hash;
			cryptoplus::buffer m_pre_shared_key;
	};
}

#endif /* FSCP_PRESENTATION_STORE_HPP */

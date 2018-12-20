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
 * \file presentation_message.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A presentation message class.
 */

#ifndef FSCP_PRESENTATION_MESSAGE_HPP
#define FSCP_PRESENTATION_MESSAGE_HPP

#include "message.hpp"

#include <cryptoplus/x509/certificate.hpp>

namespace fscp
{
	/**
	 * \brief A presentation message class.
	 */
	class presentation_message : public message
	{
		public:

			/**
			 * \brief The certificate type.
			 */
			typedef cryptoplus::x509::certificate cert_type;

			/**
			 * \brief Write a presentation message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param sig_cert The signature certificate. Cannot be null.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, cert_type sig_cert);

			/**
			 * \brief Create a presentation_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			presentation_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a presentation_message from a message.
			 * \param message The message.
			 */
			presentation_message(const message& message);

			/**
			 * \brief Get the signature certificate.
			 * \return The signature certificate.
			 * \warning The returned certificate is parsed from the underlying buffer on every call so storing the result might be a good idea.
			 */
			cert_type signature_certificate() const;

		protected:

			/**
			 * \brief The minimum body length.
			 */
			static const size_t MIN_BODY_LENGTH = sizeof(uint16_t);

		private:

			void check_format() const;
	};
}

#endif /* FSCP_PRESENTATION_MESSAGE_HPP */

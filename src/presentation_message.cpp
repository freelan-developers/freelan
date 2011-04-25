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
 * \file presentation_message.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A presentation message class.
 */

#include "presentation_message.hpp"

#include <cassert>
#include <stdexcept>
#include <cstring>

namespace fscp
{
	size_t presentation_message::write(void* buf, size_t buf_len, presentation_message::cert_type sig_cert, presentation_message::cert_type enc_cert)
	{
		size_t sig_len = sig_cert ? sig_cert.write_der(static_cast<void*>(0)) : 0;
		size_t enc_len = enc_cert ? enc_cert.write_der(static_cast<void*>(0)) : 0;

		if (buf_len < HEADER_LENGTH + MIN_BODY_LENGTH + sig_len + enc_len)
		{
			throw std::runtime_error("buf_len");
		}

		char* pbuf = static_cast<char*>(buf) + HEADER_LENGTH;

		buffer_tools::set<uint16_t>(pbuf, 0, htons(static_cast<uint16_t>(sig_len)));
		pbuf += sizeof(uint16_t);

		if (sig_cert)
		{
			pbuf += sig_cert.write_der(pbuf);
		}

		buffer_tools::set<uint16_t>(pbuf, 0, htons(static_cast<uint16_t>(enc_len)));
		pbuf += sizeof(uint16_t);

		if (enc_cert)
		{
			pbuf += enc_cert.write_der(pbuf);
		}

		message::write(buf, buf_len, CURRENT_PROTOCOL_VERSION, MESSAGE_TYPE_PRESENTATION, pbuf - static_cast<char*>(buf) - HEADER_LENGTH);

		return pbuf - static_cast<char*>(buf);
	}

	presentation_message::presentation_message(const void* buf, size_t buf_len) :
		message(buf, buf_len)
	{
		check_format();
	}

	presentation_message::presentation_message(const message& _message) :
		message(_message)
	{
		check_format();
	}

	presentation_message::cert_type presentation_message::signature_certificate() const
	{
		uint16_t sig_len = ntohs(buffer_tools::get<uint16_t>(payload(), 0));

		return cert_type::from_der(payload() + sizeof(uint16_t), sig_len);
	}

	presentation_message::cert_type presentation_message::encryption_certificate() const
	{
		uint16_t sig_len = ntohs(buffer_tools::get<uint16_t>(payload(), 0));
		uint16_t enc_len = ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(uint16_t) + sig_len));

		if (enc_len == 0)
		{
			return signature_certificate();
		}
		else
		{
			return cert_type::from_der(payload() + MIN_BODY_LENGTH + sig_len, enc_len);
		}
	}

	void presentation_message::check_format() const
	{
		if (length() < MIN_BODY_LENGTH)
		{
			throw std::runtime_error("bad message length");
		}

		uint16_t sig_len = ntohs(buffer_tools::get<uint16_t>(payload(), 0));

		if (sig_len == 0)
		{
			throw std::runtime_error("invalid sig_len");
		}

		cert_type sig_cert = cert_type::from_der(payload() + sizeof(uint16_t), sig_len);

		if (length() < MIN_BODY_LENGTH + sig_len)
		{
			throw std::runtime_error("sig_len value mismatch");
		}

		uint16_t enc_len = ntohs(buffer_tools::get<uint16_t>(payload(), sizeof(uint16_t) + sig_len));

		if (enc_len == 0)
		{
			if (length() != MIN_BODY_LENGTH + sig_len)
			{
				throw std::runtime_error("unexpected data after null enc_len");
			}
		}
		else
		{
			cert_type enc_cert = cert_type::from_der(payload() + MIN_BODY_LENGTH + sig_len, enc_len);

			if (cryptoplus::x509::compare(sig_cert.subject(), enc_cert.subject()) != 0)
			{
				throw std::runtime_error("certificate subject name do not match");
			}

			if (cryptoplus::x509::compare(sig_cert.issuer(), enc_cert.issuer()) != 0)
			{
				throw std::runtime_error("certificate issuer name do not match");
			}
		}
	}
}

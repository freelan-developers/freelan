/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file certificate.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A X509 certificate class.
 */

#include "x509/certificate.hpp"

#include "bio/bio_chain.hpp"

#include <cassert>

namespace cryptoplus
{
	template <>
	x509::certificate::deleter_type pointer_wrapper<x509::certificate::value_type>::deleter = X509_free;

	namespace x509
	{
		namespace
		{
			bio::bio_chain get_bio_chain_from_buffer(const void* buf, size_t buf_len)
			{
				return bio::bio_chain(BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(buf_len)));
			}
		}

		certificate certificate::from_certificate(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_certificate(bio_chain.first(), callback, callback_arg);
		}

		certificate certificate::from_trusted_certificate(const void* buf, size_t buf_len, pem_passphrase_callback_type callback, void* callback_arg)
		{
			bio::bio_chain bio_chain = get_bio_chain_from_buffer(buf, buf_len);

			return from_trusted_certificate(bio_chain.first(), callback, callback_arg);
		}

		certificate certificate::take_ownership(pointer _ptr)
		{
			throw_error_if_not(_ptr);

			return certificate(_ptr, deleter);
		}

		std::string certificate::to_string() const
		{
			bio::bio_ptr bio(BIO_new(BIO_s_mem()));

			this->print(bio);

			BUF_MEM* const str = bio.get_mem_buf();

			return std::string(str->data, str->length);
		}
	}
}


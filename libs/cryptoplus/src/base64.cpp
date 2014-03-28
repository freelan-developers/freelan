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
 * \file base64.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Base 64 related functions.
 */

#include "base64.hpp"

#include "bio/bio_chain.hpp"
#include "bio/bio_ptr.hpp"

#include <openssl/buffer.h>

namespace cryptoplus
{
	size_t base64_encode(void* output, size_t outputlen, const void* input, size_t inputlen)
	{
		bio::bio_chain bio_chain(BIO_f_base64());
		bio_chain.first().set_flags(BIO_FLAGS_BASE64_NO_NL);
		bio_chain.first().push(BIO_new(BIO_s_mem()));

		ptrdiff_t cnt = bio_chain.first().write(input, inputlen);

		if (cnt <= 0)
		{
			throw std::runtime_error("Unable to perform base 64 encoding");
		}

		bio_chain.first().flush();

		BUF_MEM* b64ptr = bio_chain.first().next().get_mem_buf();

		const size_t min_size = std::min(static_cast<size_t>(b64ptr->length), outputlen);

		std::copy(static_cast<const char*>(b64ptr->data), static_cast<const char*>(b64ptr->data) + min_size, static_cast<char*>(output));

		return min_size;
	}

	size_t base64_decode(void* output, size_t outputlen, const void* input, size_t inputlen)
	{
		bio::bio_chain bio_chain(BIO_f_base64());
		bio_chain.first().set_flags(BIO_FLAGS_BASE64_NO_NL);
		bio_chain.first().push(BIO_new_mem_buf(const_cast<void*>(input), static_cast<int>(inputlen)));

		ptrdiff_t cnt = bio_chain.first().read(output, outputlen);

		if (cnt <= 0)
		{
			throw std::runtime_error("Unable to perform base 64 decoding");
		}

		return static_cast<size_t>(cnt);
	}
}

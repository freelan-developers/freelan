/*
 * libsystools - open-source and multi-platform toolset C++ library.
 * Copyright 2007-2010 <julien.kauffmann@freelan.org>
 *
 * This file is part of libsystools.
 *
 * libsystools is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libsystools is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
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
 * If you intend to use libsystools in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file hmac.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A HMAC helper class.
 */

#include "hmac.hpp"

namespace systools
{
	size_t HMAC::hmac(const void* key, size_t keylen, const void* buf, size_t buflen, void* outbuf, size_t outbuflen) const
	{
		HMAC_CTX ctx;
		HMAC_CTX_init(&ctx);

		unsigned int len = static_cast<unsigned int>(outbuflen);

		HMAC_Init_ex(&ctx, key, static_cast<int>(keylen), d_hash_method, NULL);
		HMAC_Update(&ctx, static_cast<const unsigned char*>(buf), static_cast<int>(buflen));
		HMAC_Final(&ctx, static_cast<unsigned char*>(outbuf), &len);

		HMAC_CTX_cleanup(&ctx);

		return len;
	}

	SmartBuffer HMAC::hmac(const SecureBuffer& key, const SmartBuffer& buf) const
	{
		SmartBuffer result(size());

		result.resize(hmac(key.constData(), key.size(), buf.constData(), buf.size(), result.data(), result.heapSize()));

		return result;
	}
}


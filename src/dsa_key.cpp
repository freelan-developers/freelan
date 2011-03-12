/*
 * libcryptopen - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptopen.
 *
 * libcryptopen is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptopen is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptopen in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file dsa_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DSA key class.
 */

#include "pkey/dsa_key.hpp"

#include <cassert>

namespace cryptopen
{
	namespace pkey
	{
		dsa_key dsa_key::generate_parameters(int bits, void* seed, size_t seed_len, int* counter_ret, unsigned long *h_ret, generate_callback_type callback, void* callback_arg)
		{
			return dsa_key(boost::shared_ptr<DSA>(DSA_generate_parameters(bits, static_cast<unsigned char*>(seed), seed_len, counter_ret, h_ret, callback, callback_arg), DSA_free));
		}

		size_t dsa_key::sign(void* out, size_t out_len, const void* buf, size_t buf_len, int type)
		{
			unsigned int _out_len = out_len;

			error::throw_error_if_not(DSA_sign(type, static_cast<const unsigned char*>(buf), buf_len, static_cast<unsigned char*>(out), &_out_len, m_dsa.get()));

			return _out_len;
		}

		void dsa_key::verify(const void* _sign, size_t sign_len, const void* buf, size_t buf_len, int type)
		{
			error::throw_error_if_not(DSA_verify(type, static_cast<const unsigned char*>(buf), buf_len, static_cast<const unsigned char*>(_sign), sign_len, m_dsa.get()));
		}
	}
}


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
 * \file cryptoplus.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The global cryptoplus include file.
 */

#ifndef CRYPTOPLUS_CRYPTOPLUS_HPP
#define CRYPTOPLUS_CRYPTOPLUS_HPP

#include "initializer.hpp"

#include <openssl/evp.h>

namespace cryptoplus
{
	namespace
	{
		/**
		 * \brief A function wrapper to call the OpenSSL_add_all_algorithms macro.
		 */
		inline void _OpenSSL_add_all_algorithms()
		{
			OpenSSL_add_all_algorithms();
		}

		/**
		 * \brief A function that does nothing.
		 */
		inline void _null_function()
		{
		}
	}

	/**
	 * \brief The algorithms initializer.
	 *
	 * Only one instance of this class should be created. When an instance exists, the library can proceed to name resolutions.
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	typedef initializer<_OpenSSL_add_all_algorithms, _null_function> algorithms_initializer;
#else
	typedef initializer<_OpenSSL_add_all_algorithms, EVP_cleanup> algorithms_initializer;
#endif

	/**
	 * \brief The crypto initializer.
	 *
	 * Only one instance of this class should be created. When an instance exists, it will prevent memory leaks related to the libcrypto's internals.
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	typedef initializer<_null_function, _null_function> crypto_initializer;
#else
	typedef initializer<_null_function, CRYPTO_cleanup_all_ex_data> crypto_initializer;
#endif
}

#endif /* CRYPTOPLUS_CRYPTOPLUS_HPP */

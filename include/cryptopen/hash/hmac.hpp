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
 * \file hash.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A HMAC helper class.
 */

#ifndef SYSTOOLS_HMAC_HPP
#define SYSTOOLS_HMAC_HPP

#include "smartbuffer.hpp"
#include "securebuffer.hpp"

#include <openssl/hmac.h>

#include <boost/noncopyable.hpp>

#include <string>

namespace systools
{
	/**
	 * \brief A HMAC helper class.
	 *
	 * The HMAC class allows the computation of HMAC.
	 *
	 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
	 */
	class HMAC : public boost::noncopyable
	{
		public:

			/**
			 * \brief Compute the HMAC for a given buffer.
			 * \param key The key to use.
			 * \param keylen The key length.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 * \param outbuf The resulting buffer.
			 * \param outbuflen The resulting buffer length, must be big enough to hold all the returned data for the specified hash method.
			 * \param hash_method The hash method to use.
			 * \return The count of bytes written.
			 *
			 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
			 */
			static size_t hmac(const void* key, size_t keylen, const void* buf, size_t buflen, void* outbuf, size_t outbuflen, const EVP_MD* hash_method);

			/**
			 * \brief HMAC a buffer.
			 * \param key The key.
			 * \param buf The buffer.
			 * \param hash_method The hash method to use.
			 * \return The hash.
			 *
			 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
			 */
			static SmartBuffer hmac(const SecureBuffer& key, const SmartBuffer& buf, const EVP_MD* hash_method);

			/**
			 * \brief Create a new HMAC object.
			 * \param hash_method The hash method to use. Default is EVP_sha256().
			 *
			 * The list of the available hash methods depends on the version of OpenSSL and can be found on the man page of EVP_DigestInit().
			 */
			HMAC(const EVP_MD* hash_method = EVP_sha256());

			/**
			 * \brief Virtual destructor.
			 */
			virtual ~HMAC() {}

			/**
			 * \brief HMAC a buffer.
			 * \param key The key.
			 * \param keylen The key length.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 * \param outbuf The resulting buffer.
			 * \param outbuflen The resulting buffer length, must be at least size(). If outbuflen is less than size(), the behavior is undefined.
			 * \return The count of bytes written. Should be size().
			 * \see size
			 */
			virtual size_t hmac(const void* key, size_t keylen, const void* buf, size_t buflen, void* outbuf, size_t outbuflen) const;

			/**
			 * \brief HMAC a smartbuffer.
			 * \param key The key.
			 * \param buf A buffer.
			 * \return The digest.
			 */
			SmartBuffer hmac(const SecureBuffer& key, const SmartBuffer& buf) const;

			/**
			 * \brief Get the size of the HMAC.
			 * \return The size of the resulting HMAC.
			 */
			size_t size() const;

		protected:

			/**
			 * \brief Get the hash method.
			 * \return The hash method.
			 */
			const EVP_MD* hashMethod() const;

		private:

			/**
			 * \brief The hash method.
			 */
			const EVP_MD* d_hash_method;
	};

	inline size_t HMAC::hmac(const void* key, size_t keylen, const void* buf, size_t buflen, void* outbuf, size_t outbuflen, const EVP_MD* hash_method)
	{
		return HMAC(hash_method).hmac(key, keylen, buf, buflen, outbuf, outbuflen);
	}
	inline SmartBuffer HMAC::hmac(const SecureBuffer& key, const SmartBuffer& buf, const EVP_MD* hash_method)
	{
		return HMAC(hash_method).hmac(key, buf);
	}
	inline HMAC::HMAC(const EVP_MD* hash_method) : d_hash_method(hash_method) {}
	inline size_t HMAC::size() const
	{
		return EVP_MD_size(d_hash_method);
	}
	inline const EVP_MD* HMAC::hashMethod() const
	{
		return d_hash_method;
	}
}

#endif /* SYSTOOLS_HMAC_HPP */


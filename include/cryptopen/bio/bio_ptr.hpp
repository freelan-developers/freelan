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
 * \file bio_ptr.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A bio pointer class.
 */

#ifndef CRYPTOPEN_BIO_BIO_PTR_HPP
#define CRYPTOPEN_BIO_BIO_PTR_HPP

#include "nullable.hpp"
#include "cipher/cipher_algorithm.hpp"
#include "cipher/cipher_context.hpp"

#include <openssl/bio.h>

#include <cstddef>

namespace cryptopen
{
	namespace bio
	{
		/**
		 * \brief An OpenSSL BIO pointer.
		 *
		 * The bio class is a wrapper for an OpenSSL BIO* pointer.
		 *
		 * A bio instance has the same semantic as a BIO* pointer, thus two copies of the same instance share the same underlying pointer.
		 *
		 * A bio *DOES NOT* own its underlying pointer. It is the caller's responsibility to ensure that a bio_ptr always points to a valid BIO structure.
		 *
		 * If you require a wrapper for OpenSSL BIO with ownership semantic, see bio_chain.
		 *
		 * \warning Always check for the bio_ptr not to be NULL before calling any of its method. Calling any method (except raw() or reset_ptr()) on a null bio_ptr has undefined behavior.
		 */
		class bio_ptr : public nullable<bio_ptr>
		{
			public:

				/**
				 * \brief Create a new bio_ptr.
				 * \param bio The bio to point to.
				 */
				explicit bio_ptr(BIO* bio = NULL);

				/**
				 * \brief Reset the underlying pointer.
				 * \param bio The bio to point to.
				 */
				void reset_ptr(BIO* bio = NULL);

				/**
				 * \brief Get the raw BIO pointer.
				 * \return The raw BIO pointer.
				 */
				BIO* raw();

				/**
				 * \brief Push a bio_ptr at the bottom of the BIO chain.
				 * \param bio The bio to append to the BIO chain.
				 * \returns The current bio_ptr.
				 */
				bio_ptr push(bio_ptr& bio);

				/**
				 * \brief Remove the bio_ptr from its BIO chain.
				 * \return The next bio_ptr in the chain, or a null one if there is no next bio_ptr.
				 * 
				 * Once it is removed from its chain, the bio_ptr can be either deleted or added to another chain.
				 */
				bio_ptr pop();

				/**
				 * \brief Find a BIO in the BIO chain by its type.
				 * \param type The type of the bio_ptr.
				 * \return The first bio_ptr to match or an empty one if none is found that match the specified type.
				 */
				bio_ptr find_by_type(int type);

				/**
				 * \brief Get the next bio_ptr in the chain.
				 * \return The next bio_ptr in the chain.
				 */
				bio_ptr next();

				/**
				 * \brief Get the type of the bio.
				 * \return The type.
				 *
				 * The list of possible types is available on the man page for BIO_find_type(3).
				 */
				int type();

				/**
				 * \brief Determine if the last operation on the BIO should be retried.
				 * \return true if the last operation should be retried.
				 */
				bool should_retry();

				/**
				 * \brief Determine if the BIO should be read.
				 * \return true if the BIO should be read.
				 */
				bool should_read();

				/**
				 * \brief Determine if the BIO should be written.
				 * \return true if the BIO should be written.
				 */
				bool should_write();

				/**
				 * \brief Determine if the cause of the last failure was due to a special IO event.
				 * \return true if the cause of the last failure was due to a special IO event.
				 */
				bool should_io_special();

				/**
				 * \brief Get the retry type.
				 * \return The retry type, as specified on the man page of BIO_should_retry(3).
				 */
				int retry_type();

				/**
				 * \brief Determine the precise reason for the special condition and returns the BIO that caused it.
				 * \param reason If not NULL, *reason will be set to the reason code.
				 * \return The BIO that caused the special condition.
				 * \see get_retry_reason
				 */
				bio_ptr get_retry_bio(int* reason = NULL);

				/**
				 * \brief Get the reason for a special condition.
				 * \return The reason code.
				 * \see get_retry_bio
				 */
				int get_retry_reason();

				/**
				 * \brief Read some data from the BIO.
				 * \param buf The buffer to read the data to.
				 * \param buf_len The length of buf.
				 * \return The amount of data successfully read. If the return value is 0 or -1, no data could be read. If it is -2, then the operation is not available for the specific BIO type.
				 */
				ptrdiff_t read(void* buf, size_t buf_len);

				/**
				 * \brief Read a line of data from the BIO.
				 * \param buf The buffer to read the data to.
				 * \param buf_len The length of buf.
				 * \return The amount of data successfully read. If the return value is 0 or -1, no data could be read. If it is -2, then the operation is not available for the specific BIO type.
				 */
				ptrdiff_t gets(char* buf, size_t buf_len);

				/**
				 * \brief Write some data to the BIO.
				 * \param buf The buffer to write the data from.
				 * \param buf_len The length of buf.
				 * \return The amount of data successfully written. If the return value is 0 or -1, no data could be written. If it is -2, then the operation is not available for the specific BIO type.
				 */
				ptrdiff_t write(const void* buf, size_t buf_len);

				/**
				 * \brief Write a line of data to the BIO.
				 * \param buf The buffer to write the data from.
				 * \return The amount of data successfully written. If the return value is 0 or -1, no data could be written. If it is -2, then the operation is not available for the specific BIO type.
				 */
				ptrdiff_t puts(const char* buf);

				/**
				 * \brief Reset the BIO to its initial state.
				 * \return 1 for success, 0 or -1 for failure. If the BIO is a file bio, then 0 means success and -1 means failure.
				 *
				 * Please don't blame me for the inconsistent return values: take a look at BIO_ctrl(3) for the reason.
				 */
				int reset();

				/**
				 * \brief Set the file position pointer.
				 * \param offset The offset value.
				 * \return The current file position on success, and -1 for failure except file BIOs which for seek() return 0 for success and -1 for failure.
				 *
				 * Please don't blame me for the inconsistent return values: take a look at BIO_ctrl(3) for the reason.
				 */
				ptrdiff_t seek(ptrdiff_t offset);

				/**
				 * \brief Get the current file position.
				 * \return The current file position on success, and -1 for failure except file BIOs which for seek() return 0 for success and -1 for failure.
				 */
				ptrdiff_t tell();

				/**
				 * \brief Write out any internally buffered data.
				 * \return 1 for success and 0 or -1 for failure.
				 */
				int flush();

				/**
				 * \brief Determine if the BIO has reached EOF.
				 * \return true if the BIO has reached EOF.
				 */
				bool eof();

				/**
				 * \brief Set the BIO close flag.
				 * \param close The close flag. Can be either BIO_CLOSE or BIO_NOCLOSE.
				 */
				void set_close(long close);

				/**
				 * \brief Get the BIO close flag.
				 * \return The BIO close flag.
				 */
				long get_close();

				/**
				 * \brief Return the number of pending read characters.
				 * \return The number of pending read characters.
				 */
				size_t pending_read();

				/**
				 * \brief Return the number of pending write characters.
				 * \return The number of pending write characters.
				 */
				size_t pending_write();

				// BIO_f_buffer() specific methods

				/**
				 * \brief Get the number of lines currently buffered.
				 * \return The number of lines currently buffered.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_buffer()".
				 */
				unsigned int get_buffer_num_lines();

				/**
				 * \brief Set the read buffer size.
				 * \param size The size to set.
				 * \return true if the set succeeded.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_buffer()".
				 */
				bool set_read_buffer_size(size_t size);

				/**
				 * \brief Set the write buffer size.
				 * \param size The size to set.
				 * \return true if the set succeeded.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_buffer()".
				 */
				bool set_write_buffer_size(size_t size);

				/**
				 * \brief Set both read and write buffer sizes.
				 * \param size The size to set.
				 * \return true if the set succeeded.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_buffer()".
				 */
				bool set_buffer_size(size_t size);

				/**
				 * \brief Set the buffer read data.
				 * \param buf The data.
				 * \param buf_len The length of buf.
				 * \return true if the set succeeded.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_buffer()".
				 */
				bool set_buffer_read_data(const void* buf, size_t buf_len);

				// BIO_f_cipher() specific methods

				/**
				 * \brief Set the cipher associated to the BIO.
				 * \param algorithm The cipher algorithm to use.
				 * \param key The key buffer.
				 * \param iv The IV buffer.
				 * \param direction The cipher direction. Only possible values are cipher::cipher_context::decrypt and cipher::cipher_context::encrypt.
				 *
				 * This method only makes sense for BIOs of type "BIO_f_cipher()".
				 */
				void set_cipher(cipher::cipher_algorithm algorithm, const void* key, const void* iv, cipher::cipher_context::cipher_direction direction);

				/**
				 * \brief Determine whether the decryption operation was successful.
				 * \return true for success.
				 */
				bool get_cipher_status();

				/**
				 * \brief Get the associated cipher context.
				 * \return The associated cipher context.
				 */
				EVP_CIPHER_CTX* get_cipher_ctx();

			private:

				bool boolean_test() const;

				BIO* m_bio;
				
				friend bool operator==(const bio_ptr& lhs, const bio_ptr& rhs);
		};

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance share the same underlying pointer.
		 */
		bool operator==(const bio_ptr& lhs, const bio_ptr& rhs);

		/**
		 * \brief Compare two bio instances.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two bio instance do not share the same underlying pointer.
		 */
		bool operator!=(const bio_ptr& lhs, const bio_ptr& rhs);

		inline bio_ptr::bio_ptr(BIO* _bio) : m_bio(_bio)
		{
		}
		inline void bio_ptr::reset_ptr(BIO* _bio)
		{
			m_bio = _bio;
		}
		inline BIO* bio_ptr::raw()
		{
			return m_bio;
		}
		inline bio_ptr bio_ptr::push(bio_ptr& bio)
		{
			return bio_ptr(BIO_push(m_bio, bio.raw()));
		}
		inline bio_ptr bio_ptr::pop()
		{
			return bio_ptr(BIO_pop(m_bio));
		}
		inline bio_ptr bio_ptr::find_by_type(int _type)
		{
			return bio_ptr(BIO_find_type(m_bio, _type));
		}
		inline bio_ptr bio_ptr::next()
		{
			return bio_ptr(BIO_next(m_bio));
		}
		inline int bio_ptr::type()
		{
			return BIO_method_type(m_bio);
		}
		inline bool bio_ptr::should_retry()
		{
			return BIO_should_retry(m_bio) != 0;
		}
		inline bool bio_ptr::should_read()
		{
			return BIO_should_read(m_bio) != 0;
		}
		inline bool bio_ptr::should_write()
		{
			return BIO_should_write(m_bio) != 0;
		}
		inline bool bio_ptr::should_io_special()
		{
			return BIO_should_io_special(m_bio) != 0;
		}
		inline int bio_ptr::retry_type()
		{
			return BIO_retry_type(m_bio);
		}
		inline bio_ptr bio_ptr::get_retry_bio(int* reason)
		{
			return bio_ptr(BIO_get_retry_BIO(m_bio, reason));
		}
		inline int bio_ptr::get_retry_reason()
		{
			return BIO_get_retry_reason(m_bio);
		}
		inline ptrdiff_t bio_ptr::read(void* buf, size_t buf_len)
		{
			return BIO_read(m_bio, buf, static_cast<int>(buf_len));
		}
		inline ptrdiff_t bio_ptr::gets(char* buf, size_t buf_len)
		{
			return BIO_gets(m_bio, buf, static_cast<int>(buf_len));
		}
		inline ptrdiff_t bio_ptr::write(const void* buf, size_t buf_len)
		{
			return BIO_write(m_bio, buf, static_cast<int>(buf_len));
		}
		inline ptrdiff_t bio_ptr::puts(const char* buf)
		{
			return BIO_puts(m_bio, buf);
		}
		inline int bio_ptr::reset()
		{
			return BIO_reset(m_bio);
		}
		inline ptrdiff_t bio_ptr::seek(ptrdiff_t offset)
		{
			return BIO_seek(m_bio, static_cast<int>(offset));
		}
		inline ptrdiff_t bio_ptr::tell()
		{
			return BIO_tell(m_bio);
		}
		inline int bio_ptr::flush()
		{
			return BIO_flush(m_bio);
		}
		inline bool bio_ptr::eof()
		{
			return BIO_eof(m_bio) != 0;
		}
		inline void bio_ptr::set_close(long _close)
		{
			BIO_set_close(m_bio, _close);
		}
		inline long bio_ptr::get_close()
		{
			return BIO_get_close(m_bio);
		}
		inline size_t bio_ptr::pending_read()
		{
			return BIO_ctrl_pending(m_bio);
		}
		inline size_t bio_ptr::pending_write()
		{
			return BIO_ctrl_wpending(m_bio);
		}
		inline unsigned int bio_ptr::get_buffer_num_lines()
		{
			return BIO_get_buffer_num_lines(m_bio);
		}
		inline bool bio_ptr::set_read_buffer_size(size_t size)
		{
			return BIO_set_read_buffer_size(m_bio, size) > 0;
		}
		inline bool bio_ptr::set_write_buffer_size(size_t size)
		{
			return BIO_set_write_buffer_size(m_bio, size) > 0;
		}
		inline bool bio_ptr::set_buffer_size(size_t size)
		{
			return BIO_set_buffer_size(m_bio, size) > 0;
		}
		inline bool bio_ptr::set_buffer_read_data(const void* buf, size_t buf_len)
		{
			return BIO_set_buffer_read_data(m_bio, const_cast<void*>(buf), buf_len) > 0;
		}
		inline void bio_ptr::set_cipher(cipher::cipher_algorithm algorithm, const void* key, const void* iv, cipher::cipher_context::cipher_direction direction)
		{
			BIO_set_cipher(m_bio, algorithm.raw(), static_cast<const unsigned char*>(key), static_cast<const unsigned char*>(iv), static_cast<int>(direction));
		}
		inline bool bio_ptr::get_cipher_status()
		{
			return BIO_get_cipher_status(m_bio) != 0;
		}
		inline EVP_CIPHER_CTX* bio_ptr::get_cipher_ctx()
		{
			EVP_CIPHER_CTX* ctx = NULL;

			BIO_get_cipher_ctx(m_bio, &ctx);

			return ctx;
		}
		inline bool bio_ptr::boolean_test() const
		{
			return (m_bio != NULL);
		}
		inline bool operator==(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return lhs.m_bio == rhs.m_bio;
		}
		inline bool operator!=(const bio_ptr& lhs, const bio_ptr& rhs)
		{
			return !(lhs == rhs);
		}
	}
}

#endif /* CRYPTOPEN_BIO_BIO_PTR_HPP */


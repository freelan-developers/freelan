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
 * \file random.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Randomization helper functions.
 */

#ifndef CRYPTOPLUS_RANDOM_RANDOM_HPP
#define CRYPTOPLUS_RANDOM_RANDOM_HPP

#include "../error/cryptographic_exception.hpp"
#include "../os.hpp"

#include <openssl/rand.h>

#include <string>

#include <cstddef>

namespace cryptoplus
{
	namespace random
	{
		/**
		 * \brief Set the randomization engine.
		 * \param engine The randomization engine.
		 *
		 * On error, a cryptographic_exception is thrown.
		 */
		void set_randomization_engine(ENGINE* engine);

		/**
		 * \brief Get truly random bytes.
		 * \param buf The buffer to fill with the random bytes. Its content will be mixed in the enthropy pool unless disabled at OpenSSL compile time.
		 * \param buf_len The number of random bytes to request. buf must be big enough to hold the data.
		 * \see get_pseudo_random_bytes
		 *
		 * If the PRNG was not seeded with enough randomness, the call fails and a cryptographic_exception is thrown.
		 */
		void get_random_bytes(void* buf, size_t buf_len);

		/**
		 * \brief Get truly random bytes.
		 * \param cnt The count of random bytes to get.
		 * \return The random bytes.
		 * \see get_pseudo_random_bytes
		 *
		 * If the PRNG was not seeded with enough randomness, the call fails and a cryptographic_exception is thrown.
		 */
		std::string get_random_bytes(size_t cnt);

		/**
		 * \brief Get pseudo random bytes.
		 * \param buf The buffer to fill with the random bytes. Its content will be mixed in the enthropy pool unless disabled at OpenSSL compile time.
		 * \param buf_len The number of random bytes to request. buf must be big enough to hold the data.
		 * \return true if the generated numbers are cryptographically strong, false otherwise.
		 * \see get_random_bytes
		 *
		 * Do not use the resulting bytes for critical cryptographic purposes (like key generation). If require truly random bytes, see get_random_bytes().
		 *
		 * If the PRNG was not seeded with enough randomness, the call fails and a cryptographic_exception is thrown.
		 */
		bool get_pseudo_random_bytes(void* buf, size_t buf_len);

		/**
		 * \brief Get pseudo random bytes.
		 * \param cnt The count of random bytes to get.
		 * \return true if the generated numbers are cryptographically strong, false otherwise.
		 * \see get_random_bytes
		 *
		 * Do not use the resulting bytes for critical cryptographic purposes (like key generation). If require truly random bytes, see get_random_bytes().
		 *
		 * If the PRNG was not seeded with enough randomness, the call fails and a cryptographic_exception is thrown.
		 */
		std::string get_pseudo_random_bytes(size_t cnt);

		/**
		 * \brief Mix some bytes into the PRNG state.
		 * \param buf The buffer that contains the bytes.
		 * \param buf_len The size of buf.
		 * \param entropy An estimate (lower bound) of how much randomness is contained in buf, measured in bytes. See RFC 1750 for details.
		 */
		void add(const void* buf, size_t buf_len, double entropy);

		/**
		 * \brief Mix some bytes into the PRNG state.
		 * \param buf The buffer that contains the bytes.
		 * \param buf_len The size of buf.
		 * \see add
		 *
		 * A call to seed(buf, buf_len) is equivalent to a call of add(buf, buf_len, buf_len).
		 */
		void seed(const void* buf, size_t buf_len);

		/**
		 * \brief Check if the PRNG was seeded enough to provide strong cryptographic material.
		 * \return true if the PRNG was seeded enough, false otherwise.
		 */
		bool status();

#ifdef WINDOWS

		/**
		 * \brief Mix some bytes into the PRNG from Windows events.
		 * \param imsg The imsg param.
		 * \param wparam The WPARAM.
		 * \param lparam The LPARAM.
		 * \return true if the PRNG was seeded enough, false otherwise.
		 */
		bool windows_event(UINT imsg, WPARAM wparam, LPARAM lparam);

		/**
		 * \brief Mix some bytes from the current screen state into the PRNG.
		 */
		void windows_screen();

#endif

		/**
		 * \brief Get a seed filename.
		 * \param buf The buffer to put the seed filename into.
		 * \param buf_len The size of buf.
		 * \return buf.
		 * \warning If buf is not long enough to hold the filename, a cryptographic_exception is thrown.
		 */
		const char* get_seed_filename(char* buf, size_t buf_len);

		/**
		 * \brief Load a seed file.
		 * \param file The file to load the seed from.
		 * \param cnt The number of bytes to read. If cnt is -1 (the default), the complete file is read.
		 * \return The number of bytes read.
		 */
		size_t load_seed_file(const std::string& file, ptrdiff_t cnt = -1);

		/**
		 * \brief Write a seed file from the current PRNG state.
		 * \param file The file to write the seed to.
		 * \return The number of bytes written.
		 */
		size_t write_seed_file(const std::string& file);

		/**
		 * \brief Query the entropy gathering daemon for 255 bytes.
		 * \param path The EGD socket path.
		 * \return The count of bytes read and added to the PRNG.
		 */
		size_t egd_query(const std::string& path);

		/**
		 * \brief Query the entropy gathering daemon for the specified ammount of bytes.
		 * \param path The EGD socket path.
		 * \param cnt The count of bytes to query.
		 * \return The count of bytes read and added to the PRNG.
		 */
		size_t egd_query(const std::string& path, size_t cnt);

		/**
		 * \brief Query the entropy gathering daemon for the specified ammount of bytes without adding the to the PRNG.
		 * \param path The EGD socket path.
		 * \param buf The buffer to put the random bytes into. If buf is NULL, the bytes are added to the PRNG state.
		 * \param cnt The count of bytes to query. buf must be long enough to hold cnt bytes.
		 * \return The count of bytes read.
		 */
		size_t egd_query(const std::string& path, void* buf, size_t cnt);

		/**
		 * \brief Clean up the PRNG.
		 */
		void cleanup();

		inline void set_randomization_engine(ENGINE* engine)
		{
			error::throw_error_if_not(RAND_set_rand_engine(engine) != 0);
		}

		inline void get_random_bytes(void* buf, size_t buf_len)
		{
			error::throw_error_if_not(RAND_bytes(static_cast<unsigned char*>(buf), static_cast<int>(buf_len)) == 1);
		}

		inline std::string get_random_bytes(size_t cnt)
		{
			std::string result(cnt, char());

			get_random_bytes(&result[0], result.size());

			return result;
		}

		inline bool get_pseudo_random_bytes(void* buf, size_t buf_len)
		{
			int result = RAND_pseudo_bytes(static_cast<unsigned char*>(buf), static_cast<int>(buf_len));

			error::throw_error_if(result < 0);

			return (result == 1);
		}

		inline std::string get_pseudo_random_bytes(size_t cnt)
		{
			std::string result(cnt, char());

			get_pseudo_random_bytes(&result[0], result.size());

			return result;
		}

		inline void add(const void* buf, size_t buf_len, double entropy)
		{
			RAND_add(buf, static_cast<int>(buf_len), entropy);
		}

		inline void seed(const void* buf, size_t buf_len)
		{
			RAND_seed(buf, static_cast<int>(buf_len));
		}

		inline bool status()
		{
			return (RAND_status() == 1);
		}

#ifdef WINDOWS

		inline bool windows_event(UINT imsg, WPARAM wparam, LPARAM lparam)
		{
			return (RAND_event(imsg, wparam, lparam) == 1);
		}

		inline void windows_screen()
		{
			RAND_screen();
		}

#endif

		inline const char* get_seed_filename(char* buf, size_t buf_len)
		{
			const char* result = RAND_file_name(buf, buf_len);

			error::throw_error_if_not(result);

			return result;
		}

		inline size_t load_seed_file(const std::string& file, ptrdiff_t cnt)
		{
			int result = RAND_load_file(file.c_str(), static_cast<long>(cnt));

			error::throw_error_if_not(result >= 0);

			return result;
		}

		inline size_t write_seed_file(const std::string& file)
		{
			int result = RAND_write_file(file.c_str());

			error::throw_error_if_not(result >= 0);

			return result;
		}

		inline size_t egd_query(const std::string& path)
		{
			int result = RAND_egd(path.c_str());

			error::throw_error_if_not(result >= 0);

			return result;
		}

		inline size_t egd_query(const std::string& path, size_t cnt)
		{
			int result = RAND_egd_bytes(path.c_str(), static_cast<int>(cnt));

			error::throw_error_if_not(result >= 0);

			return result;
		}

		inline size_t egd_query(const std::string& path, void* buf, size_t cnt)
		{
			int result = RAND_query_egd_bytes(path.c_str(), static_cast<unsigned char*>(buf), static_cast<int>(cnt));

			error::throw_error_if_not(result >= 0);

			return result;
		}

		inline void cleanup()
		{
			RAND_cleanup();
		}
	}
}

#endif /* CRYPTOPLUS_RANDOM_RANDOM_HPP */

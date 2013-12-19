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
 * \file memory_pool.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A memory pool class.
 */

#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

#include <vector>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/asio.hpp>

#include <stdint.h>

namespace fscp
{
	class memory_pool : public boost::noncopyable
	{
		public:

			/**
			 * @brief Create a memory pool instance.
			 * @param size The size of the internal memory.
			 */
			explicit memory_pool(size_t size = 65536 * 16);

			/**
			 * @brief Allocate a buffer.
			 * @param size The size of the buffer to allocate.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return The allocated buffer.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			boost::asio::buffer allocate_buffer(size_t size, bool use_heap_as_fallback = true)
			{
				return boost::asio::buffer(allocate(size, use_heap_as_fallback), size);
			}

			/**
			 * @brief Deallocate a buffer.
			 * @param buffer The buffer to deallocate. If buffer was not allocated by this allocator (or if it was deallocated already), the behavior is undefined.
			 * @tparam MutableBufferType The buffer type.
			 *
			 * This method is thread-safe.
			 */
			template <typename MutableBufferType>
			void deallocate_buffer(MutableBufferType buffer)
			{
				deallocate(boost::asio::buffer_cast<uint8_t*>(buffer));
			}

			/**
			 * @brief Allocate some memory.
			 * @param size The amount of bytes to allocate.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return A pointer to the allocated memory.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			uint8_t* allocate(size_t size, bool use_heap_as_fallback = true);

			/**
			 * @brief Deallocate a buffer.
			 * @param buffer The buffer to deallocate. If buffer was not allocated by this allocator (or if it was deallocated already), the behavior is undefined.
			 *
			 * This method is thread-safe.
			 */
			void deallocate(uint8_t* buffer);

		private:
			typedef std::vector<uint8_t> pool_type;
			typedef std::map<pool_type::iterator, size_t> pool_allocations_type;

			pool_type m_pool;
			pool_allocations_type m_pool_allocations;
			boost::mutex m_pool_mutex;
	};
}

#endif /* MEMORY_POOL_HPP */

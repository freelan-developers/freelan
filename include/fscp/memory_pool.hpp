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
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <stdint.h>

namespace fscp
{
	/**
	 * @brief A memory pool.
	 *
	 * Preallocates a pool of memory and handles allocation/deallocation with heap fallback mechanism.
	 *
	 * memory_pool is optimized for the allocation of buffers of similar sizes.
	 */
	class memory_pool : public boost::noncopyable
	{
		public:

			/**
			 * @brief A mutable buffer type.
			 */
			typedef boost::asio::mutable_buffers_1 buffer_type;

			/**
			 * @brief A scoped buffer type that gets deallocated upon destruction.
			 */
			class scoped_buffer_type : public boost::noncopyable
			{
				public:
					~scoped_buffer_type();

				private:

					scoped_buffer_type(memory_pool& mempool, buffer_type buffer) : m_memory_pool(mempool), m_buffer(buffer) {}

					memory_pool& m_memory_pool;
					buffer_type m_buffer;

					friend class memory_pool;

					friend inline buffer_type buffer(const scoped_buffer_type& _buffer)
					{
						return boost::asio::buffer(_buffer.m_buffer);
					}

					friend inline buffer_type buffer(const scoped_buffer_type& _buffer, size_t size)
					{
						return boost::asio::buffer(_buffer.m_buffer, size);
					}

					template <typename Type>
					friend inline Type buffer_cast(const scoped_buffer_type& _buffer)
					{
						return boost::asio::buffer_cast<Type>(buffer(_buffer));
					}

					friend inline size_t buffer_size(const scoped_buffer_type& _buffer)
					{
						return boost::asio::buffer_size(buffer(_buffer));
					}
			};

			/**
			 * @brief A shared buffer type.
			 */
			typedef boost::shared_ptr<scoped_buffer_type> shared_buffer_type;

			/**
			 * @brief The default block size.
			 */
			static const size_t DEFAULT_BLOCK_SIZE = 65536;

			/**
			 * @brief The default block count.
			 */
			static const size_t DEFAULT_BLOCK_COUNT = 16;

			/**
			 * @brief The default block count.
			 */
			static const size_t DEFAULT_SIZE = DEFAULT_BLOCK_SIZE * DEFAULT_BLOCK_COUNT;

			/**
			 * @brief Create a memory pool instance.
			 * @param size The size of the internal memory.
			 * @param block_size The default size for blocks.
			 */
			explicit memory_pool(size_t size = DEFAULT_SIZE, size_t block_size = DEFAULT_BLOCK_SIZE);

			/**
			 * @brief Allocate a shared buffer.
			 * @param size The size of the buffer to allocate. If 0, the default size is assumed.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return The allocated shared buffer.
			 *
			 * This method is thread-safe.
			 */
			shared_buffer_type allocate_shared_buffer(size_t size = 0, bool use_heap_as_fallback = true)
			{
				return shared_buffer_type(new scoped_buffer_type(*this, allocate_buffer(size, use_heap_as_fallback)));
			}

			/**
			 * @brief Allocate a buffer.
			 * @param size The size of the buffer to allocate. If 0, the default size is assumed.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return The allocated buffer.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			buffer_type allocate_buffer(size_t size = 0, bool use_heap_as_fallback = true)
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
			 * @param size The amount of bytes to allocate. If 0, the default size is assumed.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return A pointer to the allocated memory.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			uint8_t* allocate(size_t size = 0, bool use_heap_as_fallback = true);

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
			const size_t m_block_size;
			pool_allocations_type m_pool_allocations;
			boost::mutex m_pool_mutex;
	};

	inline memory_pool::buffer_type buffer(memory_pool::shared_buffer_type _buffer)
	{
		return buffer(*_buffer);
	}

	inline memory_pool::buffer_type buffer(memory_pool::shared_buffer_type _buffer, size_t size)
	{
		return buffer(*_buffer, size);
	}

	template <typename Type>
	inline Type buffer_cast(memory_pool::shared_buffer_type _buffer)
	{
		return buffer_cast<Type>(*_buffer);
	}

	inline size_t buffer_size(memory_pool::shared_buffer_type _buffer)
	{
		return buffer_size(*_buffer);
	}
}

#endif /* MEMORY_POOL_HPP */

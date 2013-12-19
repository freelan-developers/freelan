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

#include <boost/thread/lock_guard.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <set>
#include <new>
#include <cassert>

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
	template <size_t BlockSize = 65536, unsigned int BlockCount = 32>
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
					~scoped_buffer_type()
					{
						m_memory_pool.deallocate_buffer(m_buffer);
					}

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
			static const size_t block_size = BlockSize;

			/**
			 * @brief The default block count.
			 */
			static const unsigned int block_count = BlockCount;

			/**
			 * @brief Create a memory pool instance.
			 *
			 * The internal memory pool occupies exactly block_size * block_count bytes.
			 */
			memory_pool() :
				m_next_available_block(0),
				m_pool(BlockSize * BlockCount)
			{
			}

			/**
			 * @brief Allocate a shared buffer.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return The allocated shared buffer.
			 *
			 * This method is thread-safe.
			 */
			shared_buffer_type allocate_shared_buffer(bool use_heap_as_fallback = true)
			{
				return shared_buffer_type(new scoped_buffer_type(*this, allocate_buffer(use_heap_as_fallback)));
			}

			/**
			 * @brief Allocate a buffer.
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return The allocated buffer.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			buffer_type allocate_buffer(bool use_heap_as_fallback = true)
			{
				return boost::asio::buffer(allocate(use_heap_as_fallback), block_size);
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
			 * @param use_heap_as_fallback If true and no internal memory is available, an heap allocation is made instead. If false and no internal memory is available, a std::bad_alloc is thrown.
			 * @return A pointer to the allocated memory.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			uint8_t* allocate(bool use_heap_as_fallback = true)
			{
				boost::unique_lock<boost::mutex> guard(m_pool_mutex);

				unsigned int block = 0;

				if (m_pool_allocations.size() >= block_count)
				{
					block = block_count;
				}
				else if (m_next_available_block >= block_count)
				{
					for (pool_allocations_type::const_iterator allocation = m_pool_allocations.begin(); allocation != m_pool_allocations.end(); ++allocation)
					{
						if (block < *allocation)
						{
							break;
						}
						else
						{
							block = *allocation + 1;
						}
					}
				}
				else
				{
					block = m_next_available_block;
					m_next_available_block = block_count;
				}

				if (block >= block_count)
				{
					// We can release the lock sooner since we won't modify the allocation table.
					guard.unlock();

					// There is no more room for this allocation: trying heap allocation if permitted.
					if (use_heap_as_fallback)
					{
						return new uint8_t[block_size];
					}
					else
					{
						throw std::bad_alloc();
					}
				}
				else
				{
					m_pool_allocations.insert(block);

					return (&m_pool[0] + block_size * block);
				}
			}

			/**
			 * @brief Deallocate a buffer.
			 * @param buffer The buffer to deallocate. If buffer was not allocated by this allocator (or if it was deallocated already), the behavior is undefined.
			 *
			 * This method is thread-safe.
			 */
			void deallocate(uint8_t* buffer)
			{
				if ((buffer < &m_pool[0]) || (buffer >= &m_pool[0] + m_pool.size()))
				{
					// The buffer was heap allocated: we don't need to lock.
					delete[] buffer;
				}
				else
				{
					boost::lock_guard<boost::mutex> guard(m_pool_mutex);

					const unsigned int block = static_cast<unsigned int>(std::distance(&m_pool[0], buffer) / block_size);

					// This should never happen (or we have a programming error).
					assert(&m_pool[0] + block * block_size == buffer);

					m_pool_allocations.erase(block);
					m_next_available_block = block;
				}
			}

			friend inline memory_pool::buffer_type buffer(memory_pool::shared_buffer_type _buffer)
			{
				return buffer(*_buffer);
			}

			friend inline memory_pool::buffer_type buffer(memory_pool::shared_buffer_type _buffer, size_t size)
			{
				return buffer(*_buffer, size);
			}

			template <typename Type>
			friend inline Type buffer_cast(memory_pool::shared_buffer_type _buffer)
			{
				return buffer_cast<Type>(*_buffer);
			}

			friend inline size_t buffer_size(memory_pool::shared_buffer_type _buffer)
			{
				return buffer_size(*_buffer);
			}

		private:
			typedef std::vector<uint8_t> pool_type;
			typedef std::set<unsigned int> pool_allocations_type;

			unsigned int m_next_available_block;
			pool_type m_pool;
			pool_allocations_type m_pool_allocations;
			boost::mutex m_pool_mutex;
	};
}

#endif /* MEMORY_POOL_HPP */

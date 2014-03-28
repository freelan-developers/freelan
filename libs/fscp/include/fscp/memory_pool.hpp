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
#include <boost/iterator/counting_iterator.hpp>

#include <vector>
#include <set>
#include <new>
#include <cassert>

#include <stdint.h>

namespace fscp
{
	// We declare those functions since they are defined as class friend functions and so only available via ADL otherwise.
	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	class memory_pool;

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::buffer_type buffer(const typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::scoped_buffer_type&);

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::buffer_type buffer(const typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::scoped_buffer_type&, size_t);

	template <typename Type, size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	Type buffer_cast(const typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::scoped_buffer_type&);

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	size_t buffer_size(const typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::scoped_buffer_type&);

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::buffer_type buffer(typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::shared_buffer_type);

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::buffer_type buffer(typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::shared_buffer_type, size_t);

	template <typename Type, size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	Type buffer_cast(typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::shared_buffer_type);

	template <size_t BlockSize, unsigned int BlockCount, bool UseHeapFallback>
	size_t buffer_size(typename memory_pool<BlockSize, BlockCount, UseHeapFallback>::shared_buffer_type);

	/**
	 * @brief A memory pool.
	 *
	 * Preallocates a pool of memory and handles allocation/deallocation with heap fallback mechanism.
	 *
	 * memory_pool is optimized for blocks allocation.
	 *
	 * allocation has a constant cost; deallocation has a logarithmic cost.
	 */
	template <size_t BlockSize = 65536, unsigned int BlockCount = 32, bool UseHeapFallback = true>
	class memory_pool : public boost::noncopyable
	{
		public:

			/**
			 * @brief The default block size.
			 */
			static const size_t block_size = BlockSize;

			/**
			 * @brief The default block count.
			 */
			static const unsigned int block_count = BlockCount;

			/**
			 * @brief The heap fallback policy.
			 */
			static const bool use_heap_fallback = UseHeapFallback;

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

			friend inline typename memory_pool::buffer_type buffer(typename memory_pool::shared_buffer_type _buffer)
			{
				return buffer(*_buffer);
			}

			friend inline typename memory_pool::buffer_type buffer(typename memory_pool::shared_buffer_type _buffer, size_t size)
			{
				return buffer(*_buffer, size);
			}

			template <typename Type>
			friend inline Type buffer_cast(typename memory_pool::shared_buffer_type _buffer)
			{
				return buffer_cast<Type>(*_buffer);
			}

			friend inline size_t buffer_size(typename memory_pool::shared_buffer_type _buffer)
			{
				return buffer_size(*_buffer);
			}

			/**
			 * @brief Create a memory pool instance.
			 *
			 * The internal memory pool occupies exactly block_size * block_count bytes.
			 */
			memory_pool() :
				m_pool(BlockSize * BlockCount),
				m_available_blocks(boost::counting_iterator<unsigned int>(0), boost::counting_iterator<unsigned int>(BlockCount))
			{
			}

			/**
			 * @brief Allocate a shared buffer.
			 * @return The allocated shared buffer.
			 *
			 * This method is thread-safe.
			 */
			shared_buffer_type allocate_shared_buffer()
			{
				return shared_buffer_type(new scoped_buffer_type(*this, allocate_buffer()));
			}

			/**
			 * @brief Allocate a buffer.
			 * @return The allocated buffer.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			buffer_type allocate_buffer()
			{
				return boost::asio::buffer(allocate(), block_size);
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
			 * @return A pointer to the allocated memory.
			 *
			 * This method is thread-safe.
			 *
			 * The return buffer must be deallocated by passing it to deallocate() to avoid memory leaks.
			 */
			uint8_t* allocate()
			{
				boost::unique_lock<boost::mutex> guard(m_pool_mutex);

				if (m_available_blocks.empty())
				{
					// We can release the lock sooner since we won't modify the allocation table.
					guard.unlock();

					// There is no more room for this allocation: trying heap allocation if permitted.
					if (use_heap_fallback)
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
					const unsigned int block = *m_available_blocks.begin();

					m_available_blocks.erase(m_available_blocks.begin());

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

					m_available_blocks.insert(block);
				}
			}

		private:
			typedef std::vector<uint8_t> pool_type;
			typedef std::set<unsigned int> available_blocks_type;

			pool_type m_pool;
			available_blocks_type m_available_blocks;
			boost::mutex m_pool_mutex;
	};
}

#endif /* MEMORY_POOL_HPP */

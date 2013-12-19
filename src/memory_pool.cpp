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
 * \file memory_pool.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A memory pool class.
 */

#include "memory_pool.hpp"

#include <new>
#include <cassert>

#include <boost/thread/lock_guard.hpp>

namespace fscp
{
	memory_pool::scoped_buffer_type::~scoped_buffer_type()
	{
		m_memory_pool.deallocate_buffer(m_buffer);
	}

	memory_pool::memory_pool(size_t block_size, unsigned int block_count) :
		m_block_size(block_size),
		m_block_count(block_count),
		m_next_available_block(0),
		m_pool(m_block_size* m_block_count)
	{
	}

	uint8_t* memory_pool::allocate(bool use_heap_as_fallback)
	{
		const size_t size = m_block_size;

		boost::unique_lock<boost::mutex> guard(m_pool_mutex);

		unsigned int block = 0;

		if (m_pool_allocations.size() >= m_block_count)
		{
			block = m_block_count;
		}
		else if (m_next_available_block >= m_block_count)
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
			m_next_available_block = m_block_count;
		}

		if (block >= m_block_count)
		{
			// We can release the lock sooner since we won't modify the allocation table.
			guard.unlock();

			// There is no more room for this allocation: trying heap allocation if permitted.
			if (use_heap_as_fallback)
			{
				return new uint8_t[size];
			}
			else
			{
				throw std::bad_alloc();
			}
		}
		else
		{
			m_pool_allocations.insert(block);

			return (&m_pool[0] + m_block_size * block);
		}
	}

	void memory_pool::deallocate(uint8_t* buffer)
	{
		if ((buffer < &m_pool[0]) || (buffer >= &m_pool[0] + m_pool.size()))
		{
			// The buffer was heap allocated: we don't need to lock.
			delete[] buffer;
		}
		else
		{
			boost::lock_guard<boost::mutex> guard(m_pool_mutex);

			const unsigned int block = static_cast<unsigned int>(std::distance(&m_pool[0], buffer) / m_block_size);

			// This should never happen (or we have a programming error).
			assert(&m_pool[0] + block * m_block_size == buffer);

			m_pool_allocations.erase(block);
			m_next_available_block = block;
		}
	}
}

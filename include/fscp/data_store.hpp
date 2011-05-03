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
 * \file data_store.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief An data store class.
 */

#ifndef FSCP_DATA_STORE_HPP
#define FSCP_DATA_STORE_HPP

#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace fscp
{
	/**
	 * \brief An data store class.
	 */
	class data_store
	{
		public:

			/**
			 * \brief The data type.
			 */
			typedef unsigned char data_type;

			/**
			 * \brief The array data type.
			 */
			typedef std::vector<data_type> array_data_type;

			/**
			 * \brief The pointer data type.
			 */
			typedef boost::shared_ptr<array_data_type> pointer_data_type;

			/**
			 * \brief Push data to the data store.
			 * \param buf The data.
			 * \param buf_len The length of buf.
			 */
			void push(const void* buf, size_t buf_len);

			/**
			 * \brief Check if the data store is empty.
			 * \return true if the data store is empty.
			 */
			bool empty() const;

			/**
			 * \brief Get the front element, if any.
			 * \return A reference to the front element.
			 * \warning Calling this method on an empty data_store is undefined behavior.
			 * \see empty
			 */
			const array_data_type& front() const;

			/**
			 * \brief Pop the front element, if any.
			 * \warning Calling this method on an empty data_store is undefined behavior.
			 * \see empty
			 */
			void pop();

		private:

			std::queue<pointer_data_type> m_queue;
	};

	inline void data_store::push(const void* buf, size_t buf_len)
	{
		m_queue.push(boost::make_shared<array_data_type>(static_cast<const data_type*>(buf), static_cast<const data_type*>(buf) + buf_len));
	}

	inline bool data_store::empty() const
	{
		return m_queue.empty();
	}

	inline const data_store::array_data_type& data_store::front() const
	{
		return *m_queue.front();
	}

	inline void data_store::pop()
	{
		m_queue.pop();
	}
}

#endif /* FSCP_DATA_STORE_HPP */

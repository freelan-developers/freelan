/*
 * libfscp - A C++ library to establish peer-to-peer virtual private networks.
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
 * \file shared_buffer.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A shared buffer class.
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>

#include <cmath>
#include <stdint.h>

namespace fscp
{
	class SharedBuffer;

	boost::asio::mutable_buffers_1 buffer(const SharedBuffer&);
	boost::asio::mutable_buffers_1 buffer(const SharedBuffer&, size_t);
	template <typename Type> Type buffer_cast(const SharedBuffer&);
	size_t buffer_size(const SharedBuffer&);

	class SharedBuffer
	{
		public:
			SharedBuffer(size_t size) :
				m_size(size),
				m_data(new uint8_t[size])
			{}

			template <typename Handler>
			SharedBuffer(const SharedBuffer& buffer, Handler handler) :
				m_buffer(new SharedBuffer(buffer), [handler] (SharedBuffer* buf) {
					handler(*buf);

					delete buf;
				})
			{
			}

		private:
			size_t m_size;
			boost::shared_array<uint8_t> m_data;
			boost::shared_ptr<SharedBuffer> m_buffer;

			friend inline boost::asio::mutable_buffers_1 buffer(const SharedBuffer& buf)
			{
				if (buf.m_buffer)
				{
					return buffer(*(buf.m_buffer));
				}
				else
				{
					return boost::asio::buffer(buf.m_data.get(), buf.m_size);
				}
			}

			friend inline boost::asio::mutable_buffers_1 buffer(const SharedBuffer& buf, size_t size)
			{
				if (buf.m_buffer)
				{
					return buffer(*(buf.m_buffer), size);
				}
				else
				{
					return boost::asio::buffer(buf.m_data.get(), std::min(size, buf.m_size));
				}
			}

			template <typename Type>
			friend inline Type buffer_cast(const SharedBuffer& buf)
			{
				if (buf.m_buffer)
				{
					return buffer_cast<Type>(*(buf.m_buffer));
				}
				else
				{
					return boost::asio::buffer_cast<Type>(buffer(buf));
				}
			}

			friend inline size_t buffer_size(const SharedBuffer& buf)
			{
				if (buf.m_buffer)
				{
					return buffer_size((*buf.m_buffer));
				}
				else
				{
					return boost::asio::buffer_size(buffer(buf));
				}
			}
	};

	template <typename Handler>
	class SharedBufferHandler
	{
		public:

			typedef void result_type;

			SharedBufferHandler(SharedBuffer buf, Handler handler) :
				m_buffer(buf),
				m_handler(handler)
			{}

			result_type operator()()
			{
				m_handler();
			}

			template <typename Arg1>
			result_type operator()(Arg1 arg1)
			{
				m_handler(arg1);
			}

			template <typename Arg1, typename Arg2>
			result_type operator()(Arg1 arg1, Arg2 arg2)
			{
				m_handler(arg1, arg2);
			}

		private:

			SharedBuffer m_buffer;
			Handler m_handler;
	};

	template <typename Handler>
	inline SharedBufferHandler<Handler> make_shared_buffer_handler(SharedBuffer buf, Handler handler)
	{
		return SharedBufferHandler<Handler>(buf, handler);
	}
}

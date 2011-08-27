/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file dhcp_builder.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An dhcp frame builder class.
 */

#include "osi/dhcp_builder.hpp"

#include "osi/dhcp_helper.hpp"
#include "osi/dhcp_option_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		namespace
		{
			void memcpy(boost::asio::mutable_buffer dst, boost::asio::const_buffer src)
			{
				const size_t src_size = boost::asio::buffer_size(src);

				std::memcpy(boost::asio::buffer_cast<void*>(dst), boost::asio::buffer_cast<const void*>(src), src_size);
			}

			void memmove(boost::asio::mutable_buffer dst, boost::asio::const_buffer src)
			{
				const size_t src_size = boost::asio::buffer_size(src);

				std::memmove(boost::asio::buffer_cast<void*>(dst), boost::asio::buffer_cast<const void*>(src), src_size);
			}
		}

		void builder<dhcp_frame>::add_option(dhcp_option::dhcp_option_tag tag)
		{
			dhcp_option_helper<mutable_helper_tag> dhcp_option_helper(buffer() + m_options_offset);
			dhcp_option_helper.set_tag(tag);

			++m_options_offset;
		}

		void builder<dhcp_frame>::add_option(dhcp_option::dhcp_option_tag tag, boost::asio::const_buffer value)
		{
			dhcp_option_helper<mutable_helper_tag> dhcp_option_helper(buffer() + m_options_offset);

			const size_t value_size = boost::asio::buffer_size(value);

			if (value_size + 2 > boost::asio::buffer_size(buffer() + m_options_offset))
			{
				throw std::runtime_error("Insufficient buffer size");
			}

			dhcp_option_helper.set_tag(tag);
			dhcp_option_helper.set_length(value_size);
			memcpy(dhcp_option_helper.value(), value);

			m_options_offset += dhcp_option_helper.total_length();
		}

		void builder<dhcp_frame>::add_padding(size_t cnt)
		{
			if (cnt > boost::asio::buffer_size(buffer() + m_options_offset))
			{
				throw std::runtime_error("Insufficient buffer size");
			}

			std::memset(boost::asio::buffer_cast<void*>(buffer() + m_options_offset), static_cast<uint8_t>(dhcp_option::pad), cnt);
			m_options_offset += cnt;
		}

		size_t builder<dhcp_frame>::write() const
		{
			helper_type helper(buffer() + (boost::asio::buffer_size(buffer()) - m_options_offset - sizeof(frame_type)));

			helper.set_magic_cookie(DHCP_MAGIC_COOKIE);
			memmove(helper.options(), boost::asio::buffer(buffer(), m_options_offset));

			return boost::asio::buffer_size(helper.buffer());
		}
	}
}

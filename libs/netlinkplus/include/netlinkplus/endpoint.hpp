/*
 * libnetlinkplus - A portable netlink extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libnetlinkplus.
 *
 * libnetlinkplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libnetlinkplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libnetlinkplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A netlink endpoint class.
 */

#pragma once

#include <cstring>

#include <boost/asio.hpp>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace netlinkplus
{
	/**
	 * \brief A netlink endpoint class.
	 */
	template <typename ProtocolType>
	class netlink_endpoint
	{
		public:
			/**
			 * \brief The associated protocol type.
			 */
			typedef ProtocolType protocol_type;

			/**
			 * \brief The data type.
			 */
			typedef boost::asio::detail::socket_addr_type data_type;

			/**
			 * \brief Create a new endpoint.
			 * \param groups The multicast groups to be a member of.
			 * \param pid The pid associated to the endpoint.
			 */
			explicit netlink_endpoint(uint32_t groups = 0, uint32_t pid = 0) :
				m_sockaddr{AF_NETLINK, 0, pid, groups}
			{
			}

			/**
			 * \brief Get the associated protocol.
			 * \return An instance of the associated protocol.
			 */
			protocol_type protocol() const
			{
				return protocol_type();
			}

			/**
			 * \brief Get the data.
			 * \return The data.
			 */
			data_type* data()
			{
				return reinterpret_cast<sockaddr*>(&m_sockaddr);
			}

			/**
			 * \brief Get the data.
			 * \return The data.
			 */
			const data_type* data() const
			{
				return reinterpret_cast<const sockaddr*>(&m_sockaddr);
			}

			/**
			 * \brief Get the size of the endpoint.
			 * \return The size.
			 */
			size_t size() const
			{
				return sizeof(m_sockaddr);
			}

			/**
			 * \brief Get the capacity of the endpoint.
			 * \return The capacity.
			 */
			size_t capacity() const
			{
				return size();
			}

			friend bool operator==(const netlink_endpoint& lhs, const netlink_endpoint& rhs)
			{
				return (std::memcmp(&lhs.m_sockaddr, &rhs.m_sockaddr, sizeof(sockaddr_nl)) == 0);
			}

			friend bool operator!=(const netlink_endpoint& lhs, const netlink_endpoint& rhs)
			{
				return (std::memcmp(&lhs.m_sockaddr, &rhs.m_sockaddr, sizeof(sockaddr_nl)) != 0);
			}

			friend bool operator<(const netlink_endpoint& lhs, const netlink_endpoint& rhs)
			{
				return (std::memcmp(&lhs.m_sockaddr, &rhs.m_sockaddr, sizeof(sockaddr_nl)) < 0);
			}

		private:

			sockaddr_nl m_sockaddr;
	};
}

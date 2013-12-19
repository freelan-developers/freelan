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
 * \file server.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#include "server2.hpp"

using namespace boost;

namespace fscp
{
	namespace
	{
		server2::ep_type to_socket_format(const boost::asio::ip::udp::socket& socket, const server2::ep_type& ep)
		{
#ifdef WINDOWS
			if (socket.local_endpoint().address().is_v6() && ep.address().is_v4())
			{
				return server2::ep_type(boost::asio::ip::address_v6::v4_mapped(ep.address().to_v4()), ep.port());
			}
			else
			{
				return ep;
			}
#else
			static_cast<void>(socket);

			return ep;
#endif
		}
	}

	server2::server2(boost::asio::io_service& io_service, const identity_store& identity) :
		m_socket(io_service),
		m_socket_strand(io_service),
		m_identity_store(identity)
	{
	}

	void server2::open(const ep_type& listen_endpoint)
	{
		m_socket.open(listen_endpoint.protocol());

		if (listen_endpoint.address().is_v6())
		{
			// We accept both IPv4 and IPv6 addresses
			m_socket.set_option(boost::asio::ip::v6_only(false));
		}

		m_socket.bind(listen_endpoint);
	}

	void server2::close()
	{
		m_socket.close();
	}

	void server2::do_greet(const ep_type&, resultless_handler_type)
	{
		//TODO: Implement
	}

	template <typename MutableBufferSequence, typename ReadHandler>
	void server2::do_async_receive_from(const MutableBufferSequence& buffers, ep_type& sender, ReadHandler handler)
	{
		m_socket.async_receive_from(buffers, sender, 0, handler);
	}

	template <typename ConstBufferSequence, typename WriteHandler>
	void server2::do_async_send_to(const ConstBufferSequence& buffers, const ep_type& target, WriteHandler handler)
	{
		m_socket.async_send_to(buffers, to_socket_format(m_socket, target), 0, handler);
	}
}

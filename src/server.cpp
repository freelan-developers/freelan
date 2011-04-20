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

#include "server.hpp"

#include "message.hpp"

#include <boost/bind.hpp>

#include <iostream>

using namespace boost;
using asio::ip::udp;

namespace fscp
{
	server::server(asio::io_service& io_service, const asio::ip::udp::endpoint& endpoint) :
		m_socket(io_service, endpoint)
	{
		async_receive();
	}
	
	void server::async_receive()
	{
		m_socket.async_receive_from(asio::buffer(m_recv_buffer), m_sender_endpoint, bind(&server::handle_receive_from, this, asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void server::handle_receive_from(const system::error_code& error, size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			message msg(m_recv_buffer.data(), bytes_recvd);

			handle_message(msg, m_sender_endpoint);

			async_receive();
		}
	}
	
	void server::handle_message(const message& msg, const boost::asio::ip::udp::endpoint& sender)
	{
		std::cout << "Received message " << msg.type() << " from " << sender << std::endl;
	}
}

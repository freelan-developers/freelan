/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file endpoint_switch_port.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An endpoint switch port class.
 */

#ifndef ENDPOINT_SWITCH_PORT_HPP
#define ENDPOINT_SWITCH_PORT_HPP

#include "switch_port.hpp"

#include <fscp/server.hpp>

namespace freelan
{
	/**
	 * \brief A switch port bound to an endpoint.
	 */
	class endpoint_switch_port : public switch_port
	{
		public:

			/**
			 * \brief Create a switch port bound to a specified fscp server and an
			 * endpoint.
			 * \param server
			 * \param endpoint
			 */
			endpoint_switch_port(fscp::server& server, fscp::server::ep_type endpoint);

		protected:

			/**
			 * \brief Send data trough the port.
			 * \param data The data to send trough the port.
			 */
			void write(boost::asio::const_buffer data);

		private:

			fscp::server& m_server;
			fscp::server::ep_type m_endpoint;
	};

	inline endpoint_switch_port::endpoint_switch_port(fscp::server& server, fscp::server::ep_type endpoint) :
		m_server(server),
		m_endpoint(endpoint)
	{
	}

	inline void endpoint_switch_port::write(boost::asio::const_buffer data)
	{
		m_server.async_send_data(m_endpoint, data);
	}
}

#endif /* ENDPOINT_SWITCH_PORT_HPP */


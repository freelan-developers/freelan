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
 * \file endpoint_router_port.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An endpoint router port class.
 */

#ifndef ENDPOINT_ROUTER_PORT_HPP
#define ENDPOINT_ROUTER_PORT_HPP

#include "router_port.hpp"

#include <fscp/server.hpp>

namespace freelan
{
	/**
	 * \brief A router port bound to an endpoint.
	 */
	class endpoint_router_port : public router_port
	{
		public:

			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The send data callback.
			 * \param target The target host.
			 * \param data The data to send.
			 */
			typedef boost::function<void (const ep_type& target, boost::asio::const_buffer data)> send_data_callback;

			/**
			 * \brief Create a router port bound to a specified endpoint.
			 * \param endpoint The associated endpoint.
			 * \param local_routes The local routes for the endpoint.
			 * \param callback The send data callback to use.
			 */
			endpoint_router_port(fscp::server::ep_type endpoint, const routes_type& local_routes, send_data_callback callback);

		protected:

			/**
			 * \brief Send data trough the port.
			 * \param data The data to send trough the port.
			 */
			void write(boost::asio::const_buffer data);

			/**
			 * \brief Check if the instance is equal to another.
			 * \param other The other instance to test for equality.
			 * \return true if the two instances are equal. Two instances of different subtypes are never equal.
			 */
			bool equals(const router_port& other) const;

			/**
			 * \brief Output the name of the router port to an output stream.
			 * \param os The output stream.
			 * \return os.
			 */
			std::ostream& output(std::ostream& os) const;

		private:

			fscp::server::ep_type m_endpoint;
			send_data_callback m_send_data_callback;

			friend bool operator==(const endpoint_router_port&, const endpoint_router_port&);
	};

	/**
	 * \brief Test two endpoint_router_port for equality.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if lhs and rhs have the exact same attributes.
	 */
	bool operator==(const endpoint_router_port& lhs, const endpoint_router_port& rhs);

	inline endpoint_router_port::endpoint_router_port(fscp::server::ep_type ep, const routes_type& _local_routes, send_data_callback callback) :
		router_port(_local_routes),
		m_endpoint(ep),
		m_send_data_callback(callback)
	{
	}

	inline void endpoint_router_port::write(boost::asio::const_buffer data)
	{
		if (m_send_data_callback)
		{
			m_send_data_callback(m_endpoint, data);
		}
	}

	inline std::ostream& endpoint_router_port::output(std::ostream& os) const
	{
		return os << "Endpoint (" << m_endpoint << ")";
	}

	inline bool operator==(const endpoint_router_port& lhs, const endpoint_router_port& rhs)
	{
		return (lhs.m_endpoint == rhs.m_endpoint);
	}
}

#endif /* ENDPOINT_ROUTER_PORT_HPP */

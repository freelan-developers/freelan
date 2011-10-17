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
 * \file switch.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A switch class.
 */

#ifndef SWITCH_HPP
#define SWITCH_HPP

#include <algorithm>
#include <map>

#include <boost/asio.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>

#include "switch_port.hpp"
#include "configuration.hpp"

namespace freelan
{
	/**
	 * \brief A class that represents a switch.
	 */
	class switch_
	{
		public:

			/**
			 * \brief The port type.
			 */
			typedef switch_port port_type;

			/**
			 * \brief The port list type.
			 */
			typedef boost::ptr_vector<port_type> port_list_type;

			/**
			 * \brief The port iterator type.
			 */
			typedef port_list_type::iterator port_iterator_type;

			/**
			 * \brief Create a new switch.
			 * \param routing_method The routing method to use.
			 */
			switch_(configuration::routing_method_type routing_method);

			/**
			 * \brief Add a switch port.
			 * \param port The port to add. Cannot be null.
			 * \return An iterator to the port.
			 *
			 * The switch takes ownership of the specified port.
			 */
			port_iterator_type add_port(port_type* port);

			/**
			 * \brief Remove a port.
			 * \param it An iterator to the port to remove. Must be a valid iterator.
			 * \return The next port in the list.
			 */
			port_iterator_type remove_port(port_iterator_type it);

			/**
			 * \brief Receive data trough the specified port.
			 * \param it An iterator to the port onto which the data was received.
			 * \param data The data.
			 */
			void receive_data(port_iterator_type it, boost::asio::const_buffer data);

			/**
			 * \brief Get an iterator to the first port.
			 * \return An iterator to the first port.
			 */
			port_iterator_type begin();

			/**
			 * \brief Get an iterator past the last port.
			 * \return An iterator past the last port.
			 */
			port_iterator_type end();

		private:

			void send_data_from(port_iterator_type it, boost::asio::const_buffer data);
			void send_data_to(port_iterator_type it, boost::asio::const_buffer data);

			configuration::routing_method_type m_routing_method;
			port_list_type m_ports;

			typedef boost::array<uint8_t, 6> ethernet_address_type;
			typedef std::map<ethernet_address_type, port_iterator_type> ethernet_address_map_type;

			static ethernet_address_type to_ethernet_address(boost::asio::const_buffer);

			ethernet_address_map_type m_ethernet_address_map;
	};
	
	inline switch_::switch_(configuration::routing_method_type routing_method) :
		m_routing_method(routing_method)
	{
	}

	inline switch_::port_iterator_type switch_::add_port(port_type* port)
	{
		return m_ports.insert(m_ports.end(), port);
	}

	inline switch_::port_iterator_type switch_::remove_port(port_iterator_type it)
	{
		return m_ports.erase(it);
	}

	inline switch_::port_iterator_type switch_::begin()
	{
		return m_ports.begin();
	}

	inline switch_::port_iterator_type switch_::end()
	{
		return m_ports.end();
	}
}

#endif /* SWITCH_HPP */


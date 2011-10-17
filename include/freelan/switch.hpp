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
#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "switch_port.hpp"
#include "configuration.hpp"
#include "logger.hpp"

namespace freelan
{
	/**
	 * \brief A class that represents a switch.
	 */
	class switch_
	{
		public:

			/**
			 * \brief The base port type.
			 */
			typedef switch_port base_port_type;

			/**
			 * \brief The port type.
			 */
			typedef boost::shared_ptr<base_port_type> port_type;

			/**
			 * \brief The port list type.
			 */
			typedef std::vector<port_type> port_list_type;

			/**
			 * \brief Create a new switch.
			 * \param configuration The switch configuration.
			 * \param _logger The logger to use.
			 */
			switch_(const switch_configuration& configuration, const freelan::logger& _logger);

			/**
			 * \brief Register a switch port.
			 * \param port The port to register. Cannot be null.
			 */
			void register_port(port_type port);

			/**
			 * \brief Unregister a port.
			 * \param port The port to unregister. Cannot be null.
			 *
			 * If the port was not registered, nothing is done.
			 */
			void unregister_port(port_type port);

			/**
			 * \brief Check if the specified port is registered.
			 * \param port The port to check.
			 * \return true if the port is registered, false otherwise.
			 */
			bool is_registered(port_type port) const;

			/**
			 * \brief Receive data trough the specified port.
			 * \param port The port from which the data comes. Cannot be null.
			 * \param data The data.
			 */
			void receive_data(port_type port, boost::asio::const_buffer data);

		private:

			void send_data_from(port_type, boost::asio::const_buffer);
			void send_data_to(port_type, boost::asio::const_buffer);

			switch_configuration m_configuration;
			freelan::logger m_logger;

			port_list_type m_ports;

			typedef boost::array<uint8_t, 6> ethernet_address_type;
			typedef boost::weak_ptr<base_port_type> weak_port_type;
			typedef std::map<ethernet_address_type, weak_port_type> ethernet_address_map_type;

			static ethernet_address_type to_ethernet_address(boost::asio::const_buffer);
			static bool is_multicast_address(const ethernet_address_type& address);

			ethernet_address_map_type m_ethernet_address_map;
	};
	
	inline switch_::switch_(const switch_configuration& configuration, const freelan::logger& _logger) :
		m_configuration(configuration),
		m_logger(_logger)
	{
	}

	inline void switch_::register_port(port_type port)
	{
		m_ports.push_back(port);
	}

	inline void switch_::unregister_port(port_type port)
	{
		m_ports.erase(std::remove(m_ports.begin(), m_ports.end(), port), m_ports.end());
	}

	inline bool switch_::is_registered(port_type port) const
	{
		return (std::find(m_ports.begin(), m_ports.end(), port) != m_ports.end());
	}
}

#endif /* SWITCH_HPP */


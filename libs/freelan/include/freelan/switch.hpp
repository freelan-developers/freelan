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
#include <set>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "configuration.hpp"
#include "port_index.hpp"

namespace freelan
{
	/**
	 * \brief A class that represents a switch.
	 */
	class switch_
	{
		public:

			/**
			 * \brief The default maximum entry count.
			 */
			static const unsigned int MAX_ENTRIES_DEFAULT;

			/**
			 * \brief The port group type.
			 */
			typedef unsigned int port_group_type;

			/**
			 * \brief The multi write result type.
			 */
			typedef std::map<port_index_type, boost::system::error_code> multi_write_result_type;

			/**
			 * \brief The write handler type.
			 */
			typedef boost::function<void (const multi_write_result_type&)> multi_write_handler_type;

			/**
			 * \brief A switch port type.
			 */
			class port_type
			{
				public:

					/**
					 * \brief The write handler type.
					 */
					typedef boost::function<void (boost::system::error_code)> write_handler_type;

					/**
					 * \brief A write function type.
					 */
					typedef boost::function<void (boost::asio::const_buffer data, write_handler_type handler)> write_function_type;

					/**
					 * \brief Create a new default port.
					 */
					port_type() :
						m_write_function(),
						m_group()
					{}

					/**
					 * \brief Create a new port.
					 * \param write_function The write function to use.
					 * \param _group The group this port belongs to.
					 */
					port_type(write_function_type write_function, port_group_type _group) :
						m_write_function(write_function),
						m_group(_group)
					{}

					/**
					 * \brief Write data to the port.
					 * \param data The data to write.
					 * \param handler The handler to call when the write is complete.
					 */
					void async_write(boost::asio::const_buffer data, write_handler_type handler)
					{
						m_write_function(data, handler);
					}

					port_group_type group() const
					{
						return m_group;
					}

				private:

					write_function_type m_write_function;
					port_group_type m_group;
			};

			/**
			 * \brief The port list type.
			 */
			typedef std::map<port_index_type, port_type> port_list_type;

			/**
			 * \brief Create a new switch.
			 * \param configuration The switch configuration.
			 * \param max_entries maximum entries allowed.
			 */
			switch_(const switch_configuration& configuration, const unsigned int max_entries = MAX_ENTRIES_DEFAULT) :
				m_configuration(configuration),
				m_max_entries(max_entries)
			{}

			/**
			 * \brief Register a switch port.
			 * \param index The index of the port.
			 * \param port The port to register. Cannot be null.
			 */
			void register_port(port_index_type index, port_type port)
			{
				m_ports[index] = port;
			}

			/**
			 * \brief Unregister a port.
			 * \param index The port to unregister. Cannot be null.
			 *
			 * If the port was not registered, nothing is done.
			 */
			void unregister_port(port_index_type index)
			{
				m_ports.erase(index);
			}

			/**
			 * \brief Check if the specified port is registered.
			 * \param index The port to check.
			 * \return true if the port is registered, false otherwise.
			 */
			bool is_registered(port_index_type index) const
			{
				return (m_ports.find(index) != m_ports.end());
			}

			/**
			 * \brief Receive data trough the specified port.
			 * \param index The port from which the data comes.
			 * \param data The data to write.
			 * \param handler The handler to call when the write is complete.
			 */
			void async_write(port_index_type index, boost::asio::const_buffer data, multi_write_handler_type handler);

		private:

			std::set<port_index_type> get_targets_for(port_index_type, boost::asio::const_buffer);
			std::set<port_index_type> get_targets_for(port_list_type::const_iterator);

			switch_configuration m_configuration;
			unsigned int m_max_entries;

			port_list_type m_ports;

			typedef boost::array<uint8_t, 6> ethernet_address_type;
			typedef std::map<ethernet_address_type, port_index_type> ethernet_address_map_type;

			static ethernet_address_type to_ethernet_address(boost::asio::const_buffer);
			static bool is_multicast_address(const ethernet_address_type&);

			ethernet_address_map_type m_ethernet_address_map;
	};
}

#endif /* SWITCH_HPP */


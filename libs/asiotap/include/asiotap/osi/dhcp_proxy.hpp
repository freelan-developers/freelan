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
 * \file dhcp_proxy.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An DHCP proxy class.
 */

#ifndef ASIOTAP_OSI_DHCP_PROXY_HPP
#define ASIOTAP_OSI_DHCP_PROXY_HPP

#include "proxy.hpp"

#include "ethernet_filter.hpp"
#include "ipv4_filter.hpp"
#include "udp_filter.hpp"
#include "bootp_filter.hpp"
#include "dhcp_filter.hpp"
#include "complex_filter.hpp"

#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <map>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An DHCP proxy class.
		 */
		template <>
		class proxy<dhcp_frame> : public _base_proxy<dhcp_frame>
		{
			public:

				/**
				 * \brief The default lease time.
				 */
				static const boost::posix_time::time_duration DEFAULT_LEASE_TIME;

				/**
				 * \brief The Ethernet address type.
				 */
				typedef boost::array<uint8_t, ETHERNET_ADDRESS_SIZE> ethernet_address_type;

				/**
				 * \brief An IPv4 address netmask type.
				 */
				struct ipv4_address_netmask_type
				{
					boost::asio::ip::address_v4 address; /**< The IPv4 address. */
					unsigned int prefix_length; /**< The IPv4 netmask length. */
				};

				/**
				 * \brief The entry type.
				 */
				typedef std::pair<ethernet_address_type, ipv4_address_netmask_type> entry_type;

				/**
				 * \brief Create an DHCP proxy.
				 */
				proxy() :
					m_lease_time(DEFAULT_LEASE_TIME)
				{
				}

				/**
				 * \brief Set the hardware address.
				 * \param hardware_address The hardware address.
				 */
				void set_hardware_address(const ethernet_address_type& hardware_address);

				/**
				 * \brief Set the software address.
				 * \param software_address The software address.
				 */
				void set_software_address(const boost::asio::ip::address_v4& software_address);

				/**
				 * \brief Set the lease time.
				 * \param lease_time The lease time.
				 */
				void set_lease_time(boost::posix_time::time_duration lease_time);

				/**
				 * \brief Add a proxy entry.
				 * \param entry The entry to add.
				 * \return If an entry for the specified logical address already exists, nothing is done and the call returns false. Otherwise, the call returns true.
				 */
				bool add_entry(const entry_type& entry);

				/**
				 * \brief Add a proxy entry.
				 * \param hardware_address The hardware address.
				 * \param logical_address The logical address.
				 * \param logical_prefix_length The prefix length.
				 * \return If an entry for the specified logical address already exists, nothing is done and the call returns false. Otherwise, the call returns true.
				 */
				bool add_entry(const ethernet_address_type& hardware_address, const boost::asio::ip::address_v4& logical_address, unsigned int logical_prefix_length);

				/**
				 * \brief Delete a proxy entry.
				 * \param hardware_address The hardware address.
				 * \return If an entry was deleted, true is returned. Otherwise, the call returns false.
				 */
				bool remove_entry(const ethernet_address_type& hardware_address);

				/**
				 * \brief Process a frame.
				 * \param ethernet_helper The ethernet layer.
				 * \param ipv4_helper The IPv4 layer.
				 * \param udp_helper The UDP layer.
				 * \param bootp_helper The BOOTP layer.
				 * \param dhcp_helper The DHCP layer.
				 * \param response_buffer The buffer to write the response to.
				 * \return The buffer that contains the answer, if there is one.
				 */
				boost::optional<boost::asio::const_buffer> process_frame(const_helper<ethernet_frame> ethernet_helper, const_helper<ipv4_frame> ipv4_helper, const_helper<udp_frame> udp_helper, const_helper<bootp_frame> bootp_helper, const_helper<dhcp_frame> dhcp_helper, boost::asio::mutable_buffer response_buffer) const;

			private:

				typedef std::map<ethernet_address_type, ipv4_address_netmask_type> entry_map_type;

				ethernet_address_type m_hardware_address;
				boost::asio::ip::address_v4 m_software_address;
				boost::posix_time::time_duration m_lease_time;
				entry_map_type m_entry_map;
		};

		inline void proxy<dhcp_frame>::set_hardware_address(const ethernet_address_type& hardware_address)
		{
			m_hardware_address = hardware_address;
		}

		inline void proxy<dhcp_frame>::set_software_address(const boost::asio::ip::address_v4& software_address)
		{
			m_software_address = software_address;
		}

		inline void proxy<dhcp_frame>::set_lease_time(boost::posix_time::time_duration lease_time)
		{
			m_lease_time = lease_time;
		}

		inline bool proxy<dhcp_frame>::add_entry(const entry_type& entry)
		{
			return m_entry_map.insert(entry).second;
		}

		inline bool proxy<dhcp_frame>::add_entry(const ethernet_address_type& hardware_address, const boost::asio::ip::address_v4& logical_address, unsigned int logical_prefix_length)
		{
			ipv4_address_netmask_type ipv4_address_netmask = { logical_address, logical_prefix_length };

			return add_entry(std::make_pair(hardware_address, ipv4_address_netmask));
		}

		inline bool proxy<dhcp_frame>::remove_entry(const ethernet_address_type& hardware_address)
		{
			return (m_entry_map.erase(hardware_address) > 0);
		}
	}
}

#endif /* ASIOTAP_DHCP_PROXY_HPP */


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
 * \file arp_proxy.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ARP proxy class.
 */

#ifndef ASIOTAP_OSI_ARP_PROXY_HPP
#define ASIOTAP_OSI_ARP_PROXY_HPP

#include "proxy.hpp"

#include "ethernet_filter.hpp"
#include "arp_filter.hpp"
#include "complex_filter.hpp"
#include "ethernet_address.hpp"

#include <boost/optional.hpp>

#include <map>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An ARP proxy class.
		 */
		template <>
		class proxy<arp_frame> : public _base_proxy<arp_frame>
		{
			public:

				/**
				 * \brief The Ethernet address type.
				 */
				typedef ethernet_address ethernet_address_type;

				/**
				 * \brief The entry type.
				 */
				typedef std::pair<boost::asio::ip::address_v4, ethernet_address_type> entry_type;

				/**
				 * \brief The ARP request callback type.
				 */
				typedef boost::function<bool (const boost::asio::ip::address_v4&, ethernet_address_type&)> arp_request_callback_type;

				/**
				 * \brief Create an ARP proxy.
				 */
				proxy() :
					m_arp_request_callback(0)
				{
				}

				/**
				 * \brief Add a proxy entry.
				 * \param entry The entry to add.
				 * \return If an entry for the specified logical address already exists, nothing is done and the call returns false. Otherwise, the call returns true.
				 */
				bool add_entry(const entry_type& entry);

				/**
				 * \brief Add a proxy entry.
				 * \param logical_address The logical address.
				 * \param hardware_address The hardware address.
				 * \return If an entry for the specified logical address already exists, nothing is done and the call returns false. Otherwise, the call returns true.
				 */
				bool add_entry(const boost::asio::ip::address_v4& logical_address, const ethernet_address_type& hardware_address);

				/**
				 * \brief Delete a proxy entry.
				 * \param logical_address The logical address.
				 * \return If an entry was deleted, true is returned. Otherwise, the call returns false.
				 */
				bool remove_entry(const boost::asio::ip::address_v4& logical_address);

				/**
				 * \brief Set the callback function when a ARP request is received.
				 * \param callback The callback function.
				 */
				void set_arp_request_callback(arp_request_callback_type callback);

				/**
				 * \brief Process a frame.
				 * \param ethernet_helper The ethernet layer.
				 * \param arp_helper The ARP layer.
				 * \param response_buffer The buffer to write the response to.
				 * \return The buffer that contains the answer, if there is one.
				 */
				boost::optional<boost::asio::const_buffer> process_frame(const_helper<ethernet_frame> ethernet_helper, const_helper<arp_frame> arp_helper, boost::asio::mutable_buffer response_buffer) const;

			private:

				typedef std::map<boost::asio::ip::address_v4, ethernet_address_type> entry_map_type;

				entry_map_type m_entry_map;
				arp_request_callback_type m_arp_request_callback;
		};

		inline bool proxy<arp_frame>::add_entry(const entry_type& entry)
		{
			return m_entry_map.insert(entry).second;
		}

		inline bool proxy<arp_frame>::add_entry(const boost::asio::ip::address_v4& logical_address, const ethernet_address_type& hardware_address)
		{
			return add_entry(std::make_pair(logical_address, hardware_address));
		}

		inline bool proxy<arp_frame>::remove_entry(const boost::asio::ip::address_v4& logical_address)
		{
			return (m_entry_map.erase(logical_address) > 0);
		}

		inline void proxy<arp_frame>::set_arp_request_callback(arp_request_callback_type callback)
		{
			m_arp_request_callback = callback;
		}
	}
}

#endif /* ASIOTAP_ARP_PROXY_HPP */


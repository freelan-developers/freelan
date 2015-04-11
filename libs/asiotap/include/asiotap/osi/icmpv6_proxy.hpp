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
 * \file icmpv6_proxy.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ICMPv6 proxy class.
 */

#pragma once

#include "proxy.hpp"

#include "ipv6_filter.hpp"
#include "icmpv6_filter.hpp"
#include "ethernet_address.hpp"

#include <boost/optional.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An ARP proxy class.
		 */
		template <>
		class proxy<icmpv6_frame> : public _base_proxy<icmpv6_frame>
		{
			public:

				/**
				 * \brief The Ethernet address type.
				 */
				typedef ethernet_address ethernet_address_type;

				/**
				 * \brief The neighbor sollicitation request callback type.
				 */
				typedef boost::function<bool (const boost::asio::ip::address_v6&, ethernet_address_type&)> neighbor_solicitation_callback_type;

				/**
				 * \brief Create an ARP proxy.
				 */
				proxy() :
					m_neighbor_solicitation_callback()
				{
				}

				/**
				 * \brief Set the callback function when a neighbor sollicitation is received.
				 * \param callback The callback function.
				 */
				void set_neighbor_solicitation_callback(neighbor_solicitation_callback_type callback)
				{
					m_neighbor_solicitation_callback = callback;
				}

				/**
				 * \brief Process a frame.
				 * \param ipv6_helper The IPv6 layer.
				 * \param icmpv6_helper The ICMPv6 layer.
				 * \param response_buffer The buffer to write the response to.
				 * \return The buffer that contains the answer, if there is one.
				 */
				boost::optional<boost::asio::const_buffer> process_frame(const_helper<ipv6_frame> ipv6_helper, const_helper<icmpv6_frame> icmpv6_helper,  boost::asio::mutable_buffer response_buffer) const;

			private:

				neighbor_solicitation_callback_type m_neighbor_solicitation_callback;
		};
	}
}

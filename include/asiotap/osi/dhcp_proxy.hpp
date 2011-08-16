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
				 * \brief The filter type.
				 */
				typedef complex_filter<frame_type, bootp_frame, udp_frame, ipv4_frame, ethernet_frame>::type filter_type;

				/**
				 * \brief Create an DHCP proxy.
				 * \param response_buffer The buffer to write the responses into.
				 * \param on_data_available The callback function to call when data is available for writing.
				 * \param dhcp_filter The DHCP filter to use.
				 */
				proxy(boost::asio::mutable_buffer response_buffer, data_available_callback_type on_data_available, filter_type& dhcp_filter);

			private:

				void on_frame(const_helper<frame_type>);
				void do_handle_frame(const_helper<ethernet_frame>, const_helper<ipv4_frame>, const_helper<udp_frame>, const_helper<bootp_frame>, const_helper<dhcp_frame>);

				filter_type& m_dhcp_filter;
		};

		inline proxy<dhcp_frame>::proxy(boost::asio::mutable_buffer _response_buffer, data_available_callback_type on_data_available, filter_type& dhcp_filter) :
			_base_proxy<dhcp_frame>(_response_buffer, on_data_available),
			m_dhcp_filter(dhcp_filter)
		{
			m_dhcp_filter.add_handler(boost::bind(&proxy<dhcp_frame>::on_frame, this, _1));
		}

		inline void proxy<dhcp_frame>::on_frame(const_helper<frame_type> helper)
		{
			do_handle_frame(
			    *m_dhcp_filter.parent().parent().parent().parent().get_last_helper(),
			    *m_dhcp_filter.parent().parent().parent().get_last_helper(),
			    *m_dhcp_filter.parent().parent().get_last_helper(),
			    *m_dhcp_filter.parent().get_last_helper(),
			    helper
			);
		}
	}
}

#endif /* ASIOTAP_DHCP_PROXY_HPP */


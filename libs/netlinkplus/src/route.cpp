/*
 * libnetlinkplus - A portable netlink extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libnetlinkplus.
 *
 * libnetlinkplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libnetlinkplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libnetlinkplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file route.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief netlink route classes.
 */

#include "route.hpp"
#include "route_message.hpp"

#include <net/if.h>
#include <errno.h>

namespace netlinkplus
{
	namespace
	{
		template <typename AddressType, typename AttributesType>
		route_entry get_route_entry(const AttributesType& attributes)
		{
			route_entry result;

			for (auto&& attribute : attributes)
			{
				const auto data = boost::asio::buffer(RTA_DATA(&attribute), RTA_PAYLOAD(&attribute));

				switch (attribute.rta_type)
				{
					case RTA_DST:
						{
							typename AddressType::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw std::runtime_error("Invalid destination value");
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.destination = AddressType(bytes);

							break;
						}
					case RTA_SRC:
						{
							typename AddressType::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw std::runtime_error("Invalid source value");
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.source = AddressType(bytes);

							break;
						}
					case RTA_IIF:
						{
							if (buffer_size(data) != sizeof(unsigned int))
							{
								throw std::runtime_error("Invalid interface value");
							}

							result.input_interface = interface_entry(*boost::asio::buffer_cast<unsigned int*>(data));

							break;
						}
					case RTA_OIF:
						{
							if (buffer_size(data) != sizeof(unsigned int))
							{
								throw std::runtime_error("Invalid interface value");
							}

							result.output_interface = interface_entry(*boost::asio::buffer_cast<unsigned int*>(data));

							break;
						}
					case RTA_GATEWAY:
						{
							typename AddressType::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw std::runtime_error("Invalid gateway value");
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.gateway = boost::asio::ip::address(AddressType(bytes));

							break;
						}
					case RTA_PRIORITY:
						{
							if (buffer_size(data) != sizeof(result.priority))
							{
								throw std::runtime_error("Invalid priority value");
							}

							result.priority = *boost::asio::buffer_cast<unsigned int*>(data);

							break;
						}
					case RTA_METRICS:
						{
							if (buffer_size(data) != sizeof(result.priority))
							{
								throw std::runtime_error("Invalid metric value");
							}

							result.metric = *boost::asio::buffer_cast<unsigned int*>(data);

							break;
						}
				}
			}

			return result;
		}
	}

	std::string interface_entry::name() const
	{
		if (m_name_cache.empty())
		{
			char ifname_buf[IF_NAMESIZE];
			char* const result = ::if_indextoname(m_index, ifname_buf);

			if (result == nullptr)
			{
				throw boost::system::system_error(errno, boost::system::system_category(), "Unable to find an interface with the given index");
			}

			m_name_cache = std::string(result);
		}

		return m_name_cache;
	}

	route_manager::route_manager(boost::asio::io_service& io_service) :
		m_socket(io_service, netlink_route_protocol::endpoint())
	{
		m_socket.set_option(boost::asio::socket_base::send_buffer_size(32768));
		m_socket.set_option(boost::asio::socket_base::receive_buffer_size(32768));
	}

	route_entry route_manager::get_route_for(const boost::asio::ip::address& host)
	{
		using boost::asio::buffer_size;
		using boost::asio::buffer_cast;

		route_request_type request(RTM_GETROUTE);
		route_response_type response;
		request.set_route_destination(host);

		m_socket.send(boost::asio::buffer(request.data(), request.size()));
		const size_t cnt = m_socket.receive(boost::asio::buffer(response.data(), response.max_size()));

		if (!response.is_valid(cnt))
		{
			throw std::runtime_error("Invalid response");
		}

		if (response.header().nlmsg_type != RTM_NEWROUTE) {
			throw std::runtime_error("Invalid response type");
		}

		const int family = response.subheader().rtm_family;

		if (family == AF_INET)
		{
			return get_route_entry<boost::asio::ip::address_v4>(response.attributes());
		}
		else
		{
			return get_route_entry<boost::asio::ip::address_v6>(response.attributes());
		}
	}
}

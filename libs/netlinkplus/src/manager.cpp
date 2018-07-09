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
 * \file manager.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief netlink manager classes.
 */

#include "manager.hpp"
#include "messages.hpp"
#include "error.hpp"

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
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_destination));
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
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_source));
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.source = AddressType(bytes);

							break;
						}
					case RTA_IIF:
						{
							if (buffer_size(data) != sizeof(unsigned int))
							{
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_input_interface));
							}

							result.input_interface = interface_entry(*boost::asio::buffer_cast<unsigned int*>(data));

							break;
						}
					case RTA_OIF:
						{
							if (buffer_size(data) != sizeof(unsigned int))
							{
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_output_interface));
							}

							result.output_interface = interface_entry(*boost::asio::buffer_cast<unsigned int*>(data));

							break;
						}
					case RTA_GATEWAY:
						{
							typename AddressType::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_gateway));
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.gateway = boost::asio::ip::address(AddressType(bytes));

							break;
						}
					case RTA_PRIORITY:
						{
							if (buffer_size(data) != sizeof(result.priority))
							{
								throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_priority));
							}

							result.priority = *boost::asio::buffer_cast<unsigned int*>(data);

							break;
						}
					case RTA_METRICS:
						{
							if (buffer_size(data) != sizeof(result.metric))
							{
								// On some old GNU/Linux distributions, it causes issues.
								// In addition, RTA_METRICS processing is more complex than
								// that (data should be an array of rtattr structures).
								// So disable the exception from now.
								//throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_route_metric));
								result.metric = 0;
								break;
							}

							// TODO handle properly RTA_METRICS
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

	unsigned int interface_entry::get_index_from_name(const std::string& name_)
	{
		const unsigned int result = ::if_nametoindex(name_.c_str());

		if (result == 0)
		{
				throw boost::system::system_error(errno, boost::system::system_category(), "Unable to find an interface with the given name");
		}

		return result;
	}

	manager::manager(boost::asio::io_service& io_service) :
		m_socket(io_service, netlink_route_protocol::endpoint())
	{
		m_socket.set_option(boost::asio::socket_base::send_buffer_size(32768));
		m_socket.set_option(boost::asio::socket_base::receive_buffer_size(32768));
	}

	route_entry manager::get_route_for(const boost::asio::ip::address& host)
	{
		using boost::asio::buffer_size;
		using boost::asio::buffer_cast;

		route_request_type request(RTM_GETROUTE, NLM_F_REQUEST);
		route_response_type response;
		request.set_route_destination(host);

		m_socket.send(boost::asio::buffer(request.data(), request.size()));
		const size_t cnt = m_socket.receive(boost::asio::buffer(response.data(), response.max_size()));

		if (!response.is_valid(cnt))
		{
			throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_response));
		}

		if (response.header().nlmsg_type != RTM_NEWROUTE) {
			throw boost::system::system_error(make_error_code(netlinkplus_error::unexpected_response_type));
		}

		const int family = response.subheader().rtm_family;

		route_entry result;

		if (family == AF_INET)
		{
			result = get_route_entry<boost::asio::ip::address_v4>(response.attributes());
		}
		else
		{
			result = get_route_entry<boost::asio::ip::address_v6>(response.attributes());
		}

		result.destination_length = response.subheader().rtm_dst_len;
		result.source_length = response.subheader().rtm_src_len;

		return result;
	}

	void manager::add_route(const interface_entry& interface, const boost::asio::ip::address& destination, unsigned int destination_length, boost::optional<boost::asio::ip::address> gateway)
	{
		generic_route(RTM_NEWROUTE, interface, destination, destination_length, gateway);
	}

	void manager::remove_route(const interface_entry& interface, const boost::asio::ip::address& destination, unsigned int destination_length, boost::optional<boost::asio::ip::address> gateway)
	{
		generic_route(RTM_DELROUTE, interface, destination, destination_length, gateway);
	}

	void manager::add_interface_address(const interface_entry& interface, const boost::asio::ip::address& address, size_t prefix_length)
	{
		add_interface_address(interface, address, prefix_length, address);
	}

	void manager::add_interface_address(const interface_entry& interface, const boost::asio::ip::address& address, size_t prefix_length, const boost::asio::ip::address& remote_address)
	{
		generic_interface_address(RTM_NEWADDR, interface, address, prefix_length, remote_address);
	}

	void manager::remove_interface_address(const interface_entry& interface, const boost::asio::ip::address& address, size_t prefix_length)
	{
		remove_interface_address(interface, address, prefix_length, address);
	}

	void manager::remove_interface_address(const interface_entry& interface, const boost::asio::ip::address& address, size_t prefix_length, const boost::asio::ip::address& remote_address)
	{
		generic_interface_address(RTM_DELADDR, interface, address, prefix_length, remote_address);
	}

	void manager::generic_route(uint16_t type, const interface_entry& interface, const boost::asio::ip::address& destination, unsigned int destination_length, boost::optional<boost::asio::ip::address> gateway)
	{
		using boost::asio::buffer_size;
		using boost::asio::buffer_cast;

		int flags = NLM_F_REQUEST | NLM_F_ACK;

		if (type == RTM_NEWROUTE)
		{
			flags |= NLM_F_CREATE | NLM_F_EXCL;
		}

		route_request_type request(type, flags);
		error_message_type response;

		request.subheader().rtm_table = RT_TABLE_MAIN;
		request.subheader().rtm_scope = RT_SCOPE_UNIVERSE;
		request.subheader().rtm_type = RTN_UNICAST;
		request.subheader().rtm_protocol = RTPROT_STATIC;

		request.set_route_destination(destination, destination_length);
		request.set_output_interface(interface.index());

		if (gateway)
		{
			request.set_gateway(*gateway);
		}

		m_socket.send(boost::asio::buffer(request.data(), request.size()));
		const size_t cnt = m_socket.receive(boost::asio::buffer(response.data(), response.max_size()));

		if (!response.is_valid(cnt))
		{
			throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_response));
		}

		if (response.header().nlmsg_type != NLMSG_ERROR)
		{
			throw boost::system::system_error(make_error_code(netlinkplus_error::unexpected_response_type));
		}

		if (response.subheader().error != 0)
		{
			throw boost::system::system_error(-response.subheader().error, boost::system::system_category());
		}
	}

	void manager::generic_interface_address(uint16_t type, const interface_entry& interface, const boost::asio::ip::address& address, size_t prefix_length, const boost::asio::ip::address& remote_address)
	{
		using boost::asio::buffer_size;
		using boost::asio::buffer_cast;

		address_request_type request(type, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK | NLM_F_EXCL);
		error_message_type response;
		request.set_interface(interface.index());
		request.set_address(remote_address);
		request.set_local_address(address);
		request.set_prefix_length(prefix_length);

		m_socket.send(boost::asio::buffer(request.data(), request.size()));
		const size_t cnt = m_socket.receive(boost::asio::buffer(response.data(), response.max_size()));

		if (!response.is_valid(cnt))
		{
			throw boost::system::system_error(make_error_code(netlinkplus_error::invalid_response));
		}

		if (response.header().nlmsg_type != NLMSG_ERROR)
		{
			throw boost::system::system_error(make_error_code(netlinkplus_error::unexpected_response_type));
		}

		if (response.subheader().error != 0)
		{
			throw boost::system::system_error(-response.subheader().error, boost::system::system_category());
		}
	}
}

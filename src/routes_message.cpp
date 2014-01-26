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
 * \file routes_message.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The routes messages exchanged by the peers.
 */

#include "routes_message.hpp"

#include <cassert>

namespace freelan
{
	namespace
	{
		enum ip_network_address_type
		{
			INAT_IPV4 = 0x01,
			INAT_IPV6 = 0x02
		};

		/**
		 * \brief A visitor that writes the representation of a network address to a buffer.
		 */
		class ip_network_address_representation : public boost::static_visitor<size_t>
		{
			public:

				/**
				 * \brief Create a new ip_network_address_representation.
				 * \param buf The buffer to write the representation to.
				 * \param buf_len The length of buf.
				 */
				ip_network_address_representation(char* buf, size_t buf_len) :
					m_buf(buf),
					m_buf_len(buf_len)
				{}

				/**
				 * \brief Get the representation size of the network address.
				 * \param ina The ipv4_network_address.
				 * \return The representation size.
				 */
				template <typename AddressType>
				result_type operator()(const asiotap::base_ip_network_address<AddressType>& ina) const
				{
					const uint8_t prefix_length = static_cast<uint8_t>(ina.prefix_length());
					const typename asiotap::base_ip_network_address<AddressType>::address_type::bytes_type bytes = ina.address().to_bytes();

					if (m_buf_len < 2 + bytes.size())
					{
						throw std::runtime_error("buf_len");
					}

					fscp::buffer_tools::set<uint8_t>(m_buf, 0, static_cast<uint8_t>(INAT_IPV4));
					fscp::buffer_tools::set<uint8_t>(m_buf, 1, static_cast<uint8_t>(prefix_length));

					std::copy(bytes.begin(), bytes.end(), m_buf + 2);

					return 2 + bytes.size();
				}

			private:

				char* m_buf;
				size_t m_buf_len;
		};
	}

	size_t routes_message::write(void* buf, size_t buf_len, version_type _version, const asiotap::ip_routes_set& routes)
	{
		if (buf_len < HEADER_LENGTH)
		{
			throw std::runtime_error("buf_len");
		}

		size_t required_size = 0;
		char* pbuf = static_cast<char*>(buf) + HEADER_LENGTH;
		size_t pbuf_len = buf_len - HEADER_LENGTH;

		fscp::buffer_tools::set<uint32_t>(pbuf, 0, htonl(static_cast<uint32_t>(_version)));

		required_size += sizeof(uint32_t);
		pbuf += sizeof(uint32_t);
		pbuf_len -= sizeof(uint32_t);

		for (auto&& route : routes)
		{
			const size_t count = boost::apply_visitor(ip_network_address_representation(pbuf, pbuf_len), route);

			required_size += count;
			pbuf += count;
			pbuf_len -= count;
		}

		return message::write(buf, buf_len, MT_ROUTES, required_size);
	}

	routes_message::version_type routes_message::version() const
	{
		return ntohl(static_cast<version_type>(fscp::buffer_tools::get<uint32_t>(payload(), 0)));
	}

	const asiotap::ip_routes_set& routes_message::routes() const
	{
		if (!m_routes_cache)
		{
			asiotap::ip_routes_set result;

			const uint8_t* pbuf = payload() + sizeof(uint32_t);
			size_t pbuf_len = length() - sizeof(uint32_t);

			while (pbuf_len > 2)
			{
				switch (*pbuf)
				{
					case INAT_IPV4:
						{
							++pbuf;
							--pbuf_len;

							const unsigned int prefix_length = static_cast<uint8_t>(*pbuf);

							++pbuf;
							--pbuf_len;

							boost::asio::ip::address_v4::bytes_type bytes;

							if (pbuf_len < bytes.size())
							{
								throw std::runtime_error("Not enough bytes for the expected IPv4 address");
							}

							std::copy(pbuf, pbuf + bytes.size(), bytes.begin());

							pbuf += bytes.size();
							pbuf_len -= bytes.size();

							result.insert(asiotap::ipv4_network_address(boost::asio::ip::address_v4(bytes), prefix_length));

							break;
						}

					case INAT_IPV6:
						{
							++pbuf;
							--pbuf_len;

							const unsigned int prefix_length = static_cast<uint8_t>(*pbuf);

							++pbuf;
							--pbuf_len;

							boost::asio::ip::address_v6::bytes_type bytes;

							if (pbuf_len < bytes.size())
							{
								throw std::runtime_error("Not enough bytes for the expected IPv6 address");
							}

							std::copy(pbuf, pbuf + bytes.size(), bytes.begin());

							pbuf += bytes.size();
							pbuf_len -= bytes.size();

							result.insert(asiotap::ipv6_network_address(boost::asio::ip::address_v6(bytes), prefix_length));

							break;
						}

					default:
						throw std::runtime_error("Unknown route type in message");
				}
			}

			if (pbuf_len > 0)
			{
				throw std::runtime_error("Unexpected bytes at the end of the routes list");
			}

			m_routes_cache = result;
		}

		return *m_routes_cache;
	}

	routes_message::routes_message(const void* buf, size_t buf_len) :
		message(buf, buf_len)
	{
		routes();
	}

	routes_message::routes_message(const message& _message) :
		message(_message)
	{
		routes();
	}
}

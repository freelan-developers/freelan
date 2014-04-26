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
 * \file netlink.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief NETLINK sockets classes.
 */

#include "linux/netlink.hpp"

#include <memory>
#include <cassert>
#include <iterator>

namespace asiotap
{
	namespace netlink
	{
		namespace
		{
			template <size_t DataSize>
			class route_message_type
			{
				public:
					template <class T>
					class base_attribute_iterator : public std::iterator<std::forward_iterator_tag, T>
					{
						public:

							base_attribute_iterator& operator++()
							{
								if (RTA_OK(m_ptr, m_ptr_len))
								{
									m_ptr = RTA_NEXT(m_ptr, m_ptr_len);
								}
								else
								{
									*this = base_attribute_iterator();
								}

								return *this;
							}

							base_attribute_iterator operator++(int)
							{
								base_attribute_iterator tmp(*this);
								operator++();
								return tmp;
							}

							bool operator==(const base_attribute_iterator& rhs) const
							{
								return (rhs.m_ptr == m_ptr);
							}

							bool operator!=(const base_attribute_iterator& rhs) const
							{
								return (rhs.m_ptr != m_ptr);
							}

							typename base_attribute_iterator::reference operator*()
							{
								return *m_ptr;
							}

						protected:

							explicit base_attribute_iterator() :
								m_ptr(nullptr), m_ptr_len()
							{
							}

							explicit base_attribute_iterator(typename base_attribute_iterator::pointer ptr, size_t ptr_len) :
								m_ptr(ptr),
								m_ptr_len(ptr_len)
							{
							}

						private:
							typename base_attribute_iterator::pointer m_ptr;
							size_t m_ptr_len;

							friend class route_message_type;
					};

					typedef base_attribute_iterator<::rtattr> attribute_iterator;
					typedef base_attribute_iterator<const ::rtattr> const_attribute_iterator;

					template <class MsgType>
					class base_attributes_type
					{
						public:

							const_attribute_iterator begin() const
							{
								return const_attribute_iterator(m_msg.first_attribute(), payload_size());
							}

							const_attribute_iterator end() const
							{
								return {};
							}

						protected:

							explicit base_attributes_type(MsgType msg) :
								m_msg(msg)
							{
							}

							MsgType m_msg;

							friend class route_message_type;
					};

					typedef base_attributes_type<const route_message_type&> const_attributes_type;

					class attributes_type : public base_attributes_type<route_message_type&>
					{
						public:

							attribute_iterator begin()
							{
								return attribute_iterator(this->m_msg.first_attribute(), this->m_msg.payload_size());
							}

							attribute_iterator end()
							{
								return {};
							}

						protected:

							explicit attributes_type(route_message_type& msg) :
								base_attributes_type<route_message_type&>(msg)
							{
							}

							friend class route_message_type;
					};

					explicit route_message_type(uint16_t type = 0, uint16_t flags = 0) :
						m_nlm{},
						m_rtm{},
						m_payload{}
					{
						m_nlm.nlmsg_len = NLMSG_LENGTH(sizeof(m_rtm));
						m_nlm.nlmsg_type = type;
						m_nlm.nlmsg_flags = flags;
					}

					void set_route_source(const boost::asio::ip::address& src)
					{
						if (src.is_v4())
						{
							m_rtm.rtm_family = AF_INET;
							const auto bytes = src.to_v4().to_bytes();
							push_attribute(RTA_SRC, bytes);
							m_rtm.rtm_src_len = bytes.size() * 8;
						}
						else
						{
							m_rtm.rtm_family = AF_INET6;
							const auto bytes = src.to_v6().to_bytes();
							push_attribute(RTA_SRC, bytes);
							m_rtm.rtm_src_len = bytes.size() * 8;
						}
					}

					void set_route_destination(const boost::asio::ip::address& dest)
					{
						if (dest.is_v4())
						{
							m_rtm.rtm_family = AF_INET;
							const auto bytes = dest.to_v4().to_bytes();
							push_attribute(RTA_DST, bytes);
							m_rtm.rtm_dst_len = bytes.size() * 8;
						}
						else
						{
							m_rtm.rtm_family = AF_INET6;
							const auto bytes = dest.to_v6().to_bytes();
							push_attribute(RTA_DST, bytes);
							m_rtm.rtm_dst_len = bytes.size() * 8;
						}
					}

					template <typename ValueType>
					void push_attribute(int type, const ValueType& value)
					{
						const auto attribute_len = RTA_LENGTH(value.size());
						const auto required_size = size() + attribute_len;

						assert(required_size < sizeof(route_message_type));

						// The remaining size is big enough: let's create an attribute.
						::rtattr* const attribute = next_attribute();
						attribute->rta_type = type;
						attribute->rta_len = attribute_len;
						::memcpy(RTA_DATA(attribute), static_cast<const void*>(value.data()), value.size());

						// Resize the message accordingly.
						resize(size() + attribute_len);
					}

					size_t header_size() const
					{
						return sizeof(m_nlm) + sizeof(m_rtm);
					}

					size_t payload_size() const
					{
						return size() - header_size();
					}

					size_t size() const
					{
						return NLMSG_ALIGN(m_nlm.nlmsg_len);
					}

					size_t max_size() const
					{
						return sizeof(route_message_type);
					}

					void resize(size_t new_size)
					{
						m_nlm.nlmsg_len = NLMSG_ALIGN(new_size);
					}

					::rtattr* first_attribute()
					{
						return reinterpret_cast<::rtattr*>(reinterpret_cast<char*>(&m_nlm) + header_size());
					}

					const ::rtattr* first_attribute() const
					{
						return reinterpret_cast<const ::rtattr*>(reinterpret_cast<const char*>(&m_nlm) + header_size());
					}

					::rtattr* next_attribute()
					{
						return reinterpret_cast<::rtattr*>(reinterpret_cast<char*>(&m_nlm) + size());
					}

					const ::rtattr* next_attribute() const
					{
						return reinterpret_cast<const ::rtattr*>(reinterpret_cast<const char*>(&m_nlm) + size());
					}

					void* data()
					{
						return reinterpret_cast<void*>(this);
					}

					const void* data() const
					{
						return reinterpret_cast<void*>(this);
					}

					::nlmsghdr& header()
					{
						return m_nlm;
					}

					const ::nlmsghdr& header() const
					{
						return m_nlm;
					}

					::rtmsg& route_header()
					{
						return m_rtm;
					}

					const ::rtmsg& route_header() const
					{
						return m_rtm;
					}

					bool is_valid(size_t cnt) const
					{
						return NLMSG_OK(&m_nlm, cnt);
					}

					attributes_type attributes()
					{
						return attributes_type(*this);
					}

					const_attributes_type attributes() const
					{
						return const_attributes_type(*this);
					}

				private:
					::nlmsghdr m_nlm;
					::rtmsg m_rtm;
					std::array<char, NLMSG_ALIGN(DataSize)> m_payload;
			};

			template <size_t DataSize = 1024>
			class route_request_type : public route_message_type<DataSize>
			{
				public:
					explicit route_request_type(uint16_t type = 0) :
						route_message_type<DataSize>(type, NLM_F_REQUEST)
					{
					}
			};

			template <size_t DataSize = 1024>
			class route_response_type : public route_message_type<DataSize>
			{
			};
		}

		route_entry get_route_for(const boost::asio::ip::address& host)
		{
			using boost::asio::buffer_size;
			using boost::asio::buffer_cast;

			boost::asio::io_service io_service;
			netlink_protocol<NETLINK_ROUTE>::endpoint ep;
			netlink_protocol<NETLINK_ROUTE>::socket socket(io_service, ep);
			socket.set_option(boost::asio::socket_base::send_buffer_size(32768));
			socket.set_option(boost::asio::socket_base::receive_buffer_size(32768));

			route_request_type<> request(RTM_GETROUTE);
			route_response_type<> response;
			request.set_route_destination(host);

			socket.send(boost::asio::buffer(request.data(), request.size()));

			const size_t cnt = socket.receive(boost::asio::buffer(response.data(), response.max_size()));

			if (!response.is_valid(cnt))
			{
				throw std::runtime_error("Invalid response");
			}

			if (response.header().nlmsg_type != RTM_NEWROUTE) {
				throw std::runtime_error("Invalid response type");
			}

			const int family = response.route_header().rtm_family;

			route_entry result;

			for (auto&& attribute : response.attributes())
			{
				const auto data = boost::asio::buffer(RTA_DATA(&attribute), RTA_PAYLOAD(&attribute));

				switch (attribute.rta_type)
				{
					case RTA_GATEWAY:
					{
						if (family == AF_INET)
						{
							boost::asio::ip::address_v4::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw std::runtime_error("Invalid gateway value");
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.gateway = boost::asio::ip::address(boost::asio::ip::address_v4(bytes));
						}
						else
						{
							boost::asio::ip::address_v6::bytes_type bytes;

							if (bytes.size() != buffer_size(data))
							{
								throw std::runtime_error("Invalid gateway value");
							}

							::memcpy(bytes.data(), boost::asio::buffer_cast<const void*>(data), buffer_size(data));
							result.gateway = boost::asio::ip::address(boost::asio::ip::address_v6(bytes));
						}

						break;
					}
					case RTA_OIF:
					{
						if (buffer_size(data) != sizeof(result.interface))
						{
							throw std::runtime_error("Invalid interface value");
						}

						result.interface = *boost::asio::buffer_cast<unsigned int*>(data);
					}
				}
			}

			return result;
		}
	}
}

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
 * \file messages.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Route message classes.
 */

#include "generic_message.hpp"

#include <iterator>
#include <cassert>

namespace netlinkplus
{
	namespace
	{
		template <size_t DataSize>
		class route_message_type : public generic_message_type<::rtmsg, DataSize>
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

				class attributes_type
				{
					public:

						const_attribute_iterator begin() const
						{
							return const_attribute_iterator(this->m_msg.first_attribute(), this->m_msg.payload_size());
						}

						const_attribute_iterator end() const
						{
							return {};
						}

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
							m_msg(msg)
						{
						}

						route_message_type& m_msg;

						friend class route_message_type;
				};

				explicit route_message_type(uint16_t type = 0, uint16_t flags = 0) :
					generic_message_type<::rtmsg, DataSize>(type, flags)
				{
				}

				void set_route_source(const boost::asio::ip::address& src)
				{
					if (src.is_v4())
					{
						this->subheader().rtm_family = AF_INET;
						const auto bytes = src.to_v4().to_bytes();
						push_attribute(RTA_SRC, bytes);
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
					else
					{
						this->subheader().rtm_family = AF_INET6;
						const auto bytes = src.to_v6().to_bytes();
						push_attribute(RTA_SRC, bytes);
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
				}

				void set_route_destination(const boost::asio::ip::address& dest)
				{
					if (dest.is_v4())
					{
						this->subheader().rtm_family = AF_INET;
						const auto bytes = dest.to_v4().to_bytes();
						push_attribute(RTA_DST, bytes);
						this->subheader().rtm_dst_len = bytes.size() * 8;
					}
					else
					{
						this->subheader().rtm_family = AF_INET6;
						const auto bytes = dest.to_v6().to_bytes();
						push_attribute(RTA_DST, bytes);
						this->subheader().rtm_dst_len = bytes.size() * 8;
					}
				}

				template <typename ValueType>
				void push_attribute(int type, const ValueType& value)
				{
					const auto attribute_len = RTA_LENGTH(value.size());
					const auto required_size = this->size() + attribute_len;

					assert(required_size < sizeof(route_message_type));

					// The remaining size is big enough: let's create an attribute.
					::rtattr* const attribute = next_attribute();
					attribute->rta_type = type;
					attribute->rta_len = attribute_len;
					::memcpy(RTA_DATA(attribute), static_cast<const void*>(value.data()), value.size());

					// Resize the message accordingly.
					this->resize(this->size() + attribute_len);
				}

				::rtattr* first_attribute()
				{
					return reinterpret_cast<::rtattr*>(reinterpret_cast<char*>(this->data()) + this->header_size());
				}

				const ::rtattr* first_attribute() const
				{
					return reinterpret_cast<const ::rtattr*>(reinterpret_cast<const char*>(this->data()) + this->header_size());
				}

				::rtattr* next_attribute()
				{
					return reinterpret_cast<::rtattr*>(reinterpret_cast<char*>(this->data()) + this->size());
				}

				const ::rtattr* next_attribute() const
				{
					return reinterpret_cast<const ::rtattr*>(reinterpret_cast<const char*>(this->data()) + this->size());
				}

				attributes_type attributes()
				{
					return attributes_type(*this);
				}
		};

		class route_request_type : public route_message_type<1024>
		{
			public:
				explicit route_request_type(uint16_t type = 0) :
					route_message_type<1024>(type, NLM_F_REQUEST)
				{
				}
		};

		class route_response_type : public route_message_type<1024>
		{
		};
	}
}

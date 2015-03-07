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
	template <typename Type>
	class attribute_message
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

							if (m_ptr_len == 0)
							{
								*this = base_attribute_iterator();
							}
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
						m_ptr(ptr_len > 0 ? ptr : nullptr),
						m_ptr_len(ptr_len)
					{
					}

				private:
					typename base_attribute_iterator::pointer m_ptr;
					size_t m_ptr_len;

					friend class attribute_message;
			};

			typedef base_attribute_iterator< ::rtattr> attribute_iterator;
			typedef base_attribute_iterator<const ::rtattr> const_attribute_iterator;

			class attributes_type
			{
				public:

					const_attribute_iterator begin() const
					{
						return const_attribute_iterator(this->m_msg.first_attribute(), static_cast<const Type&>(this->m_msg).payload_size());
					}

					const_attribute_iterator end() const
					{
						return const_attribute_iterator();
					}

					attribute_iterator begin()
					{
						return attribute_iterator(this->m_msg.first_attribute(), static_cast<const Type&>(this->m_msg).payload_size());
					}

					attribute_iterator end()
					{
						return {};
					}

				protected:

					explicit attributes_type(attribute_message<Type>& msg) :
						m_msg(msg)
					{
					}

					attribute_message<Type>& m_msg;

					friend class attribute_message;
			};

			template <typename ValueType>
			static size_t value_size(const ValueType& value)
			{
				return value.size();
			}

			template <typename ValueType>
			static const void* value_data(const ValueType& value)
			{
				return static_cast<const void*>(value.data());
			}

			static size_t value_size(const std::string& value)
			{
				return value.size() + 1;
			}

			static const void* value_data(const std::string& value)
			{
				return static_cast<const void*>(value.c_str());
			}

			static size_t value_size(unsigned int)
			{
				return sizeof(unsigned int);
			}

			static const void* value_data(const unsigned int& value)
			{
				return static_cast<const void*>(&value);
			}

			template <typename ValueType>
			void push_attribute(int type, const ValueType& value)
			{
				const auto attribute_len = RTA_LENGTH(value_size(value));
				const auto required_size = static_cast<Type*>(this)->size() + attribute_len;

				assert(required_size < sizeof(Type));

				// The remaining size is big enough: let's create an attribute.
				::rtattr* const attribute = next_attribute();
				attribute->rta_type = type;
				attribute->rta_len = attribute_len;
				::memcpy(RTA_DATA(attribute), value_data(value), value_size(value));

				// Resize the message accordingly.
				static_cast<Type*>(this)->resize(static_cast<Type*>(this)->size() + attribute_len);
			}

			::rtattr* first_attribute()
			{
				return reinterpret_cast< ::rtattr*>(static_cast<Type*>(this)->payload());
			}

			const ::rtattr* first_attribute() const
			{
				return reinterpret_cast<const ::rtattr*>(static_cast<const Type*>(this)->payload());
			}

			::rtattr* next_attribute()
			{
				return reinterpret_cast< ::rtattr*>(static_cast<Type*>(this)->end());
			}

			const ::rtattr* next_attribute() const
			{
				return reinterpret_cast<const ::rtattr*>(static_cast<const Type*>(this)->end());
			}

			attributes_type attributes()
			{
				return attributes_type(*this);
			}

		protected:

			void generic_set_address(int type, const boost::asio::ip::address& _address)
			{
				if (_address.is_v4())
				{
					const auto bytes = _address.to_v4().to_bytes();
					this->push_attribute(type, bytes);
				}
				else
				{
					const auto bytes = _address.to_v6().to_bytes();
					this->push_attribute(type, bytes);
				}
			}
	};

	template <size_t DataSize>
	class route_message_type : public generic_message_type< ::rtmsg, DataSize>, public attribute_message<route_message_type<DataSize>>
	{
		public:
			explicit route_message_type(uint16_t type = 0, uint16_t flags = 0) :
				generic_message_type< ::rtmsg, DataSize>(type, flags)
			{
			}

			void set_route_source(const boost::asio::ip::address& src, boost::optional<unsigned int> src_len = boost::optional<unsigned int>())
			{
				if (src.is_v4())
				{
					this->subheader().rtm_family = AF_INET;
					const auto bytes = src.to_v4().to_bytes();
					this->push_attribute(RTA_SRC, bytes);

					if (src_len)
					{
						this->subheader().rtm_src_len = *src_len;
					}
					else
					{
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
				}
				else
				{
					this->subheader().rtm_family = AF_INET6;
					const auto bytes = src.to_v6().to_bytes();
					this->push_attribute(RTA_SRC, bytes);

					if (src_len)
					{
						this->subheader().rtm_src_len = *src_len;
					}
					else
					{
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
				}
			}

			void set_route_preferred_source(const boost::asio::ip::address& src, boost::optional<unsigned int> src_len = boost::optional<unsigned int>())
			{
				if (src.is_v4())
				{
					this->subheader().rtm_family = AF_INET;
					const auto bytes = src.to_v4().to_bytes();
					this->push_attribute(RTA_PREFSRC, bytes);

					if (src_len)
					{
						this->subheader().rtm_src_len = *src_len;
					}
					else
					{
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
				}
				else
				{
					this->subheader().rtm_family = AF_INET6;
					const auto bytes = src.to_v6().to_bytes();
					this->push_attribute(RTA_PREFSRC, bytes);

					if (src_len)
					{
						this->subheader().rtm_src_len = *src_len;
					}
					else
					{
						this->subheader().rtm_src_len = bytes.size() * 8;
					}
				}
			}

			void set_route_destination(const boost::asio::ip::address& dest, boost::optional<unsigned int> dest_len = boost::optional<unsigned int>())
			{
				if (dest.is_v4())
				{
					this->subheader().rtm_family = AF_INET;
					const auto bytes = dest.to_v4().to_bytes();
					this->push_attribute(RTA_DST, bytes);

					if (dest_len)
					{
						this->subheader().rtm_dst_len = *dest_len;
					}
					else
					{
						this->subheader().rtm_dst_len = bytes.size() * 8;
					}
				}
				else
				{
					this->subheader().rtm_family = AF_INET6;
					const auto bytes = dest.to_v6().to_bytes();
					this->push_attribute(RTA_DST, bytes);

					if (dest_len)
					{
						this->subheader().rtm_dst_len = *dest_len;
					}
					else
					{
						this->subheader().rtm_dst_len = bytes.size() * 8;
					}
				}
			}

			void set_input_interface(unsigned int interface)
			{
				this->push_attribute(RTA_IIF, interface);
			}

			void set_output_interface(unsigned int interface)
			{
				this->push_attribute(RTA_OIF, interface);
			}

			void set_gateway(const boost::asio::ip::address& gateway)
			{
				if (gateway.is_v4())
				{
					const auto bytes = gateway.to_v4().to_bytes();
					this->push_attribute(RTA_GATEWAY, bytes);
				}
				else
				{
					const auto bytes = gateway.to_v6().to_bytes();
					this->push_attribute(RTA_GATEWAY, bytes);
				}
			}

			void set_priority(unsigned int priority)
			{
				this->push_attribute(RTA_PRIORITY, priority);
			}

			void set_metrics(unsigned int metrics)
			{
				this->push_attribute(RTA_METRICS, metrics);
			}
	};

	class route_request_type : public route_message_type<1024>
	{
		public:
			explicit route_request_type(uint16_t type = 0, uint16_t flags = 0) :
				route_message_type<1024>(type, flags)
			{
			}
	};

	class route_response_type : public route_message_type<1024>
	{
	};

	template <size_t DataSize>
	class address_message_type : public generic_message_type< ::ifaddrmsg, DataSize>, public attribute_message<address_message_type<DataSize>>
	{
		public:
			explicit address_message_type(uint16_t type = 0, uint16_t flags = 0) :
				generic_message_type< ::ifaddrmsg, DataSize>(type, flags)
			{
				set_flags(IFA_F_PERMANENT | IFA_F_SECONDARY);
				set_scope(RT_SCOPE_UNIVERSE);
			}

			void set_flags(unsigned char _flags)
			{
				this->subheader().ifa_flags = _flags;
			}

			void set_scope(unsigned char _scope)
			{
				this->subheader().ifa_scope = _scope;
			}

			void set_address(const boost::asio::ip::address& _address)
			{
				if (_address.is_v4())
				{
					this->subheader().ifa_family = AF_INET;
				}
				else
				{
					this->subheader().ifa_family = AF_INET6;
				}

				this->generic_set_address(IFA_ADDRESS, _address);
			}

			void set_local_address(const boost::asio::ip::address& _local_address)
			{
				this->generic_set_address(IFA_LOCAL, _local_address);
			}

			void set_broadcast_address(const boost::asio::ip::address& _broadcast_address)
			{
				this->generic_set_address(IFA_BROADCAST, _broadcast_address);
			}

			void set_anycast_address(const boost::asio::ip::address& _anycast_address)
			{
				this->generic_set_address(IFA_ANYCAST, _anycast_address);
			}

			void set_prefix_length(unsigned int _prefix_length)
			{
				this->subheader().ifa_prefixlen = static_cast<unsigned char>(_prefix_length);
			}

			void set_interface(int _interface_index)
			{
				this->subheader().ifa_index = _interface_index;
			}

			void set_label(const std::string& _label)
			{
				this->push_attribute(IFA_LABEL, _label);
			}
	};

	class address_request_type : public address_message_type<1024>
	{
		public:
			explicit address_request_type(uint16_t type = 0, uint16_t flags = 0) :
				address_message_type<1024>(type, flags)
			{
			}
	};

	class address_response_type : public address_message_type<1024>
	{
	};

	class error_message_type : public generic_message_type< ::nlmsgerr, 1024>
	{
		public:
			explicit error_message_type(uint16_t type = 0, uint16_t flags = 0) :
				generic_message_type< ::nlmsgerr, 1024>(type, flags)
			{
			}
	};
}

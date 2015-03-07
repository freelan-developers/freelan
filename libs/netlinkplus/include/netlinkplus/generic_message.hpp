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
 * \file generic_message.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Base message class.
 */

#include <memory>

namespace netlinkplus
{
	template <typename Type, size_t DataSize>
	class base_generic_message_type
	{
		public:

			explicit base_generic_message_type(uint16_t type = 0, uint16_t flags = 0) :
				m_data{}
			{
				header().nlmsg_len = NLMSG_LENGTH(0);
				header().nlmsg_type = type;
				header().nlmsg_flags = flags;
			}

			size_t header_size() const
			{
				return sizeof(::nlmsghdr);
			}

			size_t payload_size() const
			{
				return static_cast<const Type*>(this)->size() - static_cast<const Type*>(this)->header_size();
			}

			size_t size() const
			{
				return NLMSG_ALIGN(static_cast<const Type*>(this)->header().nlmsg_len);
			}

			size_t max_size() const
			{
				return sizeof(Type);
			}

			void resize(size_t new_size)
			{
				static_cast<Type*>(this)->header().nlmsg_len = NLMSG_ALIGN(new_size);
			}

			::nlmsghdr& header()
			{
				return *reinterpret_cast< ::nlmsghdr*>(m_data.data());
			}

			const ::nlmsghdr& header() const
			{
				return *reinterpret_cast<const ::nlmsghdr*>(m_data.data());
			}

			void* data()
			{
				return m_data.data();
			}

			const void* data() const
			{
				return m_data.data();
			}

			void* payload()
			{
				return static_cast<char*>(data()) + static_cast<Type*>(this)->header_size();
			}

			const void* payload() const
			{
				return static_cast<const char*>(data()) + static_cast<const Type*>(this)->header_size();
			}

			void* end()
			{
				return static_cast<char*>(data()) + static_cast<Type*>(this)->size();
			}

			const void* end() const
			{
				return static_cast<const char*>(data()) + static_cast<const Type*>(this)->size();
			}

			bool is_valid(size_t cnt) const
			{
				return NLMSG_OK(&header(), cnt);
			}

		private:
			std::array<char, NLMSG_ALIGN(DataSize)> m_data;
	};

	template <typename SubHeaderType, size_t DataSize>
	class generic_message_type : public base_generic_message_type<generic_message_type<SubHeaderType, DataSize>, DataSize>
	{
		public:

			explicit generic_message_type(uint16_t type = 0, uint16_t flags = 0) :
				base_generic_message_type<generic_message_type<SubHeaderType, DataSize>, DataSize>(type, flags)
			{
				this->header().nlmsg_len = NLMSG_LENGTH(sizeof(SubHeaderType));
			}

			size_t header_size() const
			{
				return base_generic_message_type<generic_message_type<SubHeaderType, DataSize>, DataSize>::header_size() + sizeof(SubHeaderType);
			}

			SubHeaderType& subheader()
			{
				return *reinterpret_cast<SubHeaderType*>(reinterpret_cast<char*>(this->data()) + base_generic_message_type<generic_message_type<SubHeaderType, DataSize>, DataSize>::header_size());
			}

			const SubHeaderType& subheader() const
			{
				return *reinterpret_cast<const SubHeaderType*>(reinterpret_cast<const char*>(this->data()) + base_generic_message_type<generic_message_type<SubHeaderType, DataSize>, DataSize>::header_size());
			}
	};

	template <size_t DataSize>
	class generic_message_type<void, DataSize> : public base_generic_message_type<generic_message_type<void, DataSize>, DataSize>
	{
		public:

			explicit generic_message_type(uint16_t type = 0, uint16_t flags = 0) :
				base_generic_message_type<generic_message_type<void, DataSize>, DataSize>(type, flags)
			{
			}
	};
}

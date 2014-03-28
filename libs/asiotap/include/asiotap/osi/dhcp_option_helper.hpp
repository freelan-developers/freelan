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
 * \file dhcp_option_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP option helper class.
 */

#ifndef ASIOTAP_OSI_DHCP_OPTION_HELPER_HPP
#define ASIOTAP_OSI_DHCP_OPTION_HELPER_HPP

#include "helper.hpp"
#include "dhcp_option.hpp"

#include <boost/asio.hpp>

#include <stdexcept>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base DHCP option class.
		 */
		template <class HelperTag>
		class _base_dhcp_option_helper
		{
			public:

				/**
				 * \brief The buffer type.
				 */
				typedef typename helper_buffer<HelperTag>::type buffer_type;

				/**
				 * \brief The option tag type.
				 */
				typedef dhcp_option::dhcp_option_tag dhcp_option_tag;

				/**
				 * \brief Create a new DHCP option helper.
				 * \param buf The buffer to parse and use.
				 */
				_base_dhcp_option_helper(buffer_type buf);

				/**
				 * \brief Get the underlying buffer.
				 * \return The buffer.
				 */
				buffer_type buffer() const;

				/**
				 * \brief Get the option tag.
				 * \return The option tag.
				 */
				dhcp_option_tag tag() const;

				/**
				 * \brief Check if the option tag matches his length.
				 * \return true if the option is valid.
				 */
				bool is_valid() const;

				/**
				 * \brief Check if the option has a length.
				 * \return true if the option has a length.
				 */
				bool has_length() const;

				/**
				 * \brief Get the option length.
				 * \return The option length.
				 * \warning Calling this method when has_length() returns false has undefined behavior.
				 */
				size_t length() const;

				/**
				 * \brief Get the option value.
				 * \return The option value.
				 * \warning Calling this method when has_length() returns false has undefined behavior.
				 */
				buffer_type value() const;

				/**
				 * \brief Get the option value as the specified type.
				 * \return The value.
				 * \warning Calling this method when has_length() returns false has undefined behavior.
				 * \warning If the option value and its length mismatch, a std::logic_error is thrown.
				 */
				template <typename T>
				const T& value_as() const;

				/**
				 * \brief Get the total length.
				 * \return The total length of the option, including its tag and length.
				 */
				size_t total_length() const;

			protected:

				/**
				 * \brief Returns the size of the buffer.
				 * \return buffer size.
				 */
				size_t size() const;

				/**
				 * \brief Returns raw data.
				 * \return raw data.
				 */
				const uint8_t* const_data() const;

			private:

				/**
				 * \brief Buffer type.
				 */
				buffer_type m_buf;
		};

		template <class HelperTag>
		class dhcp_option_helper;

		template <>
		class dhcp_option_helper<const_helper_tag> : public _base_dhcp_option_helper<const_helper_tag>
		{
			public:

				/**
				 * \brief Create a new DHCP option helper.
				 * \param buf The buffer to parse and use.
				 */
				dhcp_option_helper(buffer_type buf);

				/**
				 * \brief Create a DHCP option helper from a mutable DHCP option helper.
				 * \param helper The helper.
				 */
				dhcp_option_helper(const dhcp_option_helper<mutable_helper_tag>& helper);
		};

		template <>
		class dhcp_option_helper<mutable_helper_tag> : public _base_dhcp_option_helper<mutable_helper_tag>
		{
			public:

				/**
				 * \brief Create a new DHCP option helper.
				 * \param buf The buffer to parse and use.
				 */
				dhcp_option_helper(buffer_type buf);

				/**
				 * \brief Set the tag.
				 * \param tag The option tag.
				 */
				void set_tag(dhcp_option_tag tag);

				/**
				 * \brief Set the length.
				 * \param length The length to set.
				 * \warning Calling this method when has_length() returns false has undefined behavior.
				 */
				void set_length(size_t length);

			private:

				uint8_t* data() const;
		};

		template <class HelperTag>
		inline _base_dhcp_option_helper<HelperTag>::_base_dhcp_option_helper(buffer_type buf) :
			m_buf(buf)
		{
			if (size() == 0)
			{
				throw std::logic_error("Invalid buffer size");
			}
		}

		template <class HelperTag>
		inline typename _base_dhcp_option_helper<HelperTag>::buffer_type _base_dhcp_option_helper<HelperTag>::buffer() const
		{
			return m_buf;
		}

		template <class HelperTag>
		inline typename _base_dhcp_option_helper<HelperTag>::dhcp_option_tag _base_dhcp_option_helper<HelperTag>::tag() const
		{
			return static_cast<dhcp_option_tag>(const_data()[0]);
		}

		template <class HelperTag>
		inline bool _base_dhcp_option_helper<HelperTag>::is_valid() const
		{
			return (!dhcp_option::has_length(tag()) || (size() > 1));
		}

		template <class HelperTag>
		inline bool _base_dhcp_option_helper<HelperTag>::has_length() const
		{
			return (size() > 1);
		}

		template <class HelperTag>
		inline size_t _base_dhcp_option_helper<HelperTag>::length() const
		{
			return static_cast<size_t>(const_data()[1]);
		}

		template <class HelperTag>
		inline typename _base_dhcp_option_helper<HelperTag>::buffer_type _base_dhcp_option_helper<HelperTag>::value() const
		{
			return boost::asio::buffer(buffer() + 2, length());
		}

		template <class HelperTag>
		template <typename T>
		const T& _base_dhcp_option_helper<HelperTag>::value_as() const
		{
			if (boost::asio::buffer_size(value()) != sizeof(T))
				throw std::logic_error("Value size and type mismatch");

			return *boost::asio::buffer_cast<const T*>(value());
		}

		template <class HelperTag>
		inline size_t _base_dhcp_option_helper<HelperTag>::total_length() const
		{
			if (dhcp_option::has_length(tag()) && has_length())
			{
				return 2 + length();
			}
			else
			{
				return 1;
			}
		}

		template <class HelperTag>
		inline size_t _base_dhcp_option_helper<HelperTag>::size() const
		{
			return boost::asio::buffer_size(buffer());
		}

		template <class HelperTag>
		inline const uint8_t* _base_dhcp_option_helper<HelperTag>::const_data() const
		{
			return boost::asio::buffer_cast<const uint8_t*>(buffer());
		}

		inline dhcp_option_helper<const_helper_tag>::dhcp_option_helper(buffer_type buf) :
			_base_dhcp_option_helper<const_helper_tag>(buf)
		{
		}

		inline dhcp_option_helper<const_helper_tag>::dhcp_option_helper(const dhcp_option_helper<mutable_helper_tag>& helper) :
			_base_dhcp_option_helper<const_helper_tag>(helper.buffer())
		{
		}

		inline dhcp_option_helper<mutable_helper_tag>::dhcp_option_helper(buffer_type buf) :
			_base_dhcp_option_helper<mutable_helper_tag>(buf)
		{
		}

		inline void dhcp_option_helper<mutable_helper_tag>::set_tag(dhcp_option_tag _tag)
		{
			data()[0] = static_cast<uint8_t>(_tag);
		}

		inline void dhcp_option_helper<mutable_helper_tag>::set_length(size_t _length)
		{
			data()[1] = static_cast<uint8_t>(_length);
		}

		inline uint8_t* dhcp_option_helper<mutable_helper_tag>::data() const
		{
			return boost::asio::buffer_cast<uint8_t*>(buffer());
		}
	}
}

#endif /* ASIOTAP_OSI_DHCP_OPTION_HELPER_HPP */


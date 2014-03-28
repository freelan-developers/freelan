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
 * \file dhcp_builder.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP frame builder class.
 */

#ifndef ASIOTAP_OSI_DHCP_BUILDER_HPP
#define ASIOTAP_OSI_DHCP_BUILDER_HPP

#include "builder.hpp"
#include "dhcp_frame.hpp"
#include "dhcp_option.hpp"

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An dhcp frame builder class.
		 */
		template <>
		class builder<dhcp_frame> : public _base_builder<dhcp_frame>
		{
			public:

				/**
				 * \brief Create a builder.
				 * \param buf The buffer to use.
				 * \warning buf will also be used internally to generate the temporary options buffer. That is, its whole content is undefined after calling this constructor.
				 */
				builder(boost::asio::mutable_buffer buf);

				/**
				 * \brief Add an option.
				 * \param tag The option tag.
				 */
				void add_option(dhcp_option::dhcp_option_tag tag);

				/**
				 * \brief Add an option.
				 * \param tag The option tag.
				 * \param value The option value.
				 */
				void add_option(dhcp_option::dhcp_option_tag tag, boost::asio::const_buffer value);

				/**
				 * \brief Add an option.
				 * \param tag The option tag.
				 * \param value The option value.
				 * \param value_size The size of value.
				 */
				void add_option(dhcp_option::dhcp_option_tag tag, const void* value, size_t value_size);

				/**
				 * \brief Add an option.
				 * \param tag The option tag.
				 * \param value The option value.
				 */
				void add_option(dhcp_option::dhcp_option_tag tag, uint8_t value);

				/**
				 * \brief Add padding.
				 * \param cnt The count of padding bytes to add.
				 */
				void add_padding(size_t cnt);

				/**
				 * \brief Complete with padding, if needed.
				 * \param size The minimum size to get to.
				 */
				void complete_padding(size_t size);

				/**
				 * \brief Write the frame.
				 * \return The total size of the written frame, including its payload.
				 */
				size_t write() const;

			private:

				size_t m_options_offset;
		};

		inline builder<dhcp_frame>::builder(boost::asio::mutable_buffer buf) :
			_base_builder<dhcp_frame>(buf, 0),
			m_options_offset(0)
		{
		}

		inline void builder<dhcp_frame>::add_option(dhcp_option::dhcp_option_tag tag, const void* value, size_t value_size)
		{
			add_option(tag, boost::asio::buffer(value, value_size));
		}

		inline void builder<dhcp_frame>::add_option(dhcp_option::dhcp_option_tag tag, uint8_t value)
		{
			add_option(tag, &value, sizeof(value));
		}

		inline void builder<dhcp_frame>::complete_padding(size_t size)
		{
			if (size > m_options_offset)
			{
				add_padding(size - m_options_offset);
			}
		}
	}
}

#endif /* ASIOTAP_DHCP_BUILDER_HPP */


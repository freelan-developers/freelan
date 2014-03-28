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
 * \file builder.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI frame builder class.
 */

#ifndef ASIOTAP_OSI_BUILDER_HPP
#define ASIOTAP_OSI_BUILDER_HPP

#include <boost/asio.hpp>

#include "helper.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base ethernet builder class.
		 */
		template <typename OSIFrameType>
		class _base_builder
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief The helper type.
				 */
				typedef mutable_helper<frame_type> helper_type;

				/**
				 * \brief Get the underlying buffer.
				 * \return The underlying buffer.
				 */
				boost::asio::mutable_buffer buffer() const;

				/**
				 * \brief Get the payload buffer.
				 * \return The payload buffer.
				 */
				boost::asio::mutable_buffer payload() const;

				/**
				 * \brief Get a helper.
				 * \param frame_size The frame size.
				 * \return The helper.
				 */
				helper_type get_helper(size_t frame_size = sizeof(frame_type)) const;

			protected:

				/**
				 * \brief Create a builder.
				 * \param buf The buffer to use.
				 * \param payload_size The size of the payload.
				 */
				_base_builder(boost::asio::mutable_buffer buf, size_t payload_size);

			private:

				const boost::asio::mutable_buffer m_buf;
				size_t m_payload_size;
		};

		/**
		 * \brief An frame builder class.
		 */
		template <typename OSIFrameType>
		class builder : public _base_builder<OSIFrameType>
		{
		};

		template <typename OSIFrameType>
		inline boost::asio::mutable_buffer _base_builder<OSIFrameType>::buffer() const
		{
			return m_buf;
		}

		template <typename OSIFrameType>
		inline boost::asio::mutable_buffer _base_builder<OSIFrameType>::payload() const
		{
			return m_buf + (boost::asio::buffer_size(m_buf) - m_payload_size);
		}

		template <typename OSIFrameType>
		inline typename _base_builder<OSIFrameType>::helper_type _base_builder<OSIFrameType>::get_helper(size_t frame_size) const
		{
			return helper_type(m_buf + (boost::asio::buffer_size(m_buf) - m_payload_size - frame_size));
		}

		template <typename OSIFrameType>
		inline _base_builder<OSIFrameType>::_base_builder(boost::asio::mutable_buffer buf, size_t payload_size) :
			m_buf(buf),
			m_payload_size(payload_size)
		{
		}

	}
}

#endif /* ASIOTAP_OSI_BUILDER_HPP */


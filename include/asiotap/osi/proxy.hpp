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
 * \file proxy.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI frame proxy class.
 */

#ifndef ASIOTAP_OSI_PROXY_HPP
#define ASIOTAP_OSI_PROXY_HPP

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief A base proxy class.
		 */
		template <typename OSIFrameType>
		class _base_proxy
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief The data available callback type.
				 */
				typedef boost::function<void (boost::asio::const_buffer)> data_available_callback_type;

				/**
				 * \brief Create a proxy.
				 * \param response_buffer The buffer to write the responses into.
				 * \param on_data_available The callback function to call when data is available for writing.
				 */
				_base_proxy(boost::asio::mutable_buffer response_buffer, data_available_callback_type on_data_available);

			protected:

				/**
				 * \brief Get the response buffer.
				 * \return The response buffer.
				 */
				boost::asio::mutable_buffer response_buffer() const;

				/**
				 * \brief Get the truncated response buffer.
				 * \param size The size of the truncated response buffer.
				 * \return The truncated response buffer.
				 */
				boost::asio::mutable_buffer get_truncated_response_buffer(size_t size) const;

				/**
				 * \brief Data is available.
				 * \return buffer The data.
				 */
				void data_available(boost::asio::const_buffer buffer);

			private:

				boost::asio::mutable_buffer m_response_buffer;
				data_available_callback_type m_on_data_available;
		};

		/**
		 * \brief A generic proxy class.
		 */
		template <typename OSIFrameType>
		class proxy : public _base_proxy<OSIFrameType>
		{
			private:

				void cannot_instantiate_an_unspecialized_proxy_class();
		};
		
		template <typename OSIFrameType>
		inline _base_proxy<OSIFrameType>::_base_proxy(boost::asio::mutable_buffer _response_buffer, data_available_callback_type on_data_available) :
			m_response_buffer(_response_buffer),
			m_on_data_available(on_data_available)
		{
			assert(m_on_data_available);
		}
		
		template <typename OSIFrameType>
		inline boost::asio::mutable_buffer _base_proxy<OSIFrameType>::response_buffer() const
		{
			return m_response_buffer;
		}

		template <typename OSIFrameType>
		inline boost::asio::mutable_buffer _base_proxy<OSIFrameType>::get_truncated_response_buffer(size_t size) const
		{
			return m_response_buffer + (boost::asio::buffer_size(m_response_buffer) - size);
		}
		
		template <typename OSIFrameType>
		inline void _base_proxy<OSIFrameType>::data_available(boost::asio::const_buffer buffer)
		{
			m_on_data_available(buffer);
		}
	}
}

#endif /* ASIOTAP_OSI_PROXY_HPP */

/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
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
 * \file osi_filter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI filter class.
 */

#ifndef ASIOTAP_OSI_FILTER_HPP
#define ASIOTAP_OSI_FILTER_HPP

#include <boost/asio.hpp>

namespace asiotap
{
	/**
	 * \brief The base filter class.
	 */
	template <typename OSIFrameType>
	class osi_filter
	{
		public:

			/**
			 * \brief The error codes.
			 */
			enum error_code
			{
				error_invalid, /**< The frame is invalid and should be dropped. */
				error_replied, /**< A reply has been generated for the sender. */
				error_ignored /**< The frame was not handled and should be forwarded to the next filter. */
			};

			/**
			 * \brief Processes an OSI frame.
			 * \param frame The frame.
			 * \param reply_frame The reply frame, if any. If the return value differs from osi_filter::error_reply, the value is not used.
			 * \return An error code that indicates the taken action.
			 */
			error_code process(const boost::asio::const_buffer& frame, const boost::asio::mutable_buffer& reply);

			/**
			 * \brief Process an OSI frame.
			 * \param frame The frame.
			 * \param reply_frame The reply frame, if any. If the return value differs from osi_filter::error_reply, the value is not used.
			 * \return An error code that indicates the taken action.
			 */
			error_code process(const OSIFrameType& frame, const boost::asio::mutable_buffer& reply);
			
		private:

			/**
			 * \brief Processes an OSI frame.
			 * \param frame The frame.
			 * \param reply_frame The reply frame, if any. If the return value differs from osi_filter::error_reply, the value is not used.
			 * \return An error code that indicates the taken action.
			 */
			virtual error_code do_process(const boost::asio::const_buffer& frame, const boost::asio::mutable_buffer& reply) = 0;
	};
	
	template <typename OSIFrameType>
	inline typename osi_filter<OSIFrameType>::error_code osi_filter<OSIFrameType>::process(const boost::asio::const_buffer& frame, const boost::asio::mutable_buffer& reply)
	{
		if (boost::asio::buffer_size(frame) < sizeof(OSIFrameType))
		{
			return error_invalid;
		}

		return do_process(frame, reply);
	}

	template <typename OSIFrameType>
	inline typename osi_filter<OSIFrameType>::error_code osi_filter<OSIFrameType>::process(const OSIFrameType& frame, const boost::asio::mutable_buffer& reply)
	{
		return do_process(boost::asio::const_buffer(static_cast<const void*>(&frame), sizeof(frame)), reply);
	}
}

#endif /* ASIOTAP_OSI_FILTER_HPP */


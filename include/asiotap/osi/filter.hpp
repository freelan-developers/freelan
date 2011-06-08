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
 * \file filter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI filter class.
 */

#ifndef ASIOTAP_OSI_FILTER_HPP
#define ASIOTAP_OSI_FILTER_HPP

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The error codes.
		 */
		enum filter_error_code
		{
			filter_error_invalid, /**< The frame is invalid and should be dropped. */
			filter_error_handled, /**< The frame was handled by the filter. */
			filter_error_ignored /**< The frame seems well-formed, but cannot be handled by the current filter. */
		};

		/**
		 * \brief The generic filter base class.
		 */
		template <typename OSIFrameType, class ParentFilterType>
		class _base_filter
			{
				public:

					/**
					 * \brief Virtual destructor.
					 */
					virtual ~_base_filter();

					/**
					 * \brief The frame type.
					 */
					typedef OSIFrameType frame_type;

					/**
					 * \brief The parent filter type.
					 */
					typedef ParentFilterType parent_filter_type;

				protected:

					/**
					 * \brief Check if the specified frame is big enough to be parsed.
					 * \param frame The frame.
					 * \return A pointer to the mapped frame structure, or NULL if the size does not match.
					 */
					static const frame_type* check_frame_size(const boost::asio::const_buffer& frame);

					/**
					 * \brief Processes only the payload of an OSI frame.
					 * \param frame The frame.
					 * \param payload The payload. Must point to &frame. If the return value is filter_error_handled, payload is updated to point on the payload of the specified frame.
					 * \return An error code that indicates the taken action.
					 */
					virtual filter_error_code process_payload(const frame_type& frame, boost::asio::const_buffer& payload) = 0;
			};

		/**
		 * \brief The generic filter class.
		 */
		template <typename OSIFrameType, class ParentFilterType = void>
		class filter : public _base_filter<OSIFrameType, ParentFilterType>
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef typename _base_filter<OSIFrameType, ParentFilterType>::frame_type frame_type;

				/**
				 * \brief The parent filter type.
				 */
				typedef typename _base_filter<OSIFrameType, ParentFilterType>::parent_filter_type parent_filter_type;

				/**
				 * \brief Processes an OSI frame.
				 * \param frame The frame. If the return value is filter_error_handled, frame is updated to point on the payload.
				 * \return An error code that indicates the taken action.
				 */
				filter_error_code process(boost::asio::const_buffer& frame);

			private:

				parent_filter_type m_parent;
		};

		/**
		 * \brief The generic filter class.
		 */
		template <typename OSIFrameType>
		class filter<OSIFrameType, void> : public _base_filter<OSIFrameType, void>
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef typename _base_filter<OSIFrameType, void>::frame_type frame_type;

				/**
				 * \brief The parent filter type.
				 */
				typedef typename _base_filter<OSIFrameType, void>::parent_filter_type parent_filter_type;

				/**
				 * \brief Processes an OSI frame.
				 * \param frame The frame. If the return value is filter_error_handled, frame is updated to point on the payload.
				 * \return An error code that indicates the taken action.
				 */
				filter_error_code process(boost::asio::const_buffer& frame);
		};

		template <typename OSIFrameType, class ParentFilterType>
		inline _base_filter<OSIFrameType, ParentFilterType>::~_base_filter()
		{
		}

		template <typename OSIFrameType, class ParentFilterType>
		const typename _base_filter<OSIFrameType, ParentFilterType>::frame_type* _base_filter<OSIFrameType, ParentFilterType>::check_frame_size(const boost::asio::const_buffer& frame)
		{
			if (boost::asio::buffer_size(frame) < sizeof(frame_type))
			{
				return NULL;
			}

			return boost::asio::buffer_cast<const frame_type*>(frame);
		}

		template <typename OSIFrameType, class ParentFilterType>
		inline filter_error_code filter<OSIFrameType, ParentFilterType>::process(boost::asio::const_buffer& frame)
		{
			filter_error_code result = m_parent.process(frame);

			if (result == filter_error_handled)
			{
				frame_type* frame_st = _base_filter<OSIFrameType, ParentFilterType>::check_frame_size(frame);

				if (!frame_st)
				{
					return filter_error_invalid;
				}

				return process_payload(*frame_st, frame);
			}

			return result;
		}

		template <typename OSIFrameType>
		inline filter_error_code filter<OSIFrameType, void>::process(boost::asio::const_buffer& frame)
		{
			frame_type* frame_st = _base_filter<OSIFrameType, void>::check_frame_size(frame);

			if (!frame_st)
			{
				return filter_error_invalid;
			}

			return process_payload(*frame_st, frame);
		}
	}
}

#endif /* ASIOTAP_OSI_FILTER_HPP */


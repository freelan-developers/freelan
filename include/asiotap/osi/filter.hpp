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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>

#include <vector>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief A frame cast.
		 * \param buf The buffer to cast.
		 * \return A pointer to the mapped data on success, NULL if the cast fails.
		 */
		template <typename OSIFrameType>
		OSIFrameType* frame_cast(const boost::asio::mutable_buffer& buf);

		/**
		 * \brief A frame cast.
		 * \param buf The buffer to cast.
		 * \return A pointer to the mapped data on success, NULL if the cast fails.
		 */
		template <typename OSIFrameType>
		const OSIFrameType* frame_cast(const boost::asio::const_buffer& buf);

		/**
		 * \brief Get the payload associated to a given frame.
		 * \param buf The buffer. Must point to a structure of the specified type. If the return value is not NULL, buf is updated to point on the payload of the specified frame.
		 * \return A pointer to the frame or NULL on failure.
		 */
		template <typename OSIFrameType>
		OSIFrameType* frame_parse(boost::asio::mutable_buffer& buf);

		/**
		 * \brief Get the payload associated to a given frame.
		 * \param buf The buffer. Must point to a structure of the specified type. If the return value is not NULL, buf is updated to point on the payload of the specified frame.
		 * \return A pointer to the frame or NULL on failure.
		 */
		template <typename OSIFrameType>
		const OSIFrameType* frame_parse(boost::asio::const_buffer& buf);

		/**
		 * \brief Get the payload associated to a given frame.
		 * \param buf The buffer. Must point to a structure of the specified type. If the return value is not NULL, buf is updated to point on the payload of the specified frame.
		 * \param parent The parent structure.
		 * \return A pointer to the frame or NULL on failure.
		 */
		template <typename OSIFrameType, typename ParentOSIFrameType>
		OSIFrameType* frame_parse(boost::asio::mutable_buffer& buf, const ParentOSIFrameType& parent);

		/**
		 * \brief Get the payload associated to a given frame.
		 * \param buf The buffer. Must point to a structure of the specified type. If the return value is not NULL, buf is updated to point on the payload of the specified frame.
		 * \param parent The parent structure.
		 * \return A pointer to the frame or NULL on failure.
		 */
		template <typename OSIFrameType, typename ParentOSIFrameType>
		const OSIFrameType* frame_parse(boost::asio::const_buffer& buf, const ParentOSIFrameType& parent);

		/**
		 * \brief A base filter class.
		 */
		template <typename OSIFrameType>
		class _base_filter
		{
			public:

				/**
				 * \brief The frame type.
				 */
				typedef OSIFrameType frame_type;

				/**
				 * \brief The frame handled callback.
				 */
				typedef boost::function<void (const OSIFrameType&, const boost::asio::const_buffer&)> frame_handled_callback;

				/**
				 * \brief Add a callback function.
				 * \param callback The callback function to add.
				 */
				void add_callback(frame_handled_callback callback);

			protected:

				void frame_handled(const OSIFrameType& frame, const boost::asio::const_buffer& payload) const;

			public:

				std::vector<frame_handled_callback> m_callbacks;
		};

		template <typename OSIFrameType, typename ParentFilterType = void>
		class filter : public _base_filter<OSIFrameType>
		{
			public:

				/**
				 * \brief Constructor.
				 * \param parent The parent filter.
				 */
				filter(ParentFilterType& parent);

				/**
				 * \brief Parse a frame.
				 * \param parent The parent frame.
				 * \param buf The frame buffer.
				 */
				void parse(const typename ParentFilterType::frame_type& parent, const boost::asio::const_buffer& buf) const;
		};

		template <typename OSIFrameType>
		class filter<OSIFrameType, void> : public _base_filter<OSIFrameType>
		{
			public:

				/**
				 * \brief Parse a frame.
				 * \param buf The frame buffer.
				 */
				void parse(const boost::asio::const_buffer& buf) const;
		};

		template <typename OSIFrameType>
		inline OSIFrameType* frame_cast(const boost::asio::mutable_buffer& buf)
		{
			if (boost::asio::buffer_size(buf) < sizeof(OSIFrameType))
			{
				return NULL;
			}

			return boost::asio::buffer_cast<OSIFrameType*>(buf);
		}

		template <typename OSIFrameType>
		inline const OSIFrameType* frame_cast(const boost::asio::const_buffer& buf)
		{
			if (boost::asio::buffer_size(buf) < sizeof(OSIFrameType))
			{
				return NULL;
			}

			return boost::asio::buffer_cast<const OSIFrameType*>(buf);
		}

		template <typename OSIFrameType>
		inline OSIFrameType* frame_parse(boost::asio::mutable_buffer& buf)
		{
			OSIFrameType* frame = frame_cast<OSIFrameType>(buf);

			if (frame)
			{
				buf = buf + sizeof(OSIFrameType);
			}

			return frame;
		}

		template <typename OSIFrameType>
		inline const OSIFrameType* frame_parse(boost::asio::const_buffer& buf)
		{
			const OSIFrameType* frame = frame_cast<OSIFrameType>(buf);

			if (frame)
			{
				buf = buf + sizeof(OSIFrameType);
			}

			return frame;
		}

		template <typename OSIFrameType, typename ParentOSIFrameType>
		inline OSIFrameType* frame_parse(boost::asio::mutable_buffer& buf, const ParentOSIFrameType& parent)
		{
			OSIFrameType* frame = frame_parse<OSIFrameType>(buf);

			if (frame && frame_parent_match(*frame, parent))
			{
				return frame;
			}

			return NULL;
		}

		template <typename OSIFrameType, typename ParentOSIFrameType>
		inline const OSIFrameType* frame_parse(boost::asio::const_buffer& buf, const ParentOSIFrameType& parent)
		{
			const OSIFrameType* frame = frame_parse<OSIFrameType>(buf);

			if (frame && frame_parent_match(*frame, parent))
			{
				return frame;
			}

			return NULL;
		}

		template <typename OSIFrameType>
		void _base_filter<OSIFrameType>::add_callback(frame_handled_callback callback)
		{
			m_callbacks.push_back(callback);
		}

		template <typename OSIFrameType>
		void _base_filter<OSIFrameType>::frame_handled(const OSIFrameType& frame, const boost::asio::const_buffer& payload) const
		{
			std::for_each(m_callbacks.begin(), m_callbacks.end(), boost::lambda::bind(boost::lambda::_1, frame, payload));
		}

		template <typename OSIFrameType, typename ParentFilterType>
		filter<OSIFrameType, ParentFilterType>::filter(ParentFilterType& parent)
		{
			parent.add_callback(boost::bind(&filter<OSIFrameType, ParentFilterType>::parse, this, _1, _2));
		}

		template <typename OSIFrameType, typename ParentFilterType>
		void filter<OSIFrameType, ParentFilterType>::parse(const typename ParentFilterType::frame_type& parent, const boost::asio::const_buffer& buf) const
		{
			boost::asio::const_buffer _buf = buf;

			const OSIFrameType* frame = frame_parse<OSIFrameType, typename ParentFilterType::frame_type>(_buf, parent);

			if (frame)
			{
				frame_handled(*frame, buf);
			}
		}

		template <typename OSIFrameType>
		void filter<OSIFrameType, void>::parse(const boost::asio::const_buffer& buf) const
		{
			boost::asio::const_buffer _buf = buf;

			const OSIFrameType* frame = frame_parse<OSIFrameType>(_buf);

			if (frame)
			{
				frame_handled(*frame, buf);
			}
		}
	}
}

#endif /* ASIOTAP_OSI_FILTER_HPP */


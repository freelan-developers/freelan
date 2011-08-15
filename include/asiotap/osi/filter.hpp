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
 * \file filter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An OSI filter class.
 */

#ifndef ASIOTAP_OSI_FILTER_HPP
#define ASIOTAP_OSI_FILTER_HPP

#include "helper.hpp"

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base template function to check for frame encapsulation.
		 * \param parent The parent frame.
		 * \return true if the parent frame should contain a frame of the specified type.
		 */
		template <typename OSIFrameType, typename ParentOSIFrameType>
		bool frame_parent_match(const_helper<ParentOSIFrameType> parent);

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
				 * \brief The frame filter callback.
				 */
				typedef boost::function<bool (const_helper<frame_type>)> frame_filter_callback;

				/**
				 * \brief The frame handler callback.
				 */
				typedef boost::function<void (const_helper<frame_type>)> frame_handler_callback;

				/**
				 * \brief Add a filter function.
				 * \param callback The filter function to add.
				 */
				void add_filter(frame_filter_callback callback);

				/**
				 * \brief Add a handler function.
				 * \param callback The handler function to add.
				 */
				void add_handler(frame_handler_callback callback);

				/**
				 * \brief Get the last helper.
				 * \return The last helper, if any.
				 */
				boost::optional<const_helper<frame_type> > get_last_helper() const;

			protected:

				void do_parse(boost::asio::const_buffer buf) const;
				bool filter_frame(const_helper<frame_type>) const;
				void frame_handled(const_helper<frame_type>) const;

			private:

				std::vector<frame_filter_callback> m_filters;
				std::vector<frame_handler_callback> m_handlers;
				mutable boost::optional<const_helper<frame_type> > m_last_helper;
		};

		/**
		 * \brief A generic filter class.
		 */
		template <typename OSIFrameType, typename ParentFilterType = void>
		class _filter : public _base_filter<OSIFrameType>
		{
			public:

				/**
				 * \brief The frame bridge filter callback.
				 */
				typedef boost::function<bool (const_helper<typename ParentFilterType::frame_type>, const_helper<OSIFrameType>)> frame_bridge_filter_callback;

				/**
				 * \brief Constructor.
				 * \param parent The parent filter.
				 */
				_filter(ParentFilterType& parent);

				/**
				 * \brief Get the parent filter.
				 * \return The parent filter.
				 */
				ParentFilterType& parent() const;

				/**
				 * \brief Add a bridge filter function.
				 * \param callback The bridge filter function to add.
				 */
				void add_bridge_filter(frame_bridge_filter_callback callback);

				/**
				 * \brief Parse a frame.
				 * \param parent The parent frame.
				 */
				void parse(const_helper<typename ParentFilterType::frame_type> parent) const;

			protected:

				bool bridge_filter_frame(const_helper<typename ParentFilterType::frame_type>, const_helper<OSIFrameType>) const;

			private:

				ParentFilterType& m_parent;
				std::vector<frame_bridge_filter_callback> m_bridge_filters;
		};

		/**
		 * \brief A generic filter class.
		 */
		template <typename OSIFrameType>
		class _filter<OSIFrameType, void> : public _base_filter<OSIFrameType>
		{
			public:

				/**
				 * \brief Parse the specified buffer.
				 * \param buf The buffer to parse.
				 */
				void parse(boost::asio::const_buffer buf) const;
		};

		/**
		 * \brief A generic filter class.
		 */
		template <typename OSIFrameType, typename ParentFilterType = void>
		class filter;

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::add_filter(frame_filter_callback callback)
		{
			m_filters.push_back(callback);
		}

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::add_handler(frame_handler_callback callback)
		{
			m_handlers.push_back(callback);
		}

		template <typename OSIFrameType>
		inline boost::optional<const_helper<OSIFrameType> > _base_filter<OSIFrameType>::get_last_helper() const
		{
			return m_last_helper;
		}

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::do_parse(boost::asio::const_buffer buf) const
		{
			try
			{
				m_last_helper = boost::none;

				const_helper<OSIFrameType> helper(buf);

				if (_base_filter<OSIFrameType>::filter_frame(helper))
				{
					_base_filter<OSIFrameType>::frame_handled(helper);
				}
			}
			catch (std::logic_error&)
			{
			}
		}

		template <typename OSIFrameType>
		inline bool _base_filter<OSIFrameType>::filter_frame(const_helper<OSIFrameType> helper) const
		{
			return (std::find_if(m_filters.begin(), m_filters.end(), !boost::bind(&frame_filter_callback::operator(), _1, helper)) == m_filters.end());
		}

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::frame_handled(const_helper<OSIFrameType> helper) const
		{
			m_last_helper = helper;

			std::for_each(m_handlers.begin(), m_handlers.end(), boost::bind(&frame_handler_callback::operator(), _1, *m_last_helper));
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline _filter<OSIFrameType, ParentFilterType>::_filter(ParentFilterType& _parent) :
			m_parent(_parent)
		{
			m_parent.add_handler(boost::bind(&_filter<OSIFrameType, ParentFilterType>::parse, this, _1));
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline ParentFilterType& _filter<OSIFrameType, ParentFilterType>::parent() const
		{
			return m_parent;
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline void _filter<OSIFrameType, ParentFilterType>::add_bridge_filter(frame_bridge_filter_callback callback)
		{
			m_bridge_filters.push_back(callback);
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline void _filter<OSIFrameType, ParentFilterType>::parse(const_helper<typename ParentFilterType::frame_type> parent_helper) const
		{
			if (frame_parent_match<OSIFrameType, typename ParentFilterType::frame_type>(parent_helper))
			{
				try
				{
					const_helper<OSIFrameType> helper(parent_helper.payload());

					if (_base_filter<OSIFrameType>::filter_frame(helper))
					{
						if (bridge_filter_frame(parent_helper, helper))
						{
							_base_filter<OSIFrameType>::frame_handled(helper);
						}
					}
				}
				catch (std::logic_error&)
				{
				}
			}
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline bool _filter<OSIFrameType, ParentFilterType>::bridge_filter_frame(const_helper<typename ParentFilterType::frame_type> parent_helper, const_helper<OSIFrameType> helper) const
		{
			return (std::find_if(m_bridge_filters.begin(), m_bridge_filters.end(), !boost::bind(&frame_bridge_filter_callback::operator(), _1, parent_helper, helper)) == m_bridge_filters.end());
		}

		template <typename OSIFrameType>
		inline void _filter<OSIFrameType, void>::parse(boost::asio::const_buffer buf) const
		{
			_base_filter<OSIFrameType>::do_parse(buf);
		}
	}
}

#endif /* ASIOTAP_OSI_FILTER_HPP */


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
 * \file extended_filter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An extended OSI filter class.
 */

#ifndef ASIOTAP_OSI_EXTENDED_FILTER_HPP
#define ASIOTAP_OSI_EXTENDED_FILTER_HPP

#include <boost/optional.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <vector>

#include "helper.hpp"

namespace asiotap
{
	namespace osi
	{
		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		class extended_filter
		{
			public:

				/**
				 * \brief The base filter type.
				 */
				typedef BaseFilterType base_filter_type;

				/**
				 * \brief The first sub-filter type.
				 */
				typedef Filter1TypeTemplate<base_filter_type> filter_1_type;

				/**
				 * \brief The base frame type.
				 */
				typedef typename base_filter_type::frame_type base_frame_type;

				/**
				 * \brief The first frame type.
				 */
				typedef typename filter_1_type::frame_type frame_1_type;

				/**
				 * \brief The base helper type.
				 */
				typedef const_helper<base_frame_type> base_helper_type;

				/**
				 * \brief The first helper type.
				 */
				typedef const_helper<frame_1_type> helper_1_type;

				/**
				 * \brief The frame handler callback.
				 */
				typedef boost::function<void (base_helper_type, helper_1_type)> frame_handler_callback;

				/**
				 * \brief Create a new extended filter.
				 */
				extended_filter();

				/**
				 * \brief Add a handler function.
				 * \param callback The handler function to add.
				 */
				void add_handler(frame_handler_callback callback);

			protected:

				void reset();
				void frame_handled(helper_1_type helper) const;

			private:

				void base_handler(base_helper_type);
				void handler_1(helper_1_type);

				base_filter_type m_base_filter;
				filter_1_type m_filter_1;

				boost::optional<base_helper_type> m_base_helper;

				std::vector<frame_handler_callback> m_handlers;
		};
		
		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		inline extended_filter<BaseFilterType, Filter1TypeTemplate>::extended_filter() :
			m_filter_1(m_base_filter)
		{
			m_base_filter.add_handler(boost::bind(&base_handler, this, _1));
			m_filter_1.add_handler(boost::bind(&handler_1, this, _1));
		}

		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		inline void extended_filter<BaseFilterType, Filter1TypeTemplate>::add_handler(frame_handler_callback callback)
		{
			m_handlers.push_back(callback);
		}

		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		inline void extended_filter<BaseFilterType, Filter1TypeTemplate>::reset()
		{
			m_base_helper = boost::none;
		}

		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		void extended_filter<BaseFilterType, Filter1TypeTemplate>::frame_handled(helper_1_type helper) const
		{
			std::for_each(m_handlers.begin(), m_handlers.end(), boost::bind(&frame_handler_callback::operator(), _1, *m_base_helper, helper));
		}

		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		inline void extended_filter<BaseFilterType, Filter1TypeTemplate>::base_handler(base_helper_type helper)
		{
			m_base_helper = helper;
		}

		template <class BaseFilterType, template <typename ParentFilter> class Filter1TypeTemplate>
		inline void extended_filter<BaseFilterType, Filter1TypeTemplate>::handler_1(helper_1_type helper)
		{
			frame_handled(helper);
		}
	}
}

#endif /* ASIOTAP_OSI_EXTENDED_FILTER_HPP */


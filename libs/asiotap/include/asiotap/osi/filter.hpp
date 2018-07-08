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
		 * \brief The base template function to check for frame encapsulation.
		 * \param parent The parent frame.
		 * \return true if the parent frame should contain a frame of the specified type.
		 */
		template <typename OSIFrameType, typename ParentOSIFrameType>
		inline bool frame_parent_match(mutable_helper<ParentOSIFrameType> parent) {
			return frame_parent_match<OSIFrameType, ParentOSIFrameType>(const_helper<ParentOSIFrameType>(parent));
		}

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
				typedef boost::function<void (mutable_helper<frame_type>)> frame_handler_callback;

				/**
				* \brief The frame const handler callback.
				*/
				typedef boost::function<void(const_helper<frame_type>)> frame_const_handler_callback;

				/**
				 * \brief Add a filter function.
				 * \param callback The filter function to add.
				 */
				void add_filter(frame_filter_callback callback) {
					m_filters.push_back(callback);
				}

				/**
				 * \brief Add a handler function.
				 * \param callback The handler function to add.
				 */
				void add_handler(frame_handler_callback callback) {
					m_handlers.push_back(callback);
				}

				/**
				* \brief Add a const handler function.
				* \param callback The const handler function to add.
				*/
				void add_const_handler(frame_const_handler_callback callback) {
					m_const_handlers.push_back(callback);
				}

				/**
				 * \brief Get the last helper.
				 * \return The last helper, if any.
				 */
				boost::optional<mutable_helper<frame_type> > get_last_helper() const {
					return m_last_helper;
				}

				/**
				* \brief Get the last const helper.
				* \return The last const helper, if any.
				*/
				boost::optional<const_helper<frame_type> > get_last_const_helper() const {
					return m_last_const_helper;
				}

				/**
				 * \brief Clear the last helper.
				 */
				void clear_last_helper() const {
					m_last_helper = boost::none;
					m_last_const_helper = boost::none;
				}

			protected:

				/**
				 * \brief Do the parsing.
				 * \param buf buffer to parse.
				 */
				void do_parse(boost::asio::const_buffer buf) const;

				/**
				 * \brief Do the parsing.
				 * \param buf buffer to parse.
				 */
				void do_parse(boost::asio::mutable_buffer buf) const;

				/**
				 * \brief Check if the frame has to be handled.
				 * \param helper frame type helper.
				 * \return true if the frame type belongs to the filter.
				 */
				bool filter_frame(const_helper<frame_type> helper) const {
					for (auto&& filter : m_filters)
					{
						if (!filter(helper))
						{
							return false;
						}
					}

					return true;
				}

				/**
				 * \brief Handle the frame by the filter functions.
				 * \param helper frame type helper.
				 */
				void frame_handled(const_helper<frame_type> helper) const {
					m_last_const_helper = helper;

					for (auto&& handler : m_const_handlers)
					{
						handler(*m_last_const_helper);
					}
				}

				/**
				 * \brief Handle the frame by the filter functions.
				 * \param helper frame type helper.
				 */
				void frame_handled(mutable_helper<frame_type> helper) const {
					m_last_helper = helper;

					for (auto&& handler : m_handlers)
					{
						handler(*m_last_helper);
					}
				}

			private:

				std::vector<frame_filter_callback> m_filters;
				std::vector<frame_handler_callback> m_handlers;
				std::vector<frame_const_handler_callback> m_const_handlers;
				mutable boost::optional<mutable_helper<frame_type> > m_last_helper;
				mutable boost::optional<const_helper<frame_type> > m_last_const_helper;
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

				/**
				 * \brief Parse a frame.
				 * \param parent The parent frame.
				 */
				void parse(mutable_helper<typename ParentFilterType::frame_type> parent) const;

			protected:

				bool bridge_filter_frame(const_helper<typename ParentFilterType::frame_type> parent_helper, const_helper<OSIFrameType> helper) const {
					for (auto&& bridge_filter : m_bridge_filters)
					{
						if (!bridge_filter(parent_helper, helper))
						{
							return false;
						}
					}

					return true;
				}

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

				/**
				 * \brief Parse the specified buffer.
				 * \param buf The buffer to parse.
				 */
				void parse(boost::asio::mutable_buffer buf) const;
		};

		/**
		 * \brief A generic filter class.
		 */
		template <typename OSIFrameType, typename ParentFilterType = void>
		class filter;

		/**
		 * \brief Check if a frame is valid.
		 * \param frame The frame.
		 * \return true on success.
		 */
		template <typename OSIFrameType>
		inline bool check_frame(const_helper<OSIFrameType> frame) {
			static_cast<void>(frame);

			return true;
		}

		/**
		 * \brief Check if a frame is valid.
		 * \param frame The frame.
		 * \return true on success.
		 */
		template <typename OSIFrameType>
		inline bool check_frame(mutable_helper<OSIFrameType> frame) {
			return check_frame<OSIFrameType>(const_helper<OSIFrameType>(frame));
		}

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::do_parse(boost::asio::const_buffer buf) const
		{
			try
			{
				const_helper<OSIFrameType> helper(buf);

				if (check_frame(helper))
				{
					if (_base_filter<OSIFrameType>::filter_frame(helper))
					{
						_base_filter<OSIFrameType>::frame_handled(helper);
					}
				}
			}
			catch (std::logic_error&)
			{
			}
		}

		template <typename OSIFrameType>
		inline void _base_filter<OSIFrameType>::do_parse(boost::asio::mutable_buffer buf) const
		{
			try
			{
				mutable_helper<OSIFrameType> helper(buf);

				if (check_frame(helper))
				{
					if (_base_filter<OSIFrameType>::filter_frame(helper))
					{
						_base_filter<OSIFrameType>::frame_handled(helper);
					}
				}
			}
			catch (std::logic_error&)
			{
			}
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline _filter<OSIFrameType, ParentFilterType>::_filter(ParentFilterType& _parent) :
			m_parent(_parent)
		{
			typedef _filter<OSIFrameType, ParentFilterType> filter_type;
			typedef typename ParentFilterType::frame_type parent_frame_type;

			const auto mutable_parse = static_cast<void (filter_type::*)(mutable_helper<parent_frame_type>) const>(&filter_type::parse);
			const auto const_parse = static_cast<void (filter_type::*)(const_helper<parent_frame_type>) const>(&filter_type::parse);

			m_parent.add_handler(boost::bind(mutable_parse, this, _1));
			m_parent.add_const_handler(boost::bind(const_parse, this, _1));

#if 0
			// The previous lines could be simplified using lambdas, like
			// below. But there is a compiler bug in gcc 4.7 which makes this
			// impossible. And we have to support it to be able to backport on
			// Debian wheezy.
			m_parent.add_handler([this](mutable_helper<typename ParentFilterType::frame_type> helper) { parse(helper); });
			m_parent.add_const_handler([this](const_helper<typename ParentFilterType::frame_type> helper) { parse(helper); });
#endif
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
			_base_filter<OSIFrameType>::clear_last_helper();

			if (frame_parent_match<OSIFrameType, typename ParentFilterType::frame_type>(parent_helper))
			{
				try
				{
					const_helper<OSIFrameType> helper(parent_helper.payload());

					if (check_frame(helper))
					{
						if (_base_filter<OSIFrameType>::filter_frame(helper))
						{
							if (bridge_filter_frame(parent_helper, helper))
							{
								_base_filter<OSIFrameType>::frame_handled(helper);
							}
						}
					}
				}
				catch (std::logic_error&)
				{
				}
			}
		}

		template <typename OSIFrameType, typename ParentFilterType>
		inline void _filter<OSIFrameType, ParentFilterType>::parse(mutable_helper<typename ParentFilterType::frame_type> parent_helper) const
		{
			_base_filter<OSIFrameType>::clear_last_helper();

			if (frame_parent_match<OSIFrameType, typename ParentFilterType::frame_type>(parent_helper))
			{
				try
				{
					mutable_helper<OSIFrameType> helper(parent_helper.payload());

					if (check_frame(helper))
					{
						if (_base_filter<OSIFrameType>::filter_frame(helper))
						{
							if (bridge_filter_frame(parent_helper, helper))
							{
								_base_filter<OSIFrameType>::frame_handled(helper);
							}
						}
					}
				}
				catch (std::logic_error&)
				{
				}
			}
		}

		template <typename OSIFrameType>
		inline void _filter<OSIFrameType, void>::parse(boost::asio::const_buffer buf) const
		{
			_base_filter<OSIFrameType>::clear_last_helper();

			_base_filter<OSIFrameType>::do_parse(buf);
		}

		template <typename OSIFrameType>
		inline void _filter<OSIFrameType, void>::parse(boost::asio::mutable_buffer buf) const
		{
			_base_filter<OSIFrameType>::clear_last_helper();

			_base_filter<OSIFrameType>::do_parse(buf);
		}
	}
}

#endif /* ASIOTAP_OSI_FILTER_HPP */


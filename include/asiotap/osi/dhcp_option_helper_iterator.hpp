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
 * \file dhcp_option_helper_iterator.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP option helper iterator class.
 */

#ifndef ASIOTAP_OSI_DHCP_OPTION_HELPER_ITERATOR_HPP
#define ASIOTAP_OSI_DHCP_OPTION_HELPER_ITERATOR_HPP

#include "dhcp_option_helper.hpp"

#include <boost/optional.hpp>

#include <iterator>

namespace asiotap
{
	namespace osi
	{
		struct dhcp_frame;

		template <class HelperTag>
		class dhcp_option_helper_iterator;

		/**
		 * \brief Compare two iterators.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two arguments represent the same iterator.
		 */
		template <class HelperTag>
		bool operator==(const dhcp_option_helper_iterator<HelperTag>& lhs, const dhcp_option_helper_iterator<HelperTag>& rhs);

		/**
		 * \brief Compare two iterators.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two arguments represent different iterators.
		 */
		template <class HelperTag>
		bool operator!=(const dhcp_option_helper_iterator<HelperTag>& lhs, const dhcp_option_helper_iterator<HelperTag>& rhs);

		/**
		 * \brief The DHCP option helper iterator class.
		 */
		template <class HelperTag>
		class dhcp_option_helper_iterator : public std::iterator<std::forward_iterator_tag, dhcp_option_helper<HelperTag> >
		{
			public:

				/**
				 * \brief The helper type.
				 */
				typedef dhcp_option_helper<HelperTag> helper_type;

				/**
				 * \brief Create an empty iterator.
				 */
				dhcp_option_helper_iterator() {}

				/**
				 * \brief Dereference operator.
				 * \return The value.
				 */
				typename dhcp_option_helper_iterator::reference operator*();

				/**
				 * \brief Dereference operator.
				 * \return The value.
				 */
				typename dhcp_option_helper_iterator::pointer operator->();

				/**
				 * \brief Increment the iterator.
				 * \return A reference to this.
				 */
				dhcp_option_helper_iterator& operator++();

				/**
				 * \brief Increment the iterator.
				 * \return The old value.
				 */
				dhcp_option_helper_iterator operator++(int);

			private:

				/**
				 * \brief The buffer type.
				 */
				typedef typename helper_buffer<HelperTag>::type buffer_type;

				dhcp_option_helper_iterator(buffer_type);

				boost::optional<helper_type> m_helper;

				friend bool operator==(const dhcp_option_helper_iterator<HelperTag>& lhs, const dhcp_option_helper_iterator<HelperTag>& rhs)
				{
					if (lhs.m_helper)
					{
						return
							rhs.m_helper &&
							(boost::asio::buffer_cast<const void*>(lhs.m_helper->buffer()) == boost::asio::buffer_cast<const void*>(rhs.m_helper->buffer())) &&
							(boost::asio::buffer_size(lhs.m_helper->buffer()) == boost::asio::buffer_size(rhs.m_helper->buffer()))
							;
					}
					else
					{
						return (!rhs.m_helper);
					}
				}
				friend bool operator!=(const dhcp_option_helper_iterator<HelperTag>& lhs, const dhcp_option_helper_iterator<HelperTag>& rhs)
				{
					return !(lhs == rhs);
				}
				friend class _base_helper_impl<HelperTag, dhcp_frame>;
		};

		template <class HelperTag>
		inline typename dhcp_option_helper_iterator<HelperTag>::reference dhcp_option_helper_iterator<HelperTag>::operator*()
		{
			return *m_helper;
		}

		template <class HelperTag>
		inline typename dhcp_option_helper_iterator<HelperTag>::pointer dhcp_option_helper_iterator<HelperTag>::operator->()
		{
			return &*m_helper;
		}

		template <class HelperTag>
		inline dhcp_option_helper_iterator<HelperTag>& dhcp_option_helper_iterator<HelperTag>::operator++()
		{
			const buffer_type new_buffer = m_helper->buffer() + m_helper->total_length();

			if (boost::asio::buffer_size(new_buffer) > 0)
			{
				m_helper = helper_type(new_buffer);
			}
			else
			{
				m_helper = boost::optional<helper_type>();
			}

			return *this;
		}

		template <class HelperTag>
		inline dhcp_option_helper_iterator<HelperTag> dhcp_option_helper_iterator<HelperTag>::operator++(int)
		{
			dhcp_option_helper_iterator<HelperTag> value = *this;

			++*this;

			return value;
		}

		template <class HelperTag>
		inline dhcp_option_helper_iterator<HelperTag>::dhcp_option_helper_iterator(buffer_type buf) :
			m_helper(boost::asio::buffer_size(buf) > 0 ? helper_type(buf) : boost::optional<helper_type>())
		{
		}
	}
}

#endif /* ASIOTAP_OSI_DHCP_OPTION_HELPER_ITERATOR_HPP */


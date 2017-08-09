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
 * \file dhcp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP helper class.
 */

#ifndef ASIOTAP_OSI_DHCP_HELPER_HPP
#define ASIOTAP_OSI_DHCP_HELPER_HPP

#include "helper.hpp"
#include "dhcp_frame.hpp"
#include "dhcp_option_helper_iterator.hpp"

#include <boost/range.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base dhcp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, dhcp_frame> : public _base_helper<HelperTag, dhcp_frame>
		{
			public:

				/**
				 * \brief The iterator type.
				 */
				typedef dhcp_option_helper_iterator<typename _base_helper_impl::helper_tag> const_iterator;

				/**
				 * \brief Get the magic cookie.
				 * \return The magic cookie.
				 */
				uint32_t magic_cookie() const;

				/**
				 * \brief Get the options begin iterator.
				 * \return An iterator to the first option.
				 */
				const_iterator begin() const;

				/**
				 * \brief Get the options end iterator.
				 * \return An iterator past the last option.
				 */
				const_iterator end() const;

				/**
				 * \brief Get the options buffer.
				 * \return The options.
				 */
				typename _base_helper_impl::buffer_type options() const
				{
					return this->buffer() + sizeof(typename _base_helper_impl<HelperTag, dhcp_frame>::frame_type);
				}

				/**
				* \brief Search for the specified option.
				* \param tag The option tag.
				* \return An iterator to the option, or end() if no such option is found.
				*/
				const_iterator find(dhcp_option::dhcp_option_tag tag) const;

				/**
				* \brief Check if all the options are valid.
				* \return true if all the options are valid, false otherwise.
				*/
				bool check_options() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf);
		};

		/**
		 * \brief The mutable udp helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, dhcp_frame> : public _base_helper_impl<mutable_helper_tag, dhcp_frame>
		{
			public:

				/**
				 * \brief Set the magic cookie.
				 * \param magic_cookie The magic cookie.
				 */
				void set_magic_cookie(uint32_t magic_cookie) const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf);
		};

		template <class HelperTag>
		inline uint32_t _base_helper_impl<HelperTag, dhcp_frame>::magic_cookie() const
		{
			return ntohl(this->frame().magic_cookie);
		}

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator _base_helper_impl<HelperTag, dhcp_frame>::begin() const
		{
			return const_iterator(options());
		}

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator _base_helper_impl<HelperTag, dhcp_frame>::end() const
		{
			return const_iterator();
		}

		template <class HelperTag>
		inline _base_helper_impl<HelperTag, dhcp_frame>::_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
			_base_helper<HelperTag, dhcp_frame>(buf)
		{
		}

		inline void _helper_impl<mutable_helper_tag, dhcp_frame>::set_magic_cookie(uint32_t _magic_cookie) const
		{
			this->frame().magic_cookie = htonl(_magic_cookie);
		}

		inline _helper_impl<mutable_helper_tag, dhcp_frame>::_helper_impl(_helper_impl<mutable_helper_tag, dhcp_frame>::buffer_type buf) :
			_base_helper_impl<mutable_helper_tag, dhcp_frame>(buf)
		{
		}

		// Boost specializations

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator range_begin(_base_helper_impl<HelperTag, dhcp_frame>& x)
		{
			return x.begin();
		}

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator range_begin(const _base_helper_impl<HelperTag, dhcp_frame>& x)
		{
			return x.begin();
		}

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator range_end(_base_helper_impl<HelperTag, dhcp_frame>& x)
		{
			return x.end();
		}

		template <class HelperTag>
		inline typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator range_end(const _base_helper_impl<HelperTag, dhcp_frame>& x)
		{
			return x.end();
		}
	}
}

namespace boost
{
	template <>
	struct range_mutable_iterator<asiotap::osi::const_helper<asiotap::osi::dhcp_frame> >
	{
		typedef asiotap::osi::const_helper<asiotap::osi::dhcp_frame>::const_iterator type;
	};

	template <>
	struct range_const_iterator<asiotap::osi::const_helper<asiotap::osi::dhcp_frame> >
	{
		typedef asiotap::osi::const_helper<asiotap::osi::dhcp_frame>::const_iterator type;
	};

	template <>
	struct range_mutable_iterator<asiotap::osi::mutable_helper<asiotap::osi::dhcp_frame> >
	{
		typedef asiotap::osi::mutable_helper<asiotap::osi::dhcp_frame>::const_iterator type;
	};

	template <>
	struct range_const_iterator<asiotap::osi::mutable_helper<asiotap::osi::dhcp_frame> >
	{
		typedef asiotap::osi::mutable_helper<asiotap::osi::dhcp_frame>::const_iterator type;
	};
}

#endif /* ASIOTAP_OSI_DHCP_HELPER_HPP */


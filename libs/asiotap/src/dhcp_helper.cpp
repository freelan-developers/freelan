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
 * \file dhcp_helper.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP helper class.
 */

#include "osi/dhcp_helper.hpp"

#include <boost/bind.hpp>

#include <algorithm>

namespace asiotap
{
	namespace osi
	{
		namespace
		{
			template <class HelperTag>
			bool has_tag(const dhcp_option_helper<HelperTag> helper, dhcp_option::dhcp_option_tag tag)
			{
				return helper.tag() == tag;
			}
		}

		template <class HelperTag>
		typename _base_helper_impl<HelperTag, dhcp_frame>::const_iterator _base_helper_impl<HelperTag, dhcp_frame>::find(dhcp_option::dhcp_option_tag tag) const
		{
			return std::find_if(this->begin(), this->end(), boost::bind(&has_tag<HelperTag>, _1, tag));
		}

		template <class HelperTag>
		bool _base_helper_impl<HelperTag, dhcp_frame>::check_options() const
		{
			return (std::find_if(this->begin(), this->end(), !boost::bind(&dhcp_option_helper<HelperTag>::is_valid, _1)) == this->end());
		}

		template _base_helper_impl<const_helper_tag, dhcp_frame>::const_iterator _base_helper_impl<const_helper_tag, dhcp_frame>::find(dhcp_option::dhcp_option_tag) const;
		template _base_helper_impl<mutable_helper_tag, dhcp_frame>::const_iterator _base_helper_impl<mutable_helper_tag, dhcp_frame>::find(dhcp_option::dhcp_option_tag) const;
		template bool _base_helper_impl<const_helper_tag, dhcp_frame>::check_options() const;
		template bool _base_helper_impl<mutable_helper_tag, dhcp_frame>::check_options() const;
	}
}

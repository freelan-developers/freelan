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
 * \file tcp_mss_morpher.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A TCP MSS morpher class.
 */

#pragma once

#include "tcp_filter.hpp"
#include "ipv4_helper.hpp"
#include "ipv6_helper.hpp"

#include <boost/optional.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An TCP MSS morpher class.
		 */
		class tcp_mss_morpher
		{
			public:

				/**
				 * \brief Create a TCP MSS morpher.
				 * \brief The max MSS value.
				 */
				tcp_mss_morpher(size_t max_mss) :
					m_max_mss(max_mss)
				{
				}

				/**
				 * \brief Handle a TCP frame.
				 * \param ipv4_helper The IPv4 helper.
				 * \param tcp_helper The TCP helper.
				 */
				void handle(const_helper<ipv4_frame> ipv4_helper, mutable_helper<tcp_frame> tcp_helper);

				/**
				* \brief Handle a TCP frame.
				* \param ipv6_helper The IPv4 helper.
				* \param tcp_helper The TCP helper.
				*/
				void handle(const_helper<ipv6_frame> ipv6_helper, mutable_helper<tcp_frame> tcp_helper);

			private:
				size_t m_max_mss;
		};
	}
}

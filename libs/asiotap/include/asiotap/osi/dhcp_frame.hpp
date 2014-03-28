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
 * \file dhcp_frame.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP frame structure.
 */

#ifndef ASIOTAP_OSI_DHCP_FRAME_HPP
#define ASIOTAP_OSI_DHCP_FRAME_HPP

#include "frame.hpp"

namespace asiotap
{
	namespace osi
	{
#ifdef MSV
#pragma pack(push, 1)
#endif

		/**
		 * \brief The DHCP magic cookie.
		 */
		const uint32_t DHCP_MAGIC_COOKIE = 0x63825363;

		/**
		 * \brief The DHCP discover message.
		 */
		const uint8_t DHCP_DISCOVER_MESSAGE = 0x01;

		/**
		 * \brief The DHCP offer message.
		 */
		const uint8_t DHCP_OFFER_MESSAGE = 0x02;

		/**
		 * \brief The DHCP request message.
		 */
		const uint8_t DHCP_REQUEST_MESSAGE = 0x03;

		/**
		 * \brief The DHCP decline message.
		 */
		const uint8_t DHCP_DECLINE_MESSAGE = 0x04;

		/**
		 * \brief The DHCP acknowledgment message.
		 */
		const uint8_t DHCP_ACKNOWLEDGMENT_MESSAGE = 0x05;

		/**
		 * \brief The DHCP negative acknowledgment message.
		 */
		const uint8_t DHCP_NEGATIVE_ACKNOWLEDGMENT_MESSAGE = 0x06;

		/**
		 * \brief The DHCP release message.
		 */
		const uint8_t DHCP_RELEASE_MESSAGE = 0x07;

		/**
		 * \brief The DHCP informational message.
		 */
		const uint8_t DHCP_INFORMATIONAL_MESSAGE = 0x08;

		/**
		 * \brief A DHCP frame structure.
		 */
		struct dhcp_frame
		{
			uint32_t magic_cookie; /**< The magic cookie. */
		} PACKED;

#ifdef MSV
#pragma pack(pop)
#endif
	}
}

#endif /* ASIOTAP_OSI_DHCP_FRAME_HPP */


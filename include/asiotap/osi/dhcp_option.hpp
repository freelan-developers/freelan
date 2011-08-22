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
 * \file dhcp_option.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A DHCP option class.
 */

#ifndef ASIOTAP_OSI_DHCP_OPTION_HPP
#define ASIOTAP_OSI_DHCP_OPTION_HPP

namespace asiotap
{
	namespace osi
	{
		namespace dhcp_option
		{
			/**
			 * \brief The DHCP option.
			 */
			enum dhcp_option
			{
				pad = 											0x00,
				subnet_mask =								0x01,
				time_offset =								0x02,
				router =										0x03,
				time_server =								0x04,
				name_server = 							0x05,
				domain_name_server =				0x06,
				log_server = 								0x07,
				quote_server =							0x08,
				lpr_server =								0x09,
				impress_serve =							0x0a,
				resource_location_server =	0x0b,
				host_name =									0x0c,
				boot_file_size =						0x0d,
				merit_dump_file =						0x0e,
				domain_name =								0x0f,
				end =												0xff
			};
	}
}

#endif /* ASIOTAP_OSI_DHCP_OPTION_HPP */


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
 * \file tcp_mss_morpher.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A TCP MSS morpher class.
 */

#include "osi/tcp_mss_morpher.hpp"

namespace asiotap
{
	namespace osi
	{
		namespace {
			template <typename OSIHelperType>
			void generic_handle(uint16_t max_mss, OSIHelperType ip_helper, mutable_helper<tcp_frame> tcp_helper) {
				if (tcp_helper.syn_flag()) {
					for (auto option = tcp_helper.first_option(); option.valid(); option = option.next_option()) {
						if (option.kind() == TCP_OPTION_END) {
							break;
						}

						if (option.kind() == TCP_OPTION_MSS) {
							if (option.size() == 4) {
								auto value = option.value();
								uint16_t mss = ntohs(*boost::asio::buffer_cast<uint16_t*>(value));

								if (mss > max_mss) {
									*boost::asio::buffer_cast<uint16_t*>(value) = htons(max_mss);
									tcp_helper.set_checksum(0x0000);
									tcp_helper.set_checksum(tcp_helper.compute_checksum(ip_helper));
								}
							}

							break;
						}
					}
				}
			}
		}

		void tcp_mss_morpher::handle(const_helper<ipv4_frame> ipv4_helper, mutable_helper<tcp_frame> tcp_helper) {
			generic_handle(static_cast<uint16_t>(m_max_mss), ipv4_helper, tcp_helper);
		}

		void tcp_mss_morpher::handle(const_helper<ipv6_frame> ipv6_helper, mutable_helper<tcp_frame> tcp_helper) {
			generic_handle(static_cast<uint16_t>(m_max_mss), ipv6_helper, tcp_helper);
		}
	}
}

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
 * \file icmpv6_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An ICMPv6 helper class.
 */

#pragma once

#include "helper.hpp"
#include "icmpv6_frame.hpp"
#include "ipv6_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base icmpv6 helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, icmpv6_frame> : public _base_helper<HelperTag, icmpv6_frame>
		{
			public:

				/**
				 * \brief Get the type.
				 * \return The type.
				 */
				uint8_t type() const {
					return this->frame().type;
				}

				/**
				 * \brief Get the code.
				 * \return The code.
				 */
				uint8_t code() const {
					return this->frame().code;
				}

				/**
				 * \brief Get the router flag.
				 * \return The router flag.
				 */
				bool router_flag() const {
					return ((this->frame().flags & htonl(0x80000000)) != 0);
				}

				/**
				 * \brief Get the solicited flag.
				 * \return The solicited flag.
				 */
				bool solicited_flag() const {
					return ((this->frame().flags & htonl(0x40000000)) != 0);
				}

				/**
				 * \brief Get the override flag.
				 * \return The override flag.
				 */
				bool override_flag() const {
					return ((this->frame().flags & htonl(0x20000000)) != 0);
				}

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const {
					return this->frame().checksum;
				}

				/**
				 * \brief Compute the checksum.
				 * \param parent_frame The parent frame.
				 * \return The checksum.
				 */
				uint16_t compute_checksum(const_helper<ipv6_frame> parent_frame) const;

				/**
				 * \brief Verify the checksum.
				 * \param parent_frame The parent frame.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum(const_helper<ipv6_frame> parent_frame) const {
					return (compute_checksum(parent_frame) == 0x0000);
				}

				/**
				 * \brief Get the target address.
				 * \return The target address.
				 */
				boost::asio::ip::address_v6 target() const {
					using boost::asio::ip::address_v6;

					address_v6::bytes_type raw;
					std::memcpy(&raw.front(), this->frame().target.s6_addr, raw.size());

					return address_v6(raw);
				}

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + header_length();
				}

				/**
				 * \brief Get the ICMPv6 header length, in bytes.
				 * \return The ICMPv6 header length, in bytes.
				 * \warning Does not contain the options.
				 */
				size_t header_length() const {
					return sizeof(icmpv6_frame);
				}

				/**
				* \brief Get the payload length.
				* \param parent_frame The parent frame.
				* \return The payload length.
				*/
				typename _base_helper_impl::buffer_type payload_length(const_helper<ipv6_frame> parent_frame) const
				{
					return parent_frame.payload_length() - header_length();
				}

		protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
					_base_helper<HelperTag, icmpv6_frame>(buf)
				{}
		};

		/**
		 * \brief The mutable ipv6 helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, icmpv6_frame> : public _base_helper_impl<mutable_helper_tag, icmpv6_frame>
		{
			public:

				/**
				 * \brief Set the type.
				 * \param _type The type.
				 */
				void set_type(uint8_t _type) const {
					this->frame().type = _type;
				}

				/**
				 * \brief Set the code.
				 * \param _code The code.
				 */
				void set_code(uint8_t _code) const {
					this->frame().code = _code;
				}

				/**
				 * \brief Set the checksum.
				 * \param _checksum The checksum.
				 */
				void set_checksum(uint16_t _checksum) const {
					this->frame().checksum = _checksum;
				}

				/**
				 * \brief Reset the flags.
				 */
				void reset_flags() const {
					this->frame().flags = htonl(0x00000000);
				}

				/**
				 * \brief Set the router flag.
				 * \param flag The flag value.
				 */
				void set_router_flag(bool value) const {
					if (value) {
						this->frame().flags |= htonl(0x80000000);
					} else {
						this->frame().flags &= htonl(0x7fffffff);
					}
				}

				/**
				 * \brief Set the solicited flag.
				 * \param flag The flag value.
				 */
				void set_solicited_flag(bool value) const {
					if (value) {
						this->frame().flags |= htonl(0x40000000);
					}
					else {
						this->frame().flags &= htonl(0xbfffffff);
					}
				}

				/**
				 * \brief Set the override flag.
				 * \param flag The flag value.
				 */
				void set_override_flag(bool value) const {
					if (value) {
						this->frame().flags |= htonl(0x20000000);
					}
					else {
						this->frame().flags &= htonl(0xdfffffff);
					}
				}

				/**
				 * \brief Set the target address.
				 * \param _target The target address.
				 */
				void set_target(const boost::asio::ip::address_v6& _target) const
				{
					std::memcpy(this->frame().target.s6_addr, _target.to_bytes().data(), _target.to_bytes().size());
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf) :
					_base_helper_impl<mutable_helper_tag, icmpv6_frame>(buf)
				{}
		};
	}
}

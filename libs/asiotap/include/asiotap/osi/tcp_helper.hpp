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
 * \file tcp_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A TCP helper class.
 */

#pragma once

#include "helper.hpp"
#include "tcp_frame.hpp"

#include "ipv4_helper.hpp"
#include "ipv6_helper.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The base udp helper implementation class.
		 */
		template <class HelperTag>
		class _base_helper_impl<HelperTag, tcp_frame> : public _base_helper<HelperTag, tcp_frame>
		{
			public:

				/**
				 * \brief TCP option.
				 */
				class tcp_option {
					public:
						tcp_option(typename _base_helper_impl::buffer_type buf = typename _base_helper_impl::buffer_type()) :
							m_buf(buf)
						{
						}

						bool valid() const {
							const size_t buf_size = boost::asio::buffer_size(m_buf);

							if (buf_size == 0) {
								return false;
							}

							switch (kind()) {
								case TCP_OPTION_END:
								case TCP_OPTION_NOP:
									return true;
								default:
									break;
							}

							if (buf_size < 1) {
								return false;
							}

							if (size() > buf_size) {
								return false;
							}

							return true;
						}

						uint8_t kind() const {
							return boost::asio::buffer_cast<uint8_t*>(m_buf)[0];
						}

						uint8_t size() const {
							switch (kind()) {
								case TCP_OPTION_END:
								case TCP_OPTION_NOP:
									return 1;
								default:
									return boost::asio::buffer_cast<uint8_t*>(m_buf)[1];
							}
						}

						typename _base_helper_impl::buffer_type value() const {
							const uint8_t _size = size();
							if (_size <= 2) {
								return {};
							} else {
								return boost::asio::buffer(m_buf + 2, _size - 2);
							}
						}

						tcp_option next_option() const {
							if (!valid()) {
								return {};
							} else {
								return { m_buf + size() };
							}
						}

					private:
						typename _base_helper_impl::buffer_type m_buf;
				};

				/**
				 * \brief Get the source port.
				 * \return The source port.
				 */
				uint16_t source() const {
					return ntohs(this->frame().source);
				}

				/**
				 * \brief Get the destination port.
				 * \return The destination port.
				 */
				uint16_t destination() const {
					return ntohs(this->frame().destination);
				}

				/**
				 * \brief Get the sequence number.
				 * \return The sequence number.
				 */
				uint32_t sequence() const {
					return ntohl(this->frame().sequence);
				}

				/**
				 * \brief Get the ack number.
				 * \return The ack number.
				 */
				uint32_t ack() const {
					return ntohl(this->frame().ack);
				}

				/**
				 * \brief Get the offset.
				 * \return The offset.
				 */
				size_t offset() const {
					return ((ntohs(this->frame().offset_flags) & 0xf000) >> 12) * 4;
				}

				/**
				 * \brief Get the urgent flag.
				 * \return The flag's value.
				 */
				bool urgent_flag() const {
					return ((this->frame().offset_flags & htons(0x0020)) != 0);
				}

				/**
				 * \brief Get the ack flag.
				 * \return The flag's value.
				 */
				bool ack_flag() const {
					return ((this->frame().offset_flags & htons(0x0010)) != 0);
				}

				/**
				 * \brief Get the psh flag.
				 * \return The flag's value.
				 */
				bool psh_flag() const {
					return ((this->frame().offset_flags & htons(0x0008)) != 0);
				}

				/**
				 * \brief Get the rst flag.
				 * \return The flag's value.
				 */
				bool rst_flag() const {
					return ((this->frame().offset_flags & htons(0x0004)) != 0);
				}

				/**
				 * \brief Get the syn flag.
				 * \return The flag's value.
				 */
				bool syn_flag() const {
					return ((this->frame().offset_flags & htons(0x0002)) != 0);
				}

				/**
				 * \brief Get the fin flag.
				 * \return The flag's value.
				 */
				bool fin_flag() const {
					return ((this->frame().offset_flags & htons(0x0001)) != 0);
				}

				/**
				 * \brief Get the window.
				 * \return The window.
				 */
				uint16_t window() const {
					return ntohs(this->frame().window);
				}

				/**
				 * \brief Get the checksum.
				 * \return The checksum.
				 */
				uint16_t checksum() const
				{
					return this->frame().checksum;
				}

				/**
				 * \brief Get the pointer.
				 * \return The pointer.
				 */
				uint16_t pointer() const {
					return ntohs(this->frame().pointer);
				}

				/**
				 * \brief Get the options payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type options_payload() const
				{
					return boost::asio::buffer(this->buffer() + sizeof(typename _base_helper_impl<HelperTag, tcp_frame>::frame_type), offset());
				}

				/**
				 * \brief Get the first option.
				 * \return The first option.
				 */
				tcp_option first_option() const {
					return { options_payload() };
				}

				/**
				 * \brief Get the payload buffer.
				 * \return The payload.
				 */
				typename _base_helper_impl::buffer_type payload() const
				{
					return this->buffer() + sizeof(typename _base_helper_impl<HelperTag, tcp_frame>::frame_type) + offset();
				}

				/**
				 * \brief Compute the checksum.
				 * \param parent_frame The parent frame.
				 * \return The checksum.
				 */
				uint16_t compute_checksum(const_helper<ipv4_frame> parent_frame) const;

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
				bool verify_checksum(const_helper<ipv4_frame> parent_frame) const {
					return this->compute_checksum(parent_frame) == 0x0000;
				}

				/**
				 * \brief Verify the checksum.
				 * \param parent_frame The parent frame.
				 * \return true if the checksum is valid.
				 */
				bool verify_checksum(const_helper<ipv6_frame> parent_frame) const {
					return this->compute_checksum(parent_frame) == 0x0000;
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_base_helper_impl(typename _base_helper_impl::buffer_type buf) :
					_base_helper<HelperTag, tcp_frame>(buf)
				{}
		};

		/**
		 * \brief The mutable udp helper implementation class.
		 */
		template <>
		class _helper_impl<mutable_helper_tag, tcp_frame> : public _base_helper_impl<mutable_helper_tag, tcp_frame>
		{
			public:

				/**
				 * \brief Set the source port.
				 * \param _source The source port.
				 */
				void set_source(uint16_t _source) const {
					this->frame().source = htons(_source);
				}

				/**
				 * \brief Set the destination port.
				 * \param _destination The destination port.
				 */
				void set_destination(uint16_t _destination) const {
					this->frame().destination = htons(_destination);
				}

				/**
				 * \brief Set the sequence.
				 * \param _sequence The sequence.
				 */
				void set_sequence(uint32_t _sequence) const {
					this->frame().sequence = htonl(_sequence);
				}

				/**
				 * \brief Set the ack.
				 * \param _ack The ack.
				 */
				void set_ack(uint32_t _ack) const {
					this->frame().ack = htonl(_ack);
				}

				/**
				 * \brief Set the offset.
				 * \param _offset The offset. Must be a multiple of 4, or the result is undefined.
				 */
				void set_offset(size_t _offset) const {
					this->frame().offset_flags = (this->frame().offset_flags & htons(0x0fff)) | htons(static_cast<uint16_t>(_offset / 4) << 12);
				}

				/**
				 * \brief Set the urgent flag.
				 * \param value The new flag's value.
				 */
				void set_urgent_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0020);
					}
					else {
						this->frame().offset_flags &= htons(0xffdf);
					}
				}

				/**
				 * \brief Set the ack flag.
				 * \param value The new flag's value.
				 */
				void set_ack_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0010);
					}
					else {
						this->frame().offset_flags &= htons(0xffef);
					}
				}

				/**
				 * \brief Set the psh flag.
				 * \param value The new flag's value.
				 */
				void set_psh_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0008);
					}
					else {
						this->frame().offset_flags &= htons(0xfff7);
					}
				}

				/**
				 * \brief Set the rst flag.
				 * \param value The new flag's value.
				 */
				void set_rst_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0004);
					}
					else {
						this->frame().offset_flags &= htons(0xfffb);
					}
				}

				/**
				 * \brief Set the syn flag.
				 * \param value The new flag's value.
				 */
				void set_syn_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0002);
					}
					else {
						this->frame().offset_flags &= htons(0xfffd);
					}
				}

				/**
				 * \brief Set the fin flag.
				 * \param value The new flag's value.
				 */
				void set_fin_flag(bool value) const {
					if (value) {
						this->frame().offset_flags |= htons(0x0001);
					}
					else {
						this->frame().offset_flags &= htons(0xfffe);
					}
				}

				/**
				 * \brief Set the window.
				 * \param _window The window.
				 */
				void set_window(uint16_t _window) const {
					this->frame().window = htons(_window);
				}

				/**
				 * \brief Set the checksum.
				 * \param _checksum The checksum.
				 */
				void set_checksum(uint16_t _checksum) const {
					this->frame().checksum = _checksum;
				}

				/**
				 * \brief Set the pointer.
				 * \param _pointer The pointer.
				 */
				void set_pointer(uint16_t _pointer) const {
					this->frame().pointer = htons(_pointer);
				}

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_helper_impl(_helper_impl::buffer_type buf) :
					_base_helper_impl<mutable_helper_tag, tcp_frame>(buf)
				{}
		};
	}
}

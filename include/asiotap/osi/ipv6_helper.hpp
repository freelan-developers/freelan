/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
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
 * \file ipv6_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An IPv6 helper class.
 */

#ifndef ASIOTAP_OSI_IPV6_HELPER_HPP
#define ASIOTAP_OSI_IPV6_HELPER_HPP

#include "helper.hpp"
#include "ipv6_frame.hpp"

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief The const ipv6 helper class.
		 */
		typedef const_helper<ipv6_frame> const_ipv6_helper;

		/**
		 * \brief The mutable ipv6 helper class.
		 */
		typedef mutable_helper<ipv6_frame> mutable_ipv6_helper;

		/**
		 * \brief The const ipv6 helper implementation class.
		 */
		template <>
		class _const_helper_impl<ipv6_frame> : public _base_const_helper<ipv6_frame>
		{
			public:

				/**
				 * \brief Get the version.
				 * \return The version.
				 */
				uint8_t version() const;

				/**
				 * \brief Get the class.
				 * \return The class.
				 */
				uint8_t _class() const;

				/**
				 * \brief Get the label.
				 * \return The label.
				 */
				uint32_t label() const;

				/**
				 * \brief Get the payload length.
				 * \return The payload length.
				 */
				size_t payload_length() const;

				/**
				 * \brief Get the next header.
				 * \return The next header.
				 */
				uint8_t next_header() const;

				/**
				 * \brief Get the hop limit.
				 * \return The hop limit.
				 */
				uint8_t hop_limit() const;

				/**
				 * \brief Get the source address.
				 * \return The source address.
				 */
				boost::asio::ip::address_v6 source() const;

				/**
				 * \brief Get the destination address.
				 * \return The destination address.
				 */
				boost::asio::ip::address_v6 destination() const;

				/**
				 * \brief Get the IPv6 header length, in bytes.
				 * \return The IPv6 header length, in bytes.
				 */
				size_t header_length() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_const_helper_impl(boost::asio::const_buffer buf);
		};

		/**
		 * \brief The mutable ipv6 helper implementation class.
		 */
		template <>
		class _mutable_helper_impl<ipv6_frame> : public _base_mutable_helper<ipv6_frame>
		{
			public:

				/**
				 * \brief Get the version.
				 * \return The version.
				 */
				uint8_t version() const;

				/**
				 * \brief Set the version.
				 * \param version The version.
				 */
				void set_version(uint8_t version) const;

				/**
				 * \brief Get the class.
				 * \return The class.
				 */
				uint8_t _class() const;

				/**
				 * \brief Set the class.
				 * \param _class The class.
				 */
				void set_class(uint8_t _class) const;

				/**
				 * \brief Get the label.
				 * \return The label.
				 */
				uint32_t label() const;

				/**
				 * \brief Set the label.
				 * \param label The label.
				 */
				void set_label(uint32_t label) const;

				/**
				 * \brief Get the payload length.
				 * \return The payload length.
				 */
				size_t payload_length() const;

				/**
				 * \brief Set the payload length.
				 * \param payload_length The payload length.
				 */
				void set_payload_length(size_t payload_length) const;

				/**
				 * \brief Get the next header.
				 * \return The next header.
				 */
				uint8_t next_header() const;

				/**
				 * \brief Set the next header.
				 * \param next_header The next header.
				 */
				void set_next_header(uint8_t next_header) const;

				/**
				 * \brief Get the hop limit.
				 * \return The hop limit.
				 */
				uint8_t hop_limit() const;

				/**
				 * \brief Set hop limit.
				 * \param hop_limit The hop limit.
				 */
				void set_hop_limit(uint8_t hop_limit) const;

				/**
				 * \brief Get the source address.
				 * \return The source address.
				 */
				boost::asio::ip::address_v6 source() const;

				/**
				 * \brief Set the source address.
				 * \param source The source address.
				 */
				void set_source(boost::asio::ip::address_v6 source) const;

				/**
				 * \brief Get the destination address.
				 * \return The destination address.
				 */
				boost::asio::ip::address_v6 destination() const;

				/**
				 * \brief Set the destination address.
				 * \param destination The destination address.
				 */
				void set_destination(boost::asio::ip::address_v6 destination) const;
 
				/**
				 * \brief Get the IPv6 header length, in bytes.
				 * \return The IPv6 header length, in bytes.
				 */
				size_t header_length() const;

			protected:

				/**
				 * \brief Create a helper from a frame type structure.
				 * \param buf The buffer to refer to.
				 */
				_mutable_helper_impl(boost::asio::mutable_buffer buf);
		};

		inline uint8_t _const_helper_impl<ipv6_frame>::version() const
		{
			return (frame().version_class_label & 0xF0000000) >> 28;
		}

		inline uint8_t _const_helper_impl<ipv6_frame>::_class() const
		{
			return (frame().version_class_label & 0x0FF00000) >> 20;
		}

		inline uint32_t _const_helper_impl<ipv6_frame>::label() const
		{
			return (frame().version_class_label & 0x000FFFFF);
		}

		inline size_t _const_helper_impl<ipv6_frame>::payload_length() const
		{
			return ntohs(frame().payload_length);
		}

		inline uint8_t _const_helper_impl<ipv6_frame>::next_header() const
		{
			return frame().next_header;
		}

		inline uint8_t _const_helper_impl<ipv6_frame>::hop_limit() const
		{
			return frame().hop_limit;
		}

		inline boost::asio::ip::address_v6 _const_helper_impl<ipv6_frame>::source() const
		{
			using boost::asio::ip::address_v6;

			address_v6::bytes_type raw;
			std::memcpy(raw.c_array(), frame().source.s6_addr, raw.size());

			return address_v6(raw);
		}

		inline boost::asio::ip::address_v6 _const_helper_impl<ipv6_frame>::destination() const
		{
			using boost::asio::ip::address_v6;

			address_v6::bytes_type raw;
			std::memcpy(raw.c_array(), frame().destination.s6_addr, raw.size());

			return address_v6(raw);
		}

                inline size_t _const_helper_impl<ipv6_frame>::header_length() const
                {
                        return 40;
                }

		inline _const_helper_impl<ipv6_frame>::_const_helper_impl(boost::asio::const_buffer buf) :
			_base_const_helper<ipv6_frame>(buf)
		{
		}

		inline uint8_t _mutable_helper_impl<ipv6_frame>::version() const
		{
			return (frame().version_class_label & 0xF0000000) >> 28;
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_version(uint8_t _version) const
		{
			frame().version_class_label = (frame().version_class_label & 0x0FFFFFFF) | ((_version & 0x0FL) << 28);
		}

		inline uint8_t _mutable_helper_impl<ipv6_frame>::_class() const
		{
			return (frame().version_class_label & 0x0FF00000) >> 20;
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_class(uint8_t __class) const
		{
			frame().version_class_label = (frame().version_class_label & 0xF00FFFFF) | ((__class & 0xFFL) << 20);
		}

		inline uint32_t _mutable_helper_impl<ipv6_frame>::label() const
		{
			return (frame().version_class_label & 0x000FFFFF);
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_label(uint32_t _label) const
		{
			frame().version_class_label = (frame().version_class_label & 0xFFF00000) | (_label & 0x000FFFFFL);
		}

		inline size_t _mutable_helper_impl<ipv6_frame>::payload_length() const
		{
			return ntohs(frame().payload_length);
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_payload_length(size_t _payload_length) const
		{
			frame().payload_length = htons(_payload_length);
		}

		inline uint8_t _mutable_helper_impl<ipv6_frame>::next_header() const
		{
			return frame().next_header;
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_next_header(uint8_t _next_header) const
		{
			frame().next_header = _next_header;
		}

		inline uint8_t _mutable_helper_impl<ipv6_frame>::hop_limit() const
		{
			return frame().hop_limit;
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_hop_limit(uint8_t _hop_limit) const
		{
			frame().hop_limit = _hop_limit;
		}

		inline boost::asio::ip::address_v6 _mutable_helper_impl<ipv6_frame>::source() const
		{
			using boost::asio::ip::address_v6;

			address_v6::bytes_type raw;
			std::memcpy(raw.c_array(), frame().source.s6_addr, raw.size());

			return address_v6(raw);
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_source(boost::asio::ip::address_v6 _source) const
		{
			std::memcpy(frame().source.s6_addr, _source.to_bytes().data(), _source.to_bytes().size());
		}

		inline boost::asio::ip::address_v6 _mutable_helper_impl<ipv6_frame>::destination() const
		{
			using boost::asio::ip::address_v6;

			address_v6::bytes_type raw;
			std::memcpy(raw.c_array(), frame().destination.s6_addr, raw.size());

			return address_v6(raw);
		}

		inline void _mutable_helper_impl<ipv6_frame>::set_destination(boost::asio::ip::address_v6 _destination) const
		{
			std::memcpy(frame().destination.s6_addr, _destination.to_bytes().data(), _destination.to_bytes().size());
		}

		inline _mutable_helper_impl<ipv6_frame>::_mutable_helper_impl(boost::asio::mutable_buffer buf) :
			_base_mutable_helper<ipv6_frame>(buf)
		{
		}

                inline size_t _mutable_helper_impl<ipv6_frame>::header_length() const
                {
                        return 40;
                }
	}
}

#endif /* ASIOTAP_OSI_IPV6_HELPER_HPP */


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
 * \file ethernet_address.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An Ethernet address class.
 */

#ifndef ASIOTAP_OSI_ETHERNET_ADDRESS_HPP
#define ASIOTAP_OSI_ETHERNET_ADDRESS_HPP

#include "ethernet_frame.hpp"

#include <iostream>

#include <boost/array.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief An ethernet address class.
		 */
		class ethernet_address
		{
			public:

				/**
				 * \brief The underlying data type.
				 */
				typedef boost::array<uint8_t, ETHERNET_ADDRESS_SIZE> data_type;

				/**
				 * \brief Get a null address.
				 * \return A null address.
				 */
				static ethernet_address null()
				{
					const data_type data = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

					return ethernet_address(data);
				}

				/**
				 * \brief Get a broadcast address.
				 * \return A broadcast address, that is: ff:ff:ff:ff:ff:ff.
				 */
				static ethernet_address broadcast()
				{
					const data_type data = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

					return ethernet_address(data);
				}

				/**
				 * \brief Creates a default initialized ethernet_address.
				 */
				ethernet_address() {}

				/**
				 * \brief Create an address from its bytes.
				 * \param _data The data.
				 */
				ethernet_address(const data_type& _data) : m_data(_data) {}

				/**
				 * \brief Get the associated data.
				 * \return The associated data.
				 */
				const data_type& data() const
				{
					return m_data;
				}

				/**
				* \brief Get the associated data.
				* \return The associated data.
				*/
				data_type& data()
				{
					return m_data;
				}

		private:

				data_type m_data;
		};

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two addresses are equal.
		 */
		inline bool operator==(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() == rhs.data();
		}

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the two addresses are different.
		 */
		inline bool operator!=(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() != rhs.data();
		}

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs compare lexicographically smaller to rhs.
		 */
		inline bool operator<(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() < rhs.data();
		}

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs compare lexicographically larger to rhs.
		 */
		inline bool operator>(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() > rhs.data();
		}

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs compare lexicographically smaller or equal to rhs.
		 */
		inline bool operator<=(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() <= rhs.data();
		}

		/**
		 * \brief Compare two ethernet addresses.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs compare lexicographically larger or equal to rhs.
		 */
		inline bool operator>=(const ethernet_address& lhs, const ethernet_address& rhs)
		{
			return lhs.data() >= rhs.data();
		}

		/**
		 * \brief Output an ethernet address to a stream.
		 * \param os The output stream.
		 * \param value The value.
		 * \return os.
		 */
		std::ostream& operator<<(std::ostream& os, const ethernet_address& value);

		/**
		 * \brief Input an ethernet address from a stream.
		 * \param is The output stream.
		 * \param value The value.
		 * \return s.
		 *
		 * The ethernet address is expected to be in one of the following formats:
		 *
		 * - ab:cd:ef:01:23:45
		 * - ab-cd-ef-01-23-45
		 */
		std::istream& operator>>(std::istream& is, ethernet_address& value);
	}
}

#endif /* ASIOTAP_ETHERNET_ADDRESS_HPP */


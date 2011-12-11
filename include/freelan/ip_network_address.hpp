/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file ip_network_address.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief IP network address classes.
 */

#ifndef FREELAN_IP_NETWORK_ADDRESS_HPP
#define FREELAN_IP_NETWORK_ADDRESS_HPP

#include <boost/asio.hpp>

namespace freelan
{
	/**
	 * \brief A generic IP network address template class.
	 */
	template <typename AddressType>
	class ip_network_address
	{
		public:

			/**
			 * \brief Get a null network address.
			 * \return A null network address.
			 */
			static ip_network_address null() { return ip_network_address(); }

			/**
			 * \brief The address type.
			 */
			typedef AddressType address_type;

			/**
			 * \brief Create an IP network address.
			 */
			ip_network_address() : m_address(), m_prefix_length(0) {};

			/**
			 * \brief Create an IP network address.
			 * \param _address The address.
			 * \param _prefix_length The prefix length.
			 */
			ip_network_address(const address_type& _address, unsigned int _prefix_length) : m_address(_address), m_prefix_length(_prefix_length) {};

			/**
			 * \brief Get the address.
			 * \return The address.
			 */
			const address_type& address() const
			{
				return m_address;
			}

			/**
			 * \brief Get the prefix length.
			 * \return The prefix length.
			 */
			unsigned int prefix_length() const
			{
				return m_prefix_length;
			}

		private:

			address_type m_address;
			unsigned int m_prefix_length;

			template<typename OtherAddressType> friend bool operator==(const ip_network_address<OtherAddressType>& lhs, const ip_network_address<OtherAddressType>& rhs);
	};

	/**
	 * \brief Write an network address to an output stream.
	 * \tparam AddressType The address type.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const ip_network_address<AddressType>& value);

	/**
	 * \brief Read a network address from an input stream.
	 * \tparam AddressType The address type.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	template <typename AddressType>
	std::istream& operator>>(std::istream& is, ip_network_address<AddressType>& value);

	/**
	 * \brief Compare two network addresses.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two network addresses are equal.
	 */
	template <typename AddressType>
	inline bool operator==(const ip_network_address<AddressType>& lhs, const ip_network_address<AddressType>& rhs)
	{
		return (lhs.address() == rhs.address()) && (lhs.m_prefix_length == rhs.m_prefix_length);
	}

	/**
	 * \brief Compare two network addresses.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two network addresses are different.
	 */
	template <typename AddressType>
	inline bool operator!=(const ip_network_address<AddressType>& lhs, const ip_network_address<AddressType>& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * \brief The IPv4 instantiation.
	 */
	typedef ip_network_address<boost::asio::ip::address_v4> ipv4_network_address;

	/**
	 * \brief The IPv6 instantiation.
	 */
	typedef ip_network_address<boost::asio::ip::address_v6> ipv6_network_address;
}

#endif /* FREELAN_IP_NETWORK_ADDRESS_HPP */


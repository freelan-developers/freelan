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
 * \file ip_endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief IP endpoint classes.
 */

#pragma once

#include <string>
#include <set>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>

namespace asiotap
{
	/**
	 * \brief A generic IP endpoint template class.
	 */
	template <typename AddressType>
	class base_ip_endpoint
	{
		public:

			/**
			 * \brief Get a null IP endpoint.
			 * \return A null IP endpoint.
			 */
			static base_ip_endpoint null()
			{
				return base_ip_endpoint();
			}

			/**
			 * \brief The address type.
			 */
			typedef AddressType address_type;

			/**
			 * \brief The resolver type.
			 */
			typedef boost::asio::ip::udp::resolver resolver;

			/**
			 * \brief The handler type.
			 */
			typedef boost::function<void(const boost::system::error_code&, resolver::iterator)> handler;

			/**
			 * \brief Create an IP endpoint.
			 */
			base_ip_endpoint() {};

			/**
			 * \brief Create an IP endpoint.
			 * \param _address The address.
			 */
			base_ip_endpoint(const address_type& _address) : m_address(_address) {};

			/**
			 * \brief Create an IP endpoint.
			 * \param _address The address.
			 * \param _port The port number.
			 */
			base_ip_endpoint(const address_type& _address, boost::optional<uint16_t> _port) : m_address(_address), m_port(_port) {};

			/**
			 * \brief Check if the instance is null.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return (*this == null());
			}

			/**
			 * \brief Check if the ip_endpoint has a null IP address.
			 * \return true if the ip_endpoint has a null IP address.
			 */
			bool has_null_address() const
			{
				return address().is_unspecified();
			}

			/**
			 * \brief Get the address.
			 * \return The address.
			 */
			const address_type& address() const
			{
				return m_address;
			}

			/**
			 * \brief Check if the ip_endpoint contains a valid port number.
			 * \return true if the ip_endpoint contains a valid port number.
			 */
			bool has_port() const
			{
				return static_cast<bool>(m_port);
			}

			/**
			 * \brief Get the port number.
			 * \return The port number.
			 * \warning If has_port() is false, calling port() is undefined behavior.
			 */
			uint16_t port() const
			{
				return *m_port;
			}

		private:

			address_type m_address;
			boost::optional<uint16_t> m_port;

			friend bool operator<(const base_ip_endpoint& lhs, const base_ip_endpoint& rhs)
			{
				if (lhs.m_address == rhs.m_address)
				{
					if (lhs.has_port())
					{
						return (rhs.has_port() && (lhs.port() < rhs.port()));
					}
					else
					{
						return rhs.has_port();
					}
				}
				else
				{
					return (lhs.m_address < rhs.m_address);
				}
			}

			friend bool operator==(const base_ip_endpoint& lhs, const base_ip_endpoint& rhs)
			{
				return (lhs.address() == rhs.address()) && (lhs.m_port == rhs.m_port);
			}

			friend bool operator!=(const base_ip_endpoint& lhs, const base_ip_endpoint& rhs)
			{
				return !(lhs == rhs);
			}
	};

	/**
	 * \brief Perform a host resolution on the endpoint.
	 * \tparam AddressType The address type.
	 * \param ep The endpoint to resolve.
	 * \param resolver The resolver to use.
	 * \param protocol The protocol to use.
	 * \param flags The flags to use for the resolution.
	 * \param default_service The default service to use.
	 * \return The endpoint.
	 */
	template <typename AddressType>
	inline boost::asio::ip::udp::endpoint resolve(const base_ip_endpoint<AddressType>& ep, typename base_ip_endpoint<AddressType>::resolver& resolver, typename base_ip_endpoint<AddressType>::resolver::protocol_type protocol, typename base_ip_endpoint<AddressType>::resolver::query::flags flags, const std::string& default_service)
	{
		static_cast<void>(resolver);
		static_cast<void>(protocol);
		static_cast<void>(flags);

		if (ep.has_port())
		{
			return boost::asio::ip::udp::endpoint(ep.address(), ep.port());
		}
		else
		{
			return boost::asio::ip::udp::endpoint(ep.address(), boost::lexical_cast<uint16_t>(default_service));
		}
	}

	/**
	 * \brief Perform an asynchronous host resolution on the endpoint.
	 * \tparam AddressType The address type.
	 * \param ep The endpoint to resolve.
	 * \param resolver The resolver to use.
	 * \param protocol The protocol to use.
	 * \param flags The flags to use for the resolution.
	 * \param default_service The default service to use.
	 * \param handler The handler.
	 */
	template <typename AddressType>
	inline void async_resolve(const base_ip_endpoint<AddressType>& ep, typename base_ip_endpoint<AddressType>::resolver& resolver, typename base_ip_endpoint<AddressType>::resolver::protocol_type protocol, typename base_ip_endpoint<AddressType>::resolver::query::flags flags, const std::string& default_service, typename base_ip_endpoint<AddressType>::handler handler)
	{
		static_cast<void>(protocol);
		typename base_ip_endpoint<AddressType>::resolver::query query(ep.address().to_string(), ep.has_port() ? boost::lexical_cast<std::string>(ep.port()) : default_service, flags | boost::asio::ip::resolver_query_base::numeric_host);

		resolver.async_resolve(query, handler);
	}

	/**
	 * \brief Write an endpoint to an output stream.
	 * \tparam AddressType The address type.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	template <typename AddressType>
	std::ostream& operator<<(std::ostream& os, const base_ip_endpoint<AddressType>& value);

	/**
	 * \brief Read an endpoint from an input stream.
	 * \tparam AddressType The address type.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	template <typename AddressType>
	std::istream& operator>>(std::istream& is, base_ip_endpoint<AddressType>& value);

	/**
	 * \brief The IPv4 instantiation.
	 */
	typedef base_ip_endpoint<boost::asio::ip::address_v4> ipv4_endpoint;

	/**
	 * \brief The IPv6 instantiation.
	 */
	typedef base_ip_endpoint<boost::asio::ip::address_v6> ipv6_endpoint;

	/**
	 * \brief The generic IP endpoint.
	 */
	typedef boost::variant<ipv4_endpoint, ipv6_endpoint> ip_endpoint;

	/**
	 * \brief An IP endpoint set.
	 */
	typedef std::set<ip_endpoint> ip_endpoint_set;

	/**
	* \brief The generic IP address.
	*
	* Needs to be a class and not a typedef or ADL won't work.
	*/
	class ip_address {
		private:
			class to_generic_ip_address_visitor : public boost::static_visitor<boost::asio::ip::address>
			{
				public:
					template <typename AddressType>
					result_type operator()(const AddressType& addr) const {
						return addr;
					}
			};

		public:
			typedef boost::variant<boost::asio::ip::address_v4, boost::asio::ip::address_v6> value_type;

			ip_address() {}

			template <typename AnyType>
			ip_address(const AnyType& _value) : m_value(_value) {}

			template <typename AnyType>
			ip_address& operator=(const AnyType& _value) {
				m_value = _value;

				return *this;
			}

			boost::asio::ip::address value() const {
				return boost::apply_visitor(to_generic_ip_address_visitor(), m_value);
			}

		private:
			friend bool operator<(const ip_address& lhs, const ip_address& rhs) {
				return (lhs.m_value < rhs.m_value);
			}

			friend bool operator==(const ip_address& lhs, const ip_address& rhs) {
				return (lhs.m_value == rhs.m_value);
			}

			value_type m_value;
	};

	/**
	 * \brief Read an IP address from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, ip_address& value);

	/**
	 * \brief Write an IP address to an output stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const ip_address& value);

	/**
	 * \brief An IP address set.
	 */
	typedef std::set<ip_address> ip_address_set;

	/**
	 * \brief Write an IP address set to an output stream.
	 * \param os The output stream.
	 * \param values The values.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const ip_address_set& values);
}

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
 * \file ip_endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief IP endpoint classes.
 */

#ifndef FREELAN_IP_ENDPOINT_HPP
#define FREELAN_IP_ENDPOINT_HPP

#include <string>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

namespace freelan
{
	/**
	 * \brief A generic IP endpoint template class.
	 */
	template <typename AddressType>
	class ip_endpoint
	{
		public:

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
			typedef boost::function<void (const boost::system::error_code&, resolver::iterator)> handler;

			/**
			 * \brief Create an IP endpoint.
			 * \param address The address.
			 */
			ip_endpoint(const address_type& _address) : m_address(_address) {};

			/**
			 * \brief Create an IP endpoint.
			 * \param address The address.
			 * \param port The port number.
			 */
			ip_endpoint(const address_type& _address, uint16_t _port) : m_address(_address), m_port(_port) {};

			/**
			 * \brief Get the address.
			 * \return The address.
			 */
			const address_type& address() const { return m_address; }

			/**
			 * \brief Check if the ip_endpoint contains a valid port number.
			 * \return true if the ip_endpoint contains a valid port number.
			 */
			bool has_port() const { return static_cast<bool>(m_port); }

			/**
			 * \brief Get the port number.
			 * \return The port number.
			 * \warning If has_port() is false, calling port() is undefined behavior.
			 */
			uint16_t port() const { return *m_port; }

		private:

			address_type m_address;
			boost::optional<uint16_t> m_port;
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
	inline boost::asio::ip::udp::endpoint resolve(const ip_endpoint<AddressType>& ep, typename ip_endpoint<AddressType>::resolver& resolver, typename ip_endpoint<AddressType>::resolver::protocol_type protocol, typename ip_endpoint<AddressType>::resolver::query::flags flags, const std::string& default_service)
	{
		(void)resolver;
		(void)protocol;
		(void)flags;

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
	inline void async_resolve(const ip_endpoint<AddressType>& ep, typename ip_endpoint<AddressType>::resolver& resolver, typename ip_endpoint<AddressType>::resolver::protocol_type protocol, typename ip_endpoint<AddressType>::resolver::query::flags flags, const std::string& default_service, typename ip_endpoint<AddressType>::handler handler)
	{
		try
		{
			boost::asio::ip::udp::endpoint result = resolve(ep, resolver, protocol, flags, default_service);

			typename ip_endpoint<AddressType>::resolver::iterator it = ip_endpoint<AddressType>::resolver::iterator::create(result, result.address().to_string(), boost::lexical_cast<std::string>(result.port()));

			handler(boost::system::error_code(), it);
		}
		catch (boost::system::system_error& ex)
		{
			typename ip_endpoint<AddressType>::resolver::iterator it;

			handler(ex.code(), it);
		}
	}

	/**
	 * \brief The IPv4 instantiation.
	 */
	typedef ip_endpoint<boost::asio::ip::address_v4> ipv4_endpoint;

	/**
	 * \brief The IPv6 instantiation.
	 */
	typedef ip_endpoint<boost::asio::ip::address_v6> ipv6_endpoint;
}

#endif /* FREELAN_IP_ENDPOINT_HPP */


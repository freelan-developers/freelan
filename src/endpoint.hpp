/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A endpoint class.
 */

#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

/**
 * \brief A base endpoint class.
 */
class endpoint
{
	public:

		/**
		 * \brief Virtual destructor.
		 */
		virtual ~endpoint();

		/**
		 * \brief The protocol type.
		 */
		typedef boost::asio::ip::udp::resolver::query::protocol_type protocol_type;

		/**
		 * \brief The flags type.
		 */
		typedef boost::asio::ip::udp::resolver::query::flags flags_type;

		/**
		 * \brief Base service type.
		 */
		typedef std::string base_service_type;

		/**
		 * \brief The Boost ASIO endpoint type.
		 */
		typedef boost::asio::ip::udp::endpoint ep_type;

		/**
		 * \brief Get a Boost ASIO endpoint.
		 * \param protocol The protocol to use.
		 * \param flags The flags to use for the resolution.
		 * \param default_service The default service to use.
		 * \return The Boost ASIO endpoint.
		 */
		virtual ep_type to_boost_asio_endpoint(protocol_type protocol, flags_type flags, const base_service_type& default_service) = 0;
};

/**
 * \brief A generic IP endpoint template class.
 */
template <typename AddressType>
class ip_endpoint : public endpoint
{
	public:

		/**
		 * \brief The address type.
		 */
		typedef AddressType address_type;

		/**
		 * \brief The base port type.
		 */
		typedef uint16_t base_port_type;

		/**
		 * \brief The port type.
		 */
		typedef boost::optional<base_port_type> port_type;

		/**
		 * \brief Create an IPv4 endpoint.
		 * \param address The address component.
		 * \param port The port component.
		 */
		ip_endpoint(const address_type& address, port_type port = port_type());

		/**
		 * \brief Get a Boost ASIO endpoint.
		 * \param protocol The protocol to use.
		 * \param flags The flags to use for the resolution.
		 * \param default_service The default service to use.
		 * \return The Boost ASIO endpoint.
		 */
		ep_type to_boost_asio_endpoint(protocol_type protocol, flags_type flags, const base_service_type& default_service);

	private:

		address_type m_address;
		port_type m_port;
};

/**
 * \brief The IPv4 instantiation.
 */
typedef ip_endpoint<boost::asio::ip::address_v4> ipv4_endpoint;

/**
 * \brief The IPv6 instantiation.
 */
typedef ip_endpoint<boost::asio::ip::address_v6> ipv6_endpoint;

/**
 * \brief A hostname endpoint class.
 */
class hostname_endpoint : public endpoint
{
	public:

		/**
		 * \brief The host type.
		 */
		typedef std::string hostname_type;

		/**
		 * \brief The service type.
		 */
		typedef boost::optional<base_service_type> service_type;

		/**
		 * \brief Create a hostname endpoint.
		 * \param host The host component.
		 * \param service The service component.
		 */
		hostname_endpoint(const hostname_type& address, const service_type& service = service_type());

		/**
		 * \brief Get a Boost ASIO endpoint.
		 * \param protocol The protocol to use.
		 * \param flags The flags to use for the resolution.
		 * \param default_service The default service to use.
		 * \return The Boost ASIO endpoint.
		 */
		ep_type to_boost_asio_endpoint(protocol_type protocol, flags_type flags, const base_service_type& default_service);

	private:

		hostname_type m_hostname;
		service_type m_service;
};

inline endpoint::~endpoint() {}

template <typename AddressType>
inline ip_endpoint<AddressType>::ip_endpoint(const address_type& address, port_type port) :
	m_address(address),
	m_port(port)
{
}

template <typename AddressType>
inline typename ip_endpoint<AddressType>::ep_type ip_endpoint<AddressType>::to_boost_asio_endpoint(protocol_type, flags_type, const base_service_type& default_service)
{
	if (m_port)
	{
		return ep_type(m_address, *m_port);
	} else
	{
		return ep_type(m_address, boost::lexical_cast<base_port_type>(default_service));
	}
}

inline hostname_endpoint::hostname_endpoint(const hostname_type& host, const service_type& service) :
	m_hostname(host),
	m_service(service)
{
}

#endif /* ENDPOINT_HPP */


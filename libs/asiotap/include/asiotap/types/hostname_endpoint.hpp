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
 * \file hostname_endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A hostname endpoint class.
 */

#ifndef ASIOTAP_HOSTNAME_ENDPOINT_HPP
#define ASIOTAP_HOSTNAME_ENDPOINT_HPP

#include <string>

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace asiotap
{
	/**
	 * \brief A hostname endpoint class.
	 */
	class hostname_endpoint
	{
		public:

			/**
			 * \brief Get a null hostname endpoint.
			 * \return A null hostname endpoint.
			 */
			static hostname_endpoint null()
			{
				return hostname_endpoint();
			}

			/**
			 * \brief The resolver type.
			 */
			typedef boost::asio::ip::udp::resolver resolver;

			/**
			 * \brief The handler type.
			 */
			typedef boost::function<void (const boost::system::error_code&, resolver::iterator)> handler;

			/**
			 * \brief Create a hostname endpoint.
			 * \param _hostname The hostname component.
			 * \param _service The service component. An empty service indicates that the default service value should be used.
			 */
			hostname_endpoint(const std::string& _hostname = "", const std::string& _service = "") :
				m_hostname(_hostname),
				m_service(_service)
			{
			}

			/**
			 * \brief Check if the instance is null.
			 * \return true if the instance is null.
			 */
			bool is_null() const
			{
				return (*this == null());
			}

			/**
			 * \brief Get the hostname.
			 * \return The hostname.
			 */
			const std::string& hostname() const
			{
				return m_hostname;
			}

			/**
			 * \brief Get the service.
			 * \return The service.
			 */
			const std::string& service() const
			{
				return m_service;
			}

		private:

			std::string m_hostname;
			std::string m_service;

			friend bool operator<(const hostname_endpoint& lhs, const hostname_endpoint& rhs)
			{
				if (lhs.m_hostname == rhs.m_hostname)
				{
					return (lhs.m_service < rhs.m_service);
				}
				else
				{
					return (lhs.m_hostname < rhs.m_hostname);
				}
			}

			friend bool operator==(const hostname_endpoint& lhs, const hostname_endpoint& rhs)
			{
				return (lhs.m_hostname == rhs.m_hostname) && (lhs.m_service == rhs.m_service);
			}

			friend bool operator!=(const hostname_endpoint& lhs, const hostname_endpoint& rhs)
			{
				return !(lhs == rhs);
			}
	};

	/**
	 * \brief Perform a host resolution on the endpoint.
	 * \param ep The endpoint to resolve.
	 * \param resolver The resolver to use.
	 * \param protocol The protocol to use.
	 * \param flags The flags to use for the resolution.
	 * \param default_service The default service to use.
	 * \return The endpoint.
	 */
	inline boost::asio::ip::udp::endpoint resolve(const hostname_endpoint& ep, hostname_endpoint::resolver& resolver, hostname_endpoint::resolver::protocol_type protocol, hostname_endpoint::resolver::query::flags flags, const std::string& default_service)
	{
		hostname_endpoint::resolver::query query(protocol, ep.hostname(), ep.service().empty() ? default_service : ep.service(), flags);

		return *resolver.resolve(query);
	}

	/**
	 * \brief Perform an asynchronous host resolution on the endpoint.
	 * \param ep The endpoint to resolve.
	 * \param resolver The resolver to use.
	 * \param protocol The protocol to use.
	 * \param flags The flags to use for the resolution.
	 * \param default_service The default service to use.
	 * \param handler The handler.
	 */
	template <typename ResolveHandler>
	inline void async_resolve(const hostname_endpoint& ep, hostname_endpoint::resolver& resolver, hostname_endpoint::resolver::protocol_type protocol, hostname_endpoint::resolver::query::flags flags, const std::string& default_service, ResolveHandler handler)
	{
		hostname_endpoint::resolver::query query(protocol, ep.hostname(), ep.service().empty() ? default_service : ep.service(), flags);

		resolver.async_resolve(query, handler);
	}

	/**
	 * \brief Write an endpoint to an output stream.
	 * \param os The output stream.
	 * \param value The value.
	 * \return os.
	 */
	std::ostream& operator<<(std::ostream& os, const hostname_endpoint& value);

	/**
	 * \brief Read an endpoint from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, hostname_endpoint& value);
}

#endif /* ASIOTAP_HOSTNAME_ENDPOINT_HPP */


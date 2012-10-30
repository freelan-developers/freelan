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
 * \file endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An endpoint type.
 */

#ifndef FREELAN_ENDPOINT_HPP
#define FREELAN_ENDPOINT_HPP

#include <iostream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>

#include "hostname_endpoint.hpp"
#include "ip_endpoint.hpp"

namespace freelan
{
	/**
	 * \brief The endpoint type.
	 */
	typedef boost::variant<hostname_endpoint, ipv4_endpoint, ipv6_endpoint> endpoint;

	/**
	 * \brief A visitor that resolves endpoints.
	 */
	class endpoint_resolve_visitor : public boost::static_visitor<boost::asio::ip::udp::endpoint>
	{
		public:

			/**
			 * \brief The resolver type.
			 */
			typedef boost::asio::ip::udp::resolver resolver;

			/**
			 * \brief Create a new endpoint_resolve_visitor.
			 * \param _resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 */
			endpoint_resolve_visitor(resolver& _resolver, resolver::query::protocol_type protocol, resolver::query::flags flags, const std::string& default_service) :
				m_resolver(_resolver),
				m_protocol(protocol),
				m_flags(flags),
				m_default_service(default_service)
			{
			}


			/**
			 * \brief Resolve the specified endpoint.
			 * \tparam T The type of the endpoint.
			 * \param ep The endpoint.
			 * \return The resolved endpoint.
			 */
			template <typename T>
			result_type operator()(const T& ep) const
			{
				return resolve(ep, m_resolver, m_protocol, m_flags, m_default_service);
			}

		private:

			resolver& m_resolver;
			resolver::query::protocol_type m_protocol;
			resolver::query::flags m_flags;
			std::string m_default_service;
	};

	/**
	 * \brief A visitor that resolves endpoints asynchronously.
	 */
	class endpoint_async_resolve_visitor : public boost::static_visitor<>
	{
		public:

			/**
			 * \brief The resolver type.
			 */
			typedef boost::asio::ip::udp::resolver resolver;

			/**
			 * \brief The handler type.
			 */
			typedef boost::function<void (const boost::system::error_code&, resolver::iterator)> handler;

			/**
			 * \brief Create a new endpoint_async_resolve_visitor.
			 * \param _resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 * \param _handler The handler to use.
			 */
			endpoint_async_resolve_visitor(resolver& _resolver, resolver::query::protocol_type protocol, resolver::query::flags flags, const std::string& default_service, handler _handler) :
				m_resolver(_resolver),
				m_protocol(protocol),
				m_flags(flags),
				m_default_service(default_service),
				m_handler(_handler)
			{
			}

			/**
			 * \brief Resolve the specified endpoint.
			 * \tparam T The type of the endpoint.
			 * \param ep The endpoint.
			 * \return The resolved endpoint.
			 */
			template <typename T>
			void operator()(const T& ep) const
			{
				return async_resolve(ep, m_resolver, m_protocol, m_flags, m_default_service, m_handler);
			}

		private:

			resolver& m_resolver;
			resolver::query::protocol_type m_protocol;
			resolver::query::flags m_flags;
			std::string m_default_service;
			handler m_handler;
	};

	/**
	 * \brief Read an endpoint from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, endpoint& value);

	/**
	 * \brief Compare two endpoints.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if the two endpoints are different.
	 */
	inline bool operator!=(const endpoint& lhs, const endpoint& rhs)
	{
		return !(lhs == rhs);
	}
}

#endif /* FREELAN_ENDPOINT_HPP */


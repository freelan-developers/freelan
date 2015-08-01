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
 * \file endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief An endpoint type.
 */

#ifndef ASIOTAP_ENDPOINT_HPP
#define ASIOTAP_ENDPOINT_HPP

#include <iostream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>

#include "hostname_endpoint.hpp"
#include "ip_endpoint.hpp"

namespace asiotap
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
			typedef boost::asio::ip::udp::resolver resolver_type;

			/**
			 * \brief The handler type.
			 */
			typedef boost::function<void (const boost::system::error_code&, resolver_type::iterator)> handler_type;

		private:

			class resolver_handler
			{
				public:
					resolver_handler(boost::shared_ptr<resolver_type> _resolver, handler_type _handler) :
						m_resolver(_resolver),
						m_handler(_handler)
					{}

					void operator()(const boost::system::error_code& ec, resolver_type::iterator it)
					{
						m_handler(ec, it);
					}

				private:
					boost::shared_ptr<resolver_type> m_resolver;
					handler_type m_handler;
			};

		public:

			/**
			 * \brief Create a new endpoint_async_resolve_visitor.
			 * \param _resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 * \param _handler The handler to use.
			 */
			endpoint_async_resolve_visitor(boost::shared_ptr<resolver_type> _resolver, resolver_type::query::protocol_type protocol, resolver_type::query::flags flags, const std::string& default_service, handler_type _handler) :
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
				return async_resolve(ep, *m_resolver, m_protocol, m_flags, m_default_service, resolver_handler(m_resolver, m_handler));
			}

		private:

			boost::shared_ptr<resolver_type> m_resolver;
			resolver_type::query::protocol_type m_protocol;
			resolver_type::query::flags m_flags;
			std::string m_default_service;
			handler_type m_handler;
	};

	/**
	 * \brief A visitor that adds a default port number to a endpoint that doesn't have one.
	 */
	class default_port_endpoint_visitor : public boost::static_visitor<endpoint>
	{
		public:

			/**
			 * \brief Create the visitor with a default port number.
			 * \param default_port
			 */
			default_port_endpoint_visitor(uint16_t default_port) :
				m_default_port(default_port)
			{
			}

			/**
			 * \brief Get the endpoint with a default endpoint if needed.
			 * \tparam AddressType The address type.
			 * \param ep The endpoint.
			 * \return The new endpoint.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_endpoint<AddressType>& ep) const
			{
				if (ep.has_port())
				{
					return ep;
				}
				else
				{
					return base_ip_endpoint<AddressType>(ep.address(), m_default_port);
				}
			}

			/**
			 * \brief Get the endpoint with a default endpoint if needed.
			 * \param ep The endpoint.
			 * \return The new endpoint.
			 */
			result_type operator()(const hostname_endpoint& ep) const
			{
				if (!ep.service().empty())
				{
					return ep;
				}
				else
				{
					return hostname_endpoint(ep.hostname(), boost::lexical_cast<std::string>(m_default_port));
				}
			}

		private:

			uint16_t m_default_port;
	};

	/**
	 * \brief A visitor that fills the IP address part of a endpoint if is null.
	 */
	template <typename AddressType>
	class default_ip_endpoint_visitor : public boost::static_visitor<endpoint>
	{
		public:

			/**
			 * \brief Create the visitor with a default IP address.
			 * \param default_ip
			 */
			default_ip_endpoint_visitor(const AddressType& default_ip) :
				m_default_ip(default_ip)
			{
			}

			/**
			 * \brief Get the endpoint with a default endpoint if needed.
			 * \tparam AddressType The address type.
			 * \param ep The endpoint.
			 * \return The new endpoint.
			 */
			result_type operator()(const base_ip_endpoint<AddressType>& ep) const
			{
				if (ep.has_null_address())
				{
					if (ep.has_port())
					{
						return base_ip_endpoint<AddressType>(m_default_ip, ep.port());
					}
					else
					{
						return base_ip_endpoint<AddressType>(m_default_ip);
					}
				}
				else
				{
					return ep;
				}
			}

			/**
			 * \brief Get the endpoint with a default endpoint if needed.
			 * \tparam AddressType The address type.
			 * \param ep The endpoint.
			 * \return The new endpoint.
			 */
			template <typename AddressType2>
			result_type operator()(const base_ip_endpoint<AddressType2>& ep) const
			{
				return ep;
			}

			/**
			 * \brief Get the endpoint with a default endpoint if needed.
			 * \param ep The endpoint.
			 * \return The new endpoint.
			 */
			result_type operator()(const hostname_endpoint& ep) const
			{
				return ep;
			}

		private:

			AddressType m_default_ip;
	};

	/**
	 * \brief A visitor that checks if the endpoint is complete.
	 */
	class is_endpoint_complete_visitor : public boost::static_visitor<bool>
	{
		public:

			/**
			 * \brief Check if the endpoint is complete.
			 * \tparam AddressType The address type.
			 * \param ep The endpoint.
			 * \return The validity state.
			 */
			template <typename AddressType>
			result_type operator()(const base_ip_endpoint<AddressType>& ep) const
			{
				return (!ep.has_null_address() && ep.has_port());
			}

			/**
			 * \brief Check if the endpoint is complete.
			 * \tparam AddressType The address type.
			 * \param ep The endpoint.
			 * \return The validity state.
			 */
			result_type operator()(const hostname_endpoint& ep) const
			{
				return (!ep.service().empty());
			}
	};

	/**
	 * \brief Read an endpoint from an input stream.
	 * \param is The input stream.
	 * \param value The value.
	 * \return is.
	 */
	std::istream& operator>>(std::istream& is, endpoint& value);

// Note: this operator is defined in boost variant as of version 1.58. Keeping it around will
// introduce overload resolution ambiguity.
#if BOOST_VERSION < 105800
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
#endif

	/**
	 * \brief Get an endpoint with a default port.
	 * \param ep The endpoint.
	 * \param default_port The default port.
	 * \return The endpoint with the specified default port if it hadn't one yet.
	 */
	inline endpoint get_default_port_endpoint(const endpoint& ep, uint16_t default_port)
	{
		return boost::apply_visitor(default_port_endpoint_visitor(default_port), ep);
	}

	/**
	 * \brief Get an endpoint with a default IP.
	 * \param ep The endpoint.
	 * \param default_ip The default ip.
	 * \return The endpoint with the specified default port if it hadn't one yet.
	 */
	template <typename AddressType>
	inline endpoint get_default_ip_endpoint(const endpoint& ep, const AddressType& default_ip)
	{
		return boost::apply_visitor(default_ip_endpoint_visitor<AddressType>(default_ip), ep);
	}

	/**
	 * \brief Get an endpoint with a default IP.
	 * \param ep The endpoint.
	 * \param default_ip The default ip.
	 * \return The endpoint with the specified default port if it hadn't one yet.
	 */
	inline endpoint get_default_ip_endpoint(const endpoint& ep, const boost::asio::ip::address& default_ip)
	{
		if (default_ip.is_v4())
		{
			return boost::apply_visitor(default_ip_endpoint_visitor<boost::asio::ip::address_v4>(default_ip.to_v4()), ep);
		}
		else
		{
			return boost::apply_visitor(default_ip_endpoint_visitor<boost::asio::ip::address_v6>(default_ip.to_v6()), ep);
		}
	}

	/**
	 * \brief Check if an endpoint is complete.
	 * \param ep The endpoint.
	 * \param default_ip The default ip.
	 * \return The endpoint completion state.
	 */
	inline bool is_endpoint_complete(const endpoint& ep)
	{
		return boost::apply_visitor(is_endpoint_complete_visitor(), ep);
	}
}

#endif /* ASIOTAP_ENDPOINT_HPP */

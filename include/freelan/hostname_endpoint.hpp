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
 * \file hostname_endpoint.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A hostname endpoint class.
 */

#ifndef FREELAN_HOSTNAME_ENDPOINT_HPP
#define FREELAN_HOSTNAME_ENDPOINT_HPP

#include "endpoint.hpp"

namespace freelan
{
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
			 * \brief Perform a host resolution on the endpoint.
			 * \param resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 */
			endpoint::ep_type resolve(boost::asio::ip::udp::resolver& resolver, protocol_type protocol, flags_type flags, const base_service_type& default_service);

			/**
			 * \brief Perform an asynchronous host resolution on the endpoint.
			 * \param resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 * \param handler The handler.
			 */
			void async_resolve(boost::asio::ip::udp::resolver& resolver, protocol_type protocol, flags_type flags, const base_service_type& default_service, endpoint::handler_type handler);

		private:

			hostname_type m_hostname;
			service_type m_service;
	};

	inline hostname_endpoint::hostname_endpoint(const hostname_type& host, const service_type& service) :
		m_hostname(host),
		m_service(service)
	{
	}
}

#endif /* FREELAN_HOSTNAME_ENDPOINT_HPP */


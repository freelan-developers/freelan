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
 * \brief An endpoint class.
 */

#ifndef FREELAN_ENDPOINT_HPP
#define FREELAN_ENDPOINT_HPP

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace freelan
{
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
			 * \brief The handler type.
			 */
			typedef boost::function<void (const boost::system::error_code&, boost::asio::ip::udp::resolver::iterator)> handler_type;

			/**
			 * \brief Perform a host resolution on the endpoint.
			 * \param resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 */
			virtual ep_type resolve(boost::asio::ip::udp::resolver& resolver, protocol_type protocol, flags_type flags, const base_service_type& default_service) = 0;

			/**
			 * \brief Perform an asynchronous host resolution on the endpoint.
			 * \param resolver The resolver to use.
			 * \param protocol The protocol to use.
			 * \param flags The flags to use for the resolution.
			 * \param default_service The default service to use.
			 * \param handler The handler.
			 */
			virtual void async_resolve(boost::asio::ip::udp::resolver& resolver, protocol_type protocol, flags_type flags, const base_service_type& default_service, handler_type handler) = 0;

		protected:

			virtual std::ostream& write(std::ostream&) const = 0;

			friend std::ostream& operator<<(std::ostream&, const endpoint&);
	};

	std::ostream& operator<<(std::ostream&, const endpoint&);

	inline endpoint::~endpoint() {}

	inline std::ostream& operator<<(std::ostream& os, const endpoint& ep)
	{
		return ep.write(os);
	}
}

#endif /* FREELAN_ENDPOINT_HPP */


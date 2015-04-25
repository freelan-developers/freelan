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
 * \file routes_message.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The routes messages exchanged by the peers.
 */

#ifndef FREELAN_ROUTES_MESSAGE_HPP
#define FREELAN_ROUTES_MESSAGE_HPP

#include <boost/optional.hpp>

#include <asiotap/types/ip_route.hpp>
#include <asiotap/types/ip_endpoint.hpp>

#include "message.hpp"

namespace freelan
{
	/**
	 * \brief A routes message.
	 */
	class routes_message : public message
	{
		public:

			/**
			 * \brief The version typedef.
			 */
			typedef uint32_t version_type;

			/**
			 * \brief Write a routes message to a buffer.
			 * \param buf The buffer to write to.
			 * \param buf_len The length of buf.
			 * \param version The version.
			 * \param routes The routes.
			 * \return The count of bytes written.
			 */
			static size_t write(void* buf, size_t buf_len, version_type version, const asiotap::ip_route_set& routes, const asiotap::ip_address_set& dns_servers);

			/**
			 * \brief Get the version.
			 * \return The version.
			 */
			version_type version() const;

			/**
			 * \brief Get the routes.
			 * \return The routes.
			 */
			const asiotap::ip_route_set& routes() const;

			/**
			* \brief Get the DNS servers.
			* \return The DNS servers.
			*/
			const asiotap::ip_address_set& dns_servers() const;

			/**
			 * \brief Create a routes_message and map it on a buffer.
			 * \param buf The buffer.
			 * \param buf_len The buffer length.
			 *
			 * If the mapping fails, a std::runtime_error is thrown.
			 */
			routes_message(const void* buf, size_t buf_len);

			/**
			 * \brief Create a routes_message from a message.
			 * \param message The message.
			 */
			routes_message(const message& message);

		private:

			struct routes_and_dns_servers {
				asiotap::ip_route_set routes;
				asiotap::ip_address_set dns_servers;
			};

			void read_and_cache_results() const;

			mutable boost::optional<routes_and_dns_servers> m_results;
	};
}

#endif /* FREELAN_ROUTES_MESSAGE_HPP */

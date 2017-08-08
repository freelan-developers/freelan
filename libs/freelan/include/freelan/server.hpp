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
 * \file server.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan server class.
 */

#pragma once

#include "os.hpp"

#ifdef USE_MONGOOSE

#include "configuration.hpp"

#include <map>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <fscp/logger.hpp>
#include <fscp/presentation_store.hpp>

#include <asiotap/types/endpoint.hpp>

#include <mongooseplus/mongooseplus.hpp>

namespace freelan
{
	class web_server : public mongooseplus::routed_web_server
	{
		public:
			typedef boost::function<bool (const std::string& username, const std::string& password, const std::string& remote_host, uint16_t remote_post)> authentication_handler_type;

			web_server(fscp::logger& _logger, const freelan::server_configuration& configuration, authentication_handler_type authentication_handler);

		protected:
			route_type& register_authenticated_route(route_type&& route);

			template <typename... Types>
			route_type& register_authenticated_route(Types&&... values)
			{
				return register_authenticated_route(route_type(std::forward<Types>(values)...));
			}

			mongooseplus::routed_web_server::request_result handle_request(mongooseplus::request&) override;
			mongooseplus::routed_web_server::request_result handle_http_error(mongooseplus::request&) override;

		private:
			struct client_information_type
			{
				fscp::presentation_store presentation;
				boost::posix_time::ptime expiration_timestamp;
				std::set<asiotap::endpoint> endpoints;

				bool has_expired() const
				{
					const auto now = boost::posix_time::microsec_clock::universal_time();

					return (expiration_timestamp < now);
				}

				void expires_from_now(const boost::posix_time::time_duration& duration)
				{
					const auto now = boost::posix_time::microsec_clock::universal_time();

					expiration_timestamp = now + duration;
				}
			};

			client_information_type* get_client_information(mongooseplus::request&);

			fscp::logger& m_logger;
			authentication_handler_type m_authentication_handler;
			std::map<std::string, client_information_type> m_client_information_map;
	};
}

#endif

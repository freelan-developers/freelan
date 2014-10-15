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
 * \file server.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan server class.
 */

#include "server.hpp"

#include <boost/lexical_cast.hpp>

#include <cassert>

namespace freelan
{
	using kfather::value_cast;

	namespace
	{
		class session_type : public mongooseplus::generic_session, public mongooseplus::basic_session_type
		{
			public:
				session_type(const std::string& _session_id, const std::string& _username) :
					mongooseplus::generic_session(_session_id),
					basic_session_type(_username)
				{
				}
		};

		class external_authentication_handler : public mongooseplus::basic_authentication_handler
		{
			public:

				external_authentication_handler(fscp::logger& _logger, web_server::authentication_handler_type auth_handler) :
					mongooseplus::basic_authentication_handler("freelan"),
					m_logger(_logger),
					m_authentication_handler(auth_handler)
				{}

			protected:

				bool authenticate_from_username_and_password(mongooseplus::request& req, const std::string& username, const std::string& password) const override
				{
					if (!m_authentication_handler)
					{
						m_logger(fscp::log_level::warning) << "No authentication script defined ! Failing authentication for user \"" << username << "\".";

						return false;
					}

					if (!m_authentication_handler(username, password, req.remote(), req.remote_port()))
					{
						m_logger(fscp::log_level::warning) << "Authentication failed for user \"" << username << "\".";

						return false;
					}

					m_logger(fscp::log_level::information) << "Authentication succeeded for user \"" << username << "\".";

					const auto session = req.get_session<session_type>();

					if (!session || (session->username() != username))
					{
						req.set_session<session_type>(username);
					}

					return true;
				}

			private:
				fscp::logger& m_logger;
				web_server::authentication_handler_type m_authentication_handler;
		};
	}

	web_server::web_server(fscp::logger& _logger, const freelan::server_configuration& configuration, authentication_handler_type authentication_handler) :
		m_logger(_logger),
		m_authentication_handler(authentication_handler)
	{
		m_logger(fscp::log_level::debug) << "Web server's listen endpoint set to " << configuration.listen_on << ".";
		set_option("listening_port", boost::lexical_cast<std::string>(configuration.listen_on));

		// Routes
		register_authenticated_route("/", [this](mongooseplus::request& req) {
			m_logger(fscp::log_level::debug) << "Requested root.";

			const auto json = req.json();
			req.send_json(json);

			return request_result::handled;
		});
	}

	web_server::route_type& web_server::register_authenticated_route(route_type&& route)
	{
		return register_route(route).set_authentication_handler<external_authentication_handler>(m_logger, m_authentication_handler);
	}

	web_server::request_result web_server::handle_request(mongooseplus::request& req)
	{
		if (m_logger.level() <= fscp::log_level::debug)
		{
			m_logger(fscp::log_level::information) << "Web server - Received " << req.request_method() << " request from " << req.remote() << " for " << req.uri() << " (" << req.content_size() << " byte(s) content).";
			m_logger(fscp::log_level::debug) << "--- Headers follow ---";

			for (auto&& header : req.get_headers())
			{
				m_logger(fscp::log_level::debug) << header.key() << ": " << header.value();
			}

			m_logger(fscp::log_level::debug) << "--- End of headers ---";
		}

		return mongooseplus::routed_web_server::handle_request(req);
	}

	web_server::request_result web_server::handle_http_error(mongooseplus::request& req)
	{
		m_logger(fscp::log_level::warning) << "Web server - Sending back " << req.status_code() << " to " << req.remote() << ".";

		return mongooseplus::routed_web_server::handle_http_error(req);
	}
}

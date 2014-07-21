/*
 * libmongooseplus - A lightweight C++ wrapper around the libmongoose
 * library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libmongooseplus.
 *
 * libmongooseplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libmongooseplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libmongooseplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file mongooseplus.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The global mongooseplus include file.
 */

#include "mongooseplus.hpp"

#include "mongoose.h"

#include <cassert>

namespace mongooseplus
{
	namespace
	{
		void mg_destroy_server_simple(mg_server* server) {
			mg_destroy_server(&server);
		}
	}

	web_server::~web_server() {}

	void web_server::stop()
	{
		m_is_running = false;
	}

	struct web_server::underlying_server_type
	{
		static int event_handler(struct mg_connection* conn, enum mg_event ev)
		{
			assert(conn->server_param);

			web_server& ws = *static_cast<web_server*>(conn->server_param);

			const auto result = event_handler_simple(ws, conn, ev);

			switch (result)
			{
				case web_server::request_result::handled:
					return MG_TRUE;
				case web_server::request_result::ignored:
					return MG_FALSE;
				case web_server::request_result::expect_more:
					return MG_MORE;
			}

			assert(false);

			return MG_FALSE;
		}

		static web_server::request_result event_handler_simple(web_server& ws, struct mg_connection* conn, enum mg_event ev)
		{
			web_server::connection connection(conn);

			switch (ev)
			{
				case MG_AUTH:
					return ws.handle_auth(connection);
				case MG_REQUEST:
					return ws.handle_request(connection);
				case MG_POLL:
					return ws.handle_poll(connection);
				case MG_HTTP_ERROR:
					return ws.handle_http_error(connection);
				case MG_CLOSE:
					return ws.handle_close(connection);
				default:
					return web_server::request_result::ignored;
			}
		}

		explicit underlying_server_type(web_server* parent) :
			server(mg_create_server(parent, event_handler), mg_destroy_server_simple)
		{
		}

		std::unique_ptr<mg_server, void(*)(mg_server*)> server;
	};

	web_server::web_server() :
		m_server(new underlying_server_type(this)),
		m_is_running(false)
	{}

	void web_server::run(int poll_period)
	{
		m_is_running = true;

		while (m_is_running)
		{
			mg_poll_server(m_server->server.get(), poll_period);
		}
	}

	void web_server::set_option(const std::string& name, const std::string& value)
	{
		const char* result = ::mg_set_option(m_server->server.get(), name.c_str(), value.c_str());

		if (result != NULL)
		{
			throw std::runtime_error(std::string(result));
		}
	}

	web_server::connection::connection(mg_connection* _connection) :
		m_connection(_connection)
	{
		assert(_connection);		
	}

	std::string web_server::connection::uri() const
	{
		return m_connection->uri;
	}

	boost::optional<std::string> web_server::connection::get_header(const std::string& name) const
	{
		const char* const header = mg_get_header(m_connection, name.c_str());

		if (header)
		{
			return std::string(header);
		}

		return boost::none;
	}

	std::string web_server::connection::request_method() const
	{
		return m_connection->request_method;
	}

	std::string web_server::connection::http_version() const
	{
		return m_connection->http_version;
	}

	std::string web_server::connection::query_string() const
	{
		return m_connection->query_string;
	}

	const char* web_server::connection::content() const
	{
		return m_connection->content;
	}

	size_t web_server::connection::content_size() const
	{
		return m_connection->content_len;
	}

	boost::asio::ip::address web_server::connection::local_ip() const
	{
		return boost::asio::ip::address::from_string(m_connection->local_ip);
	}

	uint16_t web_server::connection::local_port() const
	{
		return m_connection->local_port;
	}

	boost::asio::ip::address web_server::connection::remote_ip() const
	{
		return boost::asio::ip::address::from_string(m_connection->remote_ip);
	}

	uint16_t web_server::connection::remote_port() const
	{
		return m_connection->remote_port;
	}

	void web_server::connection::set_user_param(void* user_param)
	{
		m_connection->connection_param = user_param;
	}

	void* web_server::connection::get_user_param() const
	{
		return m_connection->connection_param;
	}
}

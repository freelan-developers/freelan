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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <cassert>

namespace mongooseplus
{
	namespace
	{
		void mg_destroy_server_simple(mg_server* server)
		{
			mg_destroy_server(&server);
		}

		std::string ip_port_to_string(const boost::asio::ip::address& address, uint16_t port)
		{
			std::ostringstream oss;

			if (address.is_v6())
			{
				oss << "[" << address << "]:" << port;
			}
			else
			{
				oss << address << ":" << port;
			}

			return oss.str();
		}

		template <typename SequenceType, typename Predicate>
		SequenceType filter(const SequenceType& input, Predicate predicate)
		{
			SequenceType result(input.size());
			const auto it = std::copy_if(input.begin(), input.end(), result.begin(), predicate);
			result.resize(std::distance(result.begin(), it));

			return result;
		}

		template <typename SequenceType>
		std::vector<const typename SequenceType::value_type*> to_pointer_list(const SequenceType& input)
		{
			std::vector<const typename SequenceType::value_type*> result(input.size());
			const auto it = std::transform(input.begin(), input.end(), result.begin(), [](const typename SequenceType::value_type& value){ return &value; });
			result.resize(std::distance(result.begin(), it));

			return result;
		}
	}

	template <typename SequenceType>
	std::string header_type::flatten_list(const SequenceType& values)
	{
		std::ostringstream oss;

		bool first = true;

		for (auto&& value : values)
		{
			if (first)
			{
				oss << value;
				first = false;
			}
			else
			{
				oss << "," << value;
			}
		}

		return oss.str();
	}

	std::vector<std::string> header_type::unflatten_list(const std::string& value)
	{
		std::vector<std::string> items;
		boost::split(items, value, boost::is_any_of(","));

		for (auto&& item : items)
		{
			boost::trim(item);
		}

		return items;
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

	boost::optional<header_type> web_server::connection::get_header(const std::string& key) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (value)
		{
			return header_type(key, value);
		}

		return boost::none;
	}

	header_type web_server::connection::get_header(const std::string& key, const std::string& default_value) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (!value)
		{
			return header_type(key, default_value);
		}

		return header_type(key, value);
	}

	header_type web_server::connection::get_header(const std::string& key, const std::vector<std::string>& default_values) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (!value)
		{
			return header_type(key, default_values);
		}

		return header_type(key, value);
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

	int web_server::connection::status_code() const
	{
		return m_connection->status_code;
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

	std::string web_server::connection::local() const
	{
		return ip_port_to_string(local_ip(), local_port());
	}

	boost::asio::ip::address web_server::connection::remote_ip() const
	{
		return boost::asio::ip::address::from_string(m_connection->remote_ip);
	}

	uint16_t web_server::connection::remote_port() const
	{
		return m_connection->remote_port;
	}

	std::string web_server::connection::remote() const
	{
		return ip_port_to_string(remote_ip(), remote_port());
	}

	void web_server::connection::set_user_param(void* user_param)
	{
		m_connection->connection_param = user_param;
	}

	void* web_server::connection::get_user_param() const
	{
		return m_connection->connection_param;
	}

	void web_server::connection::send_status_code(int status_code)
	{
		mg_send_status(m_connection, status_code);
	}

	void web_server::connection::send_header(const header_type& header)
	{
		mg_send_header(m_connection, header.key().c_str(), header.value().c_str());
	}

	void web_server::connection::send_data(const void* data, size_t data_len)
	{
		mg_send_data(m_connection, data, data_len);
	}

	void web_server::connection::write(const void* buf, size_t buf_len)
	{
		mg_write(m_connection, buf, buf_len);
	}

	void web_server::connection::set_from_error(const http_error& ex)
	{
		send_status_code(static_cast<int>(ex.code().value()));
	}

	bool routed_web_server::route_type::url_matches(const web_server::connection& conn) const
	{
		return (std::regex_match(conn.uri(), url_regex));
	}

	bool routed_web_server::route_type::request_method_matches(const web_server::connection& conn) const
	{
		if (!request_methods.empty())
		{
			if (request_methods.find(conn.request_method()) == request_methods.end())
			{
				return false;
			}
		}

		return true;
	}

	bool routed_web_server::route_type::content_type_matches(const web_server::connection& conn) const
	{
		if (!content_types.empty())
		{
			if (content_types.find(conn.get_header("content-type", "text/html").value()) == content_types.end())
			{
				return false;
			}
		}

		return true;
	}

	void routed_web_server::register_route(const route_type& route)
	{
		m_routes.push_back(route);
	}

	routed_web_server::request_result routed_web_server::handle_request(connection& conn)
	{
		try
		{
			const route_type* const route = get_route(conn);

			if (route)
			{
				const auto result = route->function(conn);

				if (result == request_result::handled)
				{
					if (conn.status_code() == 0)
					{
						conn.send_status_code(200);
						conn.send_data("", 0);
					}
				}

				return result;
			}
		}
		catch (const http_error& ex)
		{
			conn.set_from_error(ex);

			if (const header_list_type* const headers = boost::get_error_info<headers_error_info>(ex))
			{
				conn.send_headers(*headers);
			}

			conn.send_data("", 0);

			return request_result::handled;
		}

		return web_server::handle_request(conn);
	}

	const routed_web_server::route_type* routed_web_server::get_route(const connection& conn)
	{
		const auto routes = to_pointer_list(m_routes);
		const auto matched_routes = filter(routes, [&conn](const route_type* route){ return route->url_matches(conn); });

		if (!matched_routes.empty())
		{
			const auto method_matched_routes = filter(matched_routes, [&conn](const route_type* route){ return route->request_method_matches(conn); });

			if (method_matched_routes.empty())
			{
				throw http_error(mongooseplus_error::http_405_method_not_allowed) << headers_error_info(
					{
						{"Allow", matched_routes.front()->request_methods}
					}
				);
			}

			const auto content_matched_routes = filter(method_matched_routes, [&conn](const route_type* route){ return route->content_type_matches(conn); });

			if (content_matched_routes.empty())
			{
				throw http_error(mongooseplus_error::http_406_not_acceptable);
			}

			return content_matched_routes.front();
		}

		return nullptr;
	}
}

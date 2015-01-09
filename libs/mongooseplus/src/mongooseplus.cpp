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
#include <boost/date_time/time_facet.hpp>

#include <cryptoplus/base64.hpp>
#include <cryptoplus/hash/message_digest.hpp>
#include <cryptoplus/random/random.hpp>

#include <kfather/parser.hpp>
#include <kfather/formatter.hpp>

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

	boost::optional<std::string> header_type::value(const std::string& _key) const
	{
		std::string result;
		result.resize(256);

		const int len = mg_parse_header(value().c_str(), _key.c_str(), &result[0], result.size());

		if (len > 0)
		{
			result.resize(len);

			return result;
		}

		return boost::none;
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

	void session_handler_type::clear_expired()
	{
		const boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();

		for (auto&& session_it = m_sessions.begin(); session_it != m_sessions.end();)
		{
			if (session_it->second->has_expired(now))
			{
				session_it = m_sessions.erase(session_it);
			}
			else
			{
				++session_it;
			}
		}
	}

	std::string session_handler_type::generate_session_id() const
	{
		const auto random_bytes = cryptoplus::random::get_random_bytes(32).to_string();
		m_last_session_id = cryptoplus::base64_encode(cryptoplus::hash::message_digest(cryptoplus::buffer(m_last_session_id + random_bytes), EVP_sha256()));

		return m_last_session_id;
	}

	web_server::~web_server() {}

	void web_server::stop()
	{
		m_is_running = false;
	}

	struct web_server::underlying_server_type
	{
		static int event_handler(struct mg_connection* connp, enum mg_event ev)
		{
			assert(connp->server_param);

			web_server& ws = *static_cast<web_server*>(connp->server_param);

			const auto result = event_handler_simple(ws, connp, ev);

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

		static web_server::request_result event_handler_simple(web_server& ws, struct mg_connection* connp, enum mg_event ev)
		{
			request req(ws, connp);

			switch (ev)
			{
				case MG_AUTH:
					return ws.handle_auth(req);
				case MG_REQUEST:
					ws.prepare_request(req);
					return ws.handle_request(req);
				case MG_POLL:
					return ws.handle_poll(req);
				case MG_HTTP_ERROR:
					return ws.handle_http_error(req);
				case MG_CLOSE:
					return ws.handle_close(req);
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

	boost::shared_ptr<generic_session> session_handler_type::read_session(const request& req) const
	{
		const auto header = req.get_header("cookie");

		if (header)
		{
			const auto session_id = header->value("session_id");

			if (session_id)
			{
				const auto session_it = m_sessions.find(*session_id);

				if (session_it != m_sessions.end())
				{
					return session_it->second;
				}
			}
		}

		return boost::shared_ptr<generic_session>();
	}

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

	void web_server::set_certificate_and_private_key(cryptoplus::x509::certificate cert, cryptoplus::pkey::pkey private_key)
	{
		assert(cert);
		assert(private_key);

		const int result = ::mg_set_certificate_and_private_key(m_server->server.get(), cert.raw(), private_key.raw());

		switch (result)
		{
			case 0:
				return;
			case 1:
				throw std::runtime_error("SSL not enabled");
			case 2:
				throw std::runtime_error("Unable to create a SSL context");
			case 3:
				throw std::runtime_error("Unable to load the certificate in the SSL context");
			case 4:
				throw std::runtime_error("Unable to load the private key in the SSL context");
			default:
				throw std::runtime_error("Unknown error");
		}
	}

	void web_server::prepare_request(request& req)
	{
		session_handler().clear_expired();

		boost::shared_ptr<generic_session> session = session_handler().read_session(req);

		if (!session)
		{
			session = handle_session_required(req);
		}
		else
		{
			session->expires_in(boost::posix_time::minutes(5));
		}

		req.set_session(session);
	}

	request::request(web_server& _web_server, mg_connection* _connection) :
		m_connection(_connection),
		m_web_server(_web_server)
	{
		assert(_connection);
	}

	std::string request::uri() const
	{
		return m_connection->uri;
	}

	header_list_type request::get_headers() const
	{
		size_t index = 0;
		header_list_type result;

		std::generate_n(std::back_inserter(result), m_connection->num_headers, [this, &index] () {
			const auto raw_header = m_connection->http_headers[index++];

			return header_type(raw_header.name, raw_header.value);
		});

		return result;
	}

	boost::optional<header_type> request::get_header(const std::string& key) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (value)
		{
			return header_type(key, value);
		}

		return boost::none;
	}

	header_type request::get_header(const std::string& key, const std::string& default_value) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (!value)
		{
			return header_type(key, default_value);
		}

		return header_type(key, value);
	}

	header_type request::get_header(const std::string& key, const std::vector<std::string>& default_values) const
	{
		const char* const value = mg_get_header(m_connection, key.c_str());

		if (!value)
		{
			return header_type(key, default_values);
		}

		return header_type(key, value);
	}

	std::string request::request_method() const
	{
		return m_connection->request_method;
	}

	std::string request::http_version() const
	{
		return m_connection->http_version;
	}

	std::string request::query_string() const
	{
		return m_connection->query_string;
	}

	int request::status_code() const
	{
		return m_connection->status_code;
	}

	std::string request::content_type() const
	{
		return get_header("content-type", "text/html").value();
	}

	const char* request::content() const
	{
		return m_connection->content;
	}

	size_t request::content_size() const
	{
		return m_connection->content_len;
	}

	kfather::value_type request::json() const
	{
		if (content_type() != "application/json")
		{
			throw http_error(mongooseplus_error::http_406_not_acceptable);
		}

		kfather::parser parser;
		kfather::value_type result;

		const char* error_token = nullptr;

		if (!parser.parse(result, content(), content_size(), &error_token))
		{
			const size_t position = error_token - content();

			std::ostringstream oss;

			oss << "Cannot parse JSON: ";

			if (position >= content_size())
			{
				oss << "unexpected end of stream at character " << position;
			}
			else
			{
				oss << "invalid character '" << *error_token << "' at position " << position;
			}

			throw http_error(mongooseplus_error::http_400_bad_request) << error_content_error_info(oss.str());
		}

		return result;
	}

	boost::asio::ip::address request::local_ip() const
	{
		return boost::asio::ip::address::from_string(m_connection->local_ip);
	}

	uint16_t request::local_port() const
	{
		return m_connection->local_port;
	}

	std::string request::local() const
	{
		return ip_port_to_string(local_ip(), local_port());
	}

	boost::asio::ip::address request::remote_ip() const
	{
		return boost::asio::ip::address::from_string(m_connection->remote_ip);
	}

	uint16_t request::remote_port() const
	{
		return m_connection->remote_port;
	}

	std::string request::remote() const
	{
		return ip_port_to_string(remote_ip(), remote_port());
	}

	void request::set_user_param(void* user_param)
	{
		m_connection->connection_param = user_param;
	}

	void* request::get_user_param() const
	{
		return m_connection->connection_param;
	}

	void request::send_status_code(int _status_code)
	{
		mg_send_status(m_connection, _status_code);
	}

	void request::send_header(const header_type& header)
	{
		mg_send_header(m_connection, header.key().c_str(), header.value().c_str());
	}

	void request::send_header(header_type&& header)
	{
		mg_send_header(m_connection, header.key().c_str(), header.value().c_str());
	}

	void request::send_session()
	{
		boost::shared_ptr<generic_session> session = get_session();

		if (session)
		{
			const auto date_format = "%a, %d %b %Y %H:%M:%S GMT";

			static std::locale locale = std::locale(std::cout.getloc(), new boost::posix_time::time_facet(date_format));

			std::ostringstream oss;
			oss.imbue(locale);
			oss << "session_id=" << session->session_id() << "; Expires=" << session->expiration_date() << "; HttpOnly";

			send_header("set-cookie", oss.str());
		}
	}

	void request::send_data(const void* data, size_t data_len)
	{
		mg_send_data(m_connection, data, static_cast<int>(data_len));
	}

	void request::send_json(const kfather::value_type& _json)
	{
		send_header("content-type", "application/json");
		send_data(kfather::compact_formatter().format(_json));
	}

	void request::write(const void* buf, size_t buf_len)
	{
		mg_write(m_connection, buf, static_cast<int>(buf_len));
	}

	void request::set_from_error(const http_error& ex)
	{
		send_status_code(static_cast<int>(ex.code().value()));

		if (const header_list_type* const headers = boost::get_error_info<headers_error_info>(ex))
		{
			send_headers(*headers);
		}

		if (const std::string* const error_content = boost::get_error_info<error_content_error_info>(ex))
		{
			send_data(*error_content);
		}
		else
		{
			send_data("", 0);
		}
	}

	bool basic_authentication_handler::authenticate_from_header(request& req, const header_type& header) const
	{
		std::vector<std::string> items;
		boost::split(items, header.value(), boost::is_any_of(" "));

		if (items.size() != 2)
		{
			return false;
		}

		for (auto&& item : items)
		{
			boost::trim(item);
		}

		if (items[0] != scheme())
		{
			return false;
		}

		const std::string decoded_value = cryptoplus::base64_decode(items[1]).to_string();
		const size_t separator_index = decoded_value.find_first_of(':');
		const std::string username = decoded_value.substr(0, separator_index);
		const std::string password = (separator_index != std::string::npos) ? decoded_value.substr(separator_index + 1) : "";

		return authenticate_from_username_and_password(req, username, password);
	}

	bool basic_authentication_handler::authenticate_from_session(request&, boost::shared_ptr<generic_session> session) const
	{
		return static_cast<bool>(boost::dynamic_pointer_cast<basic_session_type>(session));
	}

	void basic_authentication_handler::raise_authentication_error() const
	{
		throw http_error(mongooseplus_error::http_401_unauthorized) << headers_error_info({
			{"www-authenticate", scheme() + " realm=" + m_realm}
		}) << error_content_error_info("Unauthorized");
	}

	bool routed_web_server::route_type::url_matches(const request& req) const
	{
		return (std::regex_match(req.uri(), url_regex));
	}

	bool routed_web_server::route_type::request_method_matches(const request& req) const
	{
		if (!request_methods.empty())
		{
			if (request_methods.find(req.request_method()) == request_methods.end())
			{
				return false;
			}
		}

		return true;
	}

	bool routed_web_server::route_type::content_type_matches(const request& req) const
	{
		if (!content_types.empty())
		{
			if (content_types.find(req.content_type()) == content_types.end())
			{
				return false;
			}
		}

		return true;
	}

	routed_web_server::route_type& routed_web_server::register_route(route_type&& route)
	{
		m_routes.push_back(std::move(route));

		return m_routes.back();
	}

	routed_web_server::request_result routed_web_server::handle_request(request& req)
	{
		try
		{
			const route_type* const route = get_route(req);

			if (route)
			{
				route->check_authentication(req);
				req.send_session();

				const auto result = route->function(req);

				if (result == request_result::handled)
				{
					if (req.status_code() == 0)
					{
						req.send_status_code(200);
					}

					req.send_data("", 0);
				}

				return result;
			}
		}
		catch (const http_error& ex)
		{
			req.set_from_error(ex);

			return request_result::handled;
		}

		return web_server::handle_request(req);
	}

	const routed_web_server::route_type* routed_web_server::get_route(const request& req)
	{
		const auto routes = to_pointer_list(m_routes);
		const auto matched_routes = filter(routes, [&req](const route_type* route){ return route->url_matches(req); });

		if (!matched_routes.empty())
		{
			const auto method_matched_routes = filter(matched_routes, [&req](const route_type* route){ return route->request_method_matches(req); });

			if (method_matched_routes.empty())
			{
				throw http_error(mongooseplus_error::http_405_method_not_allowed) << headers_error_info({
					{"Allow", matched_routes.front()->request_methods}
				});
			}

			const auto content_matched_routes = filter(method_matched_routes, [&req](const route_type* route){ return route->content_type_matches(req); });

			if (content_matched_routes.empty())
			{
				throw http_error(mongooseplus_error::http_406_not_acceptable);
			}

			return content_matched_routes.front();
		}

		return nullptr;
	}
}

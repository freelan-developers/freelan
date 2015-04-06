/*
 * libmongooseplus - A C++ lightweight wrapper around the libmongoose
 * library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libmongooseplus.
 *
 * libmongooseplus is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
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
 * \file mongooseplus.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The global mongooseplus include file.
 */

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <vector>
#include <set>
#include <regex>
#include <type_traits>

#include <boost/optional.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptoplus/x509/certificate.hpp>
#include <cryptoplus/pkey/pkey.hpp>

#include <kfather/value.hpp>

#include "error.hpp"

struct mg_connection;

namespace mongooseplus
{
	class request;

	/**
	 * \brief Represents a HTTP header.
	 */
	class header_type
	{
		public:
			header_type(const std::string& _key, const std::string& _value) :
				m_key(boost::to_lower_copy(_key)),
				m_value(_value)
			{}

			template <typename SequenceType>
			header_type(const typename std::enable_if<std::is_same<typename SequenceType::value_type, std::string>::value, std::string>::type& _key, const SequenceType& _values) :
				m_key(boost::to_lower_copy(_key)),
				m_value(flatten_list(_values))
			{}

			const std::string& key() const
			{
				return m_key;
			}

			const std::string& value() const
			{
				return m_value;
			}

			boost::optional<std::string> value(const std::string& key) const;

			std::string value(const std::string& _key, const std::string& default_value) const
			{
				const auto result = value(_key);

				if (result)
				{
					return *result;
				}

				return default_value;
			}

			std::vector<std::string> values() const
			{
				return unflatten_list(m_value);
			}

		private:
			template <typename SequenceType>
			static std::string flatten_list(const SequenceType& values);
			static std::vector<std::string> unflatten_list(const std::string& value);

			std::string m_key;
			std::string m_value;
	};

	class generic_session
	{
		public:

			generic_session(const std::string& _session_id, const boost::posix_time::time_duration& duration = boost::posix_time::minutes(5)) :
				m_session_id(_session_id),
				m_expires(boost::posix_time::second_clock::universal_time() + duration)
			{
			}

			virtual ~generic_session()
			{
			}

			const std::string& session_id() const
			{
				return m_session_id;
			}

			bool has_expired(const boost::posix_time::ptime& reference = boost::posix_time::second_clock::universal_time()) const
			{
				return (m_expires <= reference);
			}

			void expires_in(const boost::posix_time::time_duration& duration)
			{
				expires_at(boost::posix_time::second_clock::universal_time() + duration);
			}

			void expires_at(const boost::posix_time::ptime& date)
			{
				m_expires = date;
			}

			const boost::posix_time::ptime& expiration_date() const
			{
				return m_expires;
			}

		private:

			void set_session_id(const std::string& _session_id)
			{
				m_session_id = _session_id;
			}

			std::string m_session_id;
			boost::posix_time::ptime m_expires;

			friend class session_handler_type;
	};

	/**
	 * \brief A session handler class.
	 */
	class session_handler_type
	{
		public:

			session_handler_type()
			{
				generate_session_id();
			}

			template <typename SessionType, typename... Types>
			boost::shared_ptr<SessionType> generate_session(Types&&... values)
			{
				const std::string session_id = generate_session_id();
				boost::shared_ptr<SessionType> session = boost::make_shared<SessionType>(session_id, std::forward<Types>(values)...);

				m_sessions[session_id] = session;

				return session;
			}

			boost::shared_ptr<generic_session> read_session(const request& req) const;

			void clear_expired();

		private:

			std::string generate_session_id() const;

			mutable std::string m_last_session_id;
			std::map<std::string, boost::shared_ptr<generic_session>> m_sessions;
	};

	/**
	 * \brief The HTTP exception class.
	 */
	class http_error : public boost::system::system_error, public boost::exception
	{
		public:
			explicit http_error(mongooseplus_error error) :
				boost::system::system_error(make_error_code(error))
			{
			}
	};

	typedef std::vector<header_type> header_list_type;
	typedef boost::error_info<struct tag_headers, header_list_type> headers_error_info;
	typedef boost::error_info<struct tag_error_content, std::string> error_content_error_info;

	/**
	 * \brief A base web server class.
	 */
	class web_server
	{
		public:
			web_server();
			virtual ~web_server();

			void run(int poll_period = 1000);
			void stop();

		private:
			struct underlying_server_type;
			std::unique_ptr<underlying_server_type> m_server;
			std::atomic<bool> m_is_running;

		protected:
			session_handler_type& session_handler()
			{
				return m_session_handler;
			}

			void set_option(const std::string& name, const std::string& value);
			void set_certificate_and_private_key(cryptoplus::x509::certificate cert, cryptoplus::pkey::pkey private_key);

			enum class request_result
			{
				handled,
				ignored,
				expect_more
			};

			virtual request_result handle_auth(request&)
			{
				return request_result::handled;
			}

			virtual void prepare_request(request&);

			virtual request_result handle_request(request&)
			{
				return request_result::ignored;
			}

			virtual request_result handle_poll(request&)
			{
				return request_result::ignored;
			};

			virtual request_result handle_http_error(request&)
			{
				return request_result::ignored;
			}

			virtual request_result handle_close(request&)
			{
				return request_result::ignored;
			}

			virtual boost::shared_ptr<generic_session> handle_session_required(const request&)
			{
				return boost::shared_ptr<generic_session>();
			}

		private:
			session_handler_type m_session_handler;

			friend class base_authentication_handler;
			friend class request;
	};

	/**
	 * \brief A base request class.
	 */
	class request
	{
		public:
			web_server& get_web_server()
			{
				return m_web_server;
			}
			boost::shared_ptr<generic_session> get_session() const
			{
				return m_session;
			}
			template <typename SessionType>
			boost::shared_ptr<SessionType> get_session() const
			{
				return boost::dynamic_pointer_cast<SessionType>(m_session);
			}
			void set_session(boost::shared_ptr<generic_session> _session)
			{
				m_session = _session;
			}
			template <typename SessionType, typename... Types>
			void set_session(Types&&... values)
			{
				set_session(get_web_server().session_handler().generate_session<SessionType>(std::forward<Types>(values)...));
			}
			std::string uri() const;
			header_list_type get_headers() const;
			boost::optional<header_type> get_header(const std::string& name) const;
			header_type get_header(const std::string& name, const std::string& default_value) const;
			header_type get_header(const std::string& name, const std::vector<std::string>& default_values) const;
			std::string request_method() const;
			std::string http_version() const;
			std::string query_string() const;
			int status_code() const;
			std::string content_type() const;
			const char* content() const;
			size_t content_size() const;
			kfather::value_type json() const;
			boost::asio::ip::address local_ip() const;
			uint16_t local_port() const;
			std::string local() const;
			boost::asio::ip::address remote_ip() const;
			uint16_t remote_port() const;
			std::string remote() const;
			void set_user_param(void* user_param);
			void* get_user_param() const;
			void send_status_code(int status_code);
			void send_header(const header_type& header);
			void send_header(header_type&& header);
			template <typename... Types>
			void send_header(Types&&... values)
			{
				send_header(header_type(std::forward<Types>(values)...));
			}
			void send_headers(const header_list_type& headers)
			{
				for (auto&& header : headers)
				{
					send_header(header);
				}
			}
			void send_session();
			void send_data(const void* data, size_t data_len);
			void send_data(const std::string& data)
			{
				send_data(data.empty() ? nullptr : &data[0], data.size());
			}
			void send_json(const kfather::value_type& json);
			void write(const void* buf, size_t buf_len);
			void set_from_error(const http_error& ex);

		private:
			request(web_server&, mg_connection* connection);
			mg_connection* m_connection;
			web_server& m_web_server;
			boost::shared_ptr<generic_session> m_session;

			friend struct web_server::underlying_server_type;
	};

	/**
	 * \brief Authentication handler class.
	 */
	class base_authentication_handler
	{
		public:

			virtual ~base_authentication_handler() {};

			const std::string& scheme() const
			{
				return m_scheme;
			}

			void authenticate(request& req) const
			{
				const auto authorization_header = req.get_header("authorization");

				if (authorization_header)
				{
					if (authenticate_from_header(req, *authorization_header))
					{
						return;
					}

					raise_authentication_error();
				}
				else
				{
					if (authenticate_from_session(req, req.get_session()))
					{
						return;
					}

					raise_authentication_error();
				}
			}

		protected:

			base_authentication_handler(const std::string& _scheme) :
				m_scheme(_scheme)
			{
			}

			virtual bool authenticate_from_header(request& req, const header_type& header) const = 0;
			virtual bool authenticate_from_session(request& req, boost::shared_ptr<generic_session> session) const = 0;
			virtual void raise_authentication_error() const = 0;

		private:

			std::string m_scheme;
	};

	class basic_session_type
	{
		public:

			explicit basic_session_type(const std::string& _username) :
				m_username(_username)
			{
			}

			virtual ~basic_session_type()
			{
			}

			const std::string& username() const
			{
				return m_username;
			}

			void set_username(const std::string& _username)
			{
				m_username = _username;
			}

		private:

			std::string m_username;
	};

	class basic_authentication_handler : public base_authentication_handler
	{
		public:

			basic_authentication_handler(const std::string& _realm) :
				base_authentication_handler("Basic"),
				m_realm(_realm)
			{
			}

			const std::string& realm() const
			{
				return m_realm;
			}

		protected:

			bool authenticate_from_header(request& req, const header_type& header) const override;
			bool authenticate_from_session(request& req, boost::shared_ptr<generic_session> session) const override;
			virtual bool authenticate_from_username_and_password(request& req, const std::string& username, const std::string& password) const = 0;
			void raise_authentication_error() const override;

		private:

			std::string m_realm;
	};

	class routed_web_server : public web_server
	{
		protected:

			struct route_type
			{
				typedef std::function<request_result (request&)> function_type;

				std::regex url_regex;
				std::set<std::string> request_methods;
				std::set<std::string> content_types;
				boost::shared_ptr<base_authentication_handler> authentication_handler;
				function_type function;

				route_type(const std::string& _url_regex, function_type _function) :
					url_regex(_url_regex),
					function(_function)
				{}

				route_type(const std::string& _url_regex, const std::set<std::string>& _request_methods, function_type _function) :
					url_regex(_url_regex),
					request_methods(_request_methods),
					function(_function)
				{}

				route_type(const std::string& _url_regex, const std::set<std::string>& _request_methods, const std::set<std::string>& _content_types, function_type _function) :
					url_regex(_url_regex),
					request_methods(_request_methods),
					content_types(_content_types),
					function(_function)
				{}

				route_type& set_authentication_handler(boost::shared_ptr<base_authentication_handler> auth_handler)
				{
					authentication_handler = auth_handler;

					return *this;
				}

				template <typename AuthenticationHandler, typename... Types>
				route_type& set_authentication_handler(Types&&... values)
				{
					return set_authentication_handler(boost::make_shared<AuthenticationHandler>(std::forward<Types>(values)...));
				}

				bool url_matches(const request&) const;
				bool request_method_matches(const request&) const;
				bool content_type_matches(const request&) const;
				void check_authentication(request& req) const
				{
					if (authentication_handler)
					{
						authentication_handler->authenticate(req);
					}
				}
			};

			route_type& register_route(route_type&& route);

			template <typename... Types>
			route_type& register_route(Types&&... values)
			{
				return register_route(route_type(std::forward<Types>(values)...));
			}

			request_result handle_request(request& req) override;

		private:
			const route_type* get_route(const request&);

			std::vector<route_type> m_routes;
	};

	template <typename RequestInfoType>
	class object_web_server : public routed_web_server
	{
		protected:

			RequestInfoType& get_request_info(request& req)
			{
				return *static_cast<RequestInfoType*>(req.get_user_param());
			}

			virtual request_result handle_pre_auth(request& req)
			{
				return routed_web_server::handle_auth(req);
			}

			virtual void handle_post_auth(request&)
			{
			}

		private:

			request_result handle_auth(request& req) override
			{
				const request_result result = handle_pre_auth(req);

				if (result != request_result::handled)
				{
					std::unique_ptr<RequestInfoType> request_info(new RequestInfoType());
					req.set_user_param(request_info.get());

					handle_post_auth(req);

					request_info.release();
				}

				return result;
			}

			request_result handle_close(request& req)
			{
				std::unique_ptr<RequestInfoType> user_data(static_cast<RequestInfoType*>(req.get_user_param()));
				req.set_user_param(nullptr);

				return routed_web_server::handle_close(req);
			}
	};
}

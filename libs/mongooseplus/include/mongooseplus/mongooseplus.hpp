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

#include <boost/optional.hpp>
#include <boost/asio.hpp>

struct mg_connection;

namespace mongooseplus
{
	class web_server
	{
		public:
			web_server();
			virtual ~web_server() {};
			
		private:
			struct underlying_server_type;
			std::unique_ptr<underlying_server_type> m_server;

		protected:
			class connection
			{
				public:
					std::string uri() const;
					boost::optional<std::string> get_header(const std::string& name) const;
					std::string request_method() const;
					std::string http_version() const;
					std::string query_string() const;
					const char* content() const;
					size_t content_size() const;
					boost::asio::ip::address local_ip() const;
					uint16_t local_port() const;
					boost::asio::ip::address remote_ip() const;
					uint16_t remote_port() const;

				private:
					explicit connection(mg_connection* connection);
					mg_connection* m_connection;

					friend struct web_server::underlying_server_type;
			};

			virtual bool handle_request(connection& conn) = 0;
	};
}
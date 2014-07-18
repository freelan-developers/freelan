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

#ifndef FREELAN_SERVER_HPP
#define FREELAN_SERVER_HPP

#include "os.hpp"
#include "configuration.hpp"

#include <boost/network/include/http/server.hpp>

namespace freelan
{
	struct server_handler_type;
	typedef boost::network::http::server<server_handler_type> server_type;

	struct server_handler_type
	{
		void operator()(const server_type::request& request, server_type::response& response);
		void log(const char* c) { std::cout << "log: " << c << std::endl; };
	};

	class web_server_type
	{
		public:
			web_server_type(boost::shared_ptr<boost::asio::io_service>& io_service, const freelan::server_configuration& configuration);
			void run() { m_server.run(); }
			void stop() { m_server.stop(); }

		private:

			static server_type::options create_options(boost::shared_ptr<boost::asio::io_service>& io_service, const freelan::server_configuration& configuration, server_handler_type& handler);

			server_handler_type m_handler;
			server_type m_server;
	};

}

#endif /* FREELAN_SERVER_HPP */

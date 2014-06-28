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

#include <cassert>

namespace freelan
{
	void server_handler_type::operator()(const server_type::request& request, server_type::response& response)
	{
		static_cast<void>(request);
		static_cast<void>(response);
	}

	web_server_type::web_server_type(boost::shared_ptr<boost::asio::io_service>& io_service, const freelan::server_configuration& configuration) :
		m_handler(),
		m_server(create_options(io_service, configuration, m_handler))
	{
	}

	server_type::options web_server_type::create_options(boost::shared_ptr<boost::asio::io_service>& io_service, const freelan::server_configuration& configuration, server_handler_type& handler)
	{
		server_type::options options(handler);
		options.io_service(io_service);

		if (!configuration.listen_on_address.empty())
		{
			options.address(configuration.listen_on_address);
		}

		if (!configuration.listen_on_port.empty())
		{
			options.port(configuration.listen_on_port);
		}

		return options;
	}
}

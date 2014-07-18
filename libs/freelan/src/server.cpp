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
	const std::string server_handler_type::LOG_PREFIX = "Web server: ";

	void server_handler_type::log(const char* c) {
		m_logger(LL_WARNING) << LOG_PREFIX << c;
	}

	void server_handler_type::operator()(const server_type::request& request, server_type::response& response)
	{
		static_cast<void>(request);
		static_cast<void>(response);

		const std::string request_source = source(request);
		m_logger(LL_DEBUG) << LOG_PREFIX << "request from " << request_source;
	}

	web_server_type::web_server_type(logger& _logger, const freelan::server_configuration& configuration) :
		m_handler(_logger),
		m_server(create_options(configuration, m_handler))
	{
	}

	server_type::options web_server_type::create_options(const freelan::server_configuration& configuration, server_handler_type& handler)
	{
		server_type::options options(handler);

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

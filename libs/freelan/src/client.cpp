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
 * \file client.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A client implementation.
 */

#include "client.hpp"

#include <boost/make_shared.hpp>

namespace freelan
{
	web_client::web_client(boost::asio::io_service& io_service, freelan::logger& _logger, const freelan::client_configuration& configuration) :
		m_curl_manager(io_service),
		m_logger(_logger)
	{
		static_cast<void>(configuration);
		static_cast<void>(m_logger);

		boost::shared_ptr<curl> request = boost::make_shared<curl>();

		request->set_url("http://www.google.fr");
		request->set_get();

		m_curl_manager.execute(request, [](const boost::system::error_code& ec) {
			std::cout << "Done: " << ec << std::endl;
		});
	}
}

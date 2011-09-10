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
 * \file core.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan core class.
 */

#include "core.hpp"

#include <boost/bind.hpp>

namespace freelan
{
	core::core(boost::asio::io_service& io_service, const configuration& _configuration) :
		m_configuration(_configuration),
		m_tap_adapter(io_service),
		m_server(io_service, m_configuration.listen_on, *m_configuration.identity)
	{
		m_server.set_hello_message_callback(boost::bind(&core::on_hello_request, this, _1, _2, _3));
		m_server.set_presentation_message_callback(boost::bind(&core::on_presentation, this, _1, _2, _3, _4, _5));
		m_server.set_session_request_message_callback(boost::bind(&core::on_session_request, this, _1, _2, _3));
		m_server.set_session_established_callback(boost::bind(&core::on_session_established, this, _1, _2));
		m_server.set_session_lost_callback(boost::bind(&core::on_session_lost, this, _1, _2));
	}
	
	void core::async_greet(const ep_type& target)
	{
		m_server.async_greet(target, boost::bind(&core::on_hello_response, this, _1, _2, _3, _4), m_configuration.hello_timeout);
	}

	bool core::on_hello_request(fscp::server& _server, const ep_type& sender, bool default_accept)
	{
		if (default_accept)
		{
			// TODO: Here we should check if sender is in the blacklist.
			// For now, let's assume it is not.
			if (true)
			{
				_server.async_introduce_to(sender);

				return true;
			}
		}

		return false;
	}
	
	void core::on_hello_response(fscp::server& _server, const ep_type& sender, const boost::posix_time::time_duration& time_duration, bool success)
	{
		(void)time_duration;

		if (success)
		{
			_server.async_introduce_to(sender);
		}
	}
	
	bool core::on_presentation(fscp::server& _server, const ep_type& sender, cert_type sig_cert, cert_type enc_cert, bool default_accept)
	{
		(void)sig_cert;
		(void)enc_cert;

		if (default_accept)
		{
			// TODO: Here we should check for the certificates validity.
			// For now, let's assume they are valid.
			if (true)
			{
				_server.async_request_session(sender);
				return true;
			}
		}

		return false;
	}
	
	bool core::on_session_request(fscp::server& _server, const ep_type& sender, bool default_accept)
	{
		(void)_server;
		(void)sender;

		if (default_accept)
		{
			// TODO: Here we should check if sender is in the blacklist.
			// For now, let's assume it is not.
			if (true)
			{
				return true;
			}
		}

		return false;
	}

	void core::on_session_established(fscp::server& _server, const ep_type& sender)
	{
		(void)_server;
		(void)sender;
	}

	void core::on_session_lost(fscp::server& _server, const ep_type& sender)
	{
		(void)_server;
		(void)sender;
	}
}

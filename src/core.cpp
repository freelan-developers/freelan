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
#include <boost/foreach.hpp>

namespace freelan
{
	const boost::posix_time::time_duration core::CONTACT_PERIOD = boost::posix_time::seconds(30);

	core::core(boost::asio::io_service& io_service, const configuration& _configuration) :
		m_configuration(_configuration),
		m_server(io_service, *m_configuration.identity),
		m_tap_adapter(io_service),
		m_contact_timer(io_service, CONTACT_PERIOD)
	{
		m_server.set_hello_message_callback(boost::bind(&core::on_hello_request, this, _1, _2, _3));
		m_server.set_presentation_message_callback(boost::bind(&core::on_presentation, this, _1, _2, _3, _4, _5));
		m_server.set_session_request_message_callback(boost::bind(&core::on_session_request, this, _1, _2, _3));
		m_server.set_session_established_callback(boost::bind(&core::on_session_established, this, _1, _2));
		m_server.set_session_lost_callback(boost::bind(&core::on_session_lost, this, _1, _2));
		m_server.set_data_message_callback(boost::bind(&core::on_data, this, _1, _2, _3));
	}

	void core::open()
	{
		m_server.open(m_configuration.listen_on);

		m_tap_adapter.open();

		BOOST_FOREACH(const configuration::ip_address_netmask_type& ian, m_configuration.tap_adapter_addresses)
		{
			m_tap_adapter.add_ip_address(ian.address, ian.netmask);
		}

		m_tap_adapter.set_connected_state(true);

		m_tap_adapter.async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(m_tap_adapter), _1, _2));

		m_contact_timer.async_wait(boost::bind(&core::do_contact, this, boost::asio::placeholders::error));
	}

	void core::close()
	{
		m_contact_timer.cancel();

		m_tap_adapter.cancel();
		m_tap_adapter.set_connected_state(false);

		BOOST_FOREACH(configuration::ip_address_netmask_type& ian, m_configuration.tap_adapter_addresses)
		{
			m_tap_adapter.remove_ip_address(ian.address, ian.netmask);
		}

		m_tap_adapter.close();

		m_server.close();
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

	void core::on_data(fscp::server& _server, const ep_type& sender, boost::asio::const_buffer data)
	{
		(void)_server;
		(void)sender;

		// TODO: Here we must read the source ethernet address and update the switch routing table according to it.

		boost::system::error_code ec;

		m_tap_adapter.write(data, ec);

		if (ec)
		{
			//TODO: Report the error somehow.
		}
	}

	void core::tap_adapter_read_done(asiotap::tap_adapter& _tap_adapter, const boost::system::error_code& ec, size_t cnt)
	{
		if (!ec)
		{
			boost::asio::const_buffer data = boost::asio::buffer(m_tap_adapter_buffer, cnt);

			//TODO: Here we must read the destination ethernet address and send to the targetted hosts only.

			m_server.async_send_data_to_all(data);

			// Start another read
			_tap_adapter.async_read(boost::asio::buffer(m_tap_adapter_buffer, m_tap_adapter_buffer.size()), boost::bind(&core::tap_adapter_read_done, this, boost::ref(_tap_adapter), _1, _2));
		}
		else
		{
			//TODO: Report the error somehow.
			close();
		}
	}

	void core::do_contact(const boost::system::error_code& ec)
	{
		if (ec != boost::asio::error::operation_aborted)
		{
			m_contact_timer.expires_from_now(CONTACT_PERIOD);
			m_contact_timer.async_wait(boost::bind(&core::do_contact, this, boost::asio::placeholders::error));
		}
	}
}

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
#include <boost/lexical_cast.hpp>

#include <cryptoplus/buffer.hpp>

#include "web_client_error.hpp"

namespace freelan
{
	void web_client::request_certificate(cryptoplus::x509::certificate_request certificate_request, request_certificate_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/request_certificate/");
		const auto data = certificate_request.write_der();

		request->set_http_header("content-type", "application/octet-stream");
		request->set_copy_post_fields(boost::asio::buffer(data.data()));

		const auto buffer = m_memory_pool.allocate_shared_buffer();
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function([buffer, count] (boost::asio::const_buffer indata) {
			using boost::asio::buffer_cast;
			using boost::asio::buffer_size;

			const char* const bytes = buffer_cast<const char*>(indata);
			const size_t bytes_len = buffer_size(indata);
			char* const dest = buffer_cast<char*>(buffer) + *count;
			const size_t dest_len = buffer_size(buffer) - *count;

			if (dest_len < bytes_len)
			{
				*count = buffer_size(buffer);
				std::memcpy(dest, bytes, dest_len);

				return dest_len;
			}
			else
			{
				*count += bytes_len;
				std::memcpy(dest, bytes, bytes_len);

				return bytes_len;
			}
		});

		m_curl_multi_asio->execute(request, [self, request, buffer, count, handler] (boost::system::error_code ec) {
			using boost::asio::buffer_cast;
			using boost::asio::buffer_size;

			cryptoplus::x509::certificate cert;

			if (ec)
			{
				self->m_logger(fscp::log_level::error) << "Error while sending HTTP(S) request to " << request->get_effective_url() << ": " << ec.message() << " (" << ec << ")";
			}
			else
			{
				self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

				const auto content_type = request->get_content_type();

				if (content_type != "application/x-x509-ca-cert")
				{
					try
					{
						cert = cryptoplus::x509::certificate::from_der(buffer_cast<const char*>(buffer), *count);
					}
					catch(const boost::system::system_error& ex)
					{
						ec = ex.code();
					}
				}
				else
				{
					ec = make_error_code(web_client_error::unsupported_content_type);
				}
			}

			handler(ec, cert);
		});
	}

	web_client::web_client(boost::asio::io_service& io_service, fscp::logger& _logger, const freelan::client_configuration& configuration) :
		m_curl_multi_asio(curl_multi_asio::create(io_service)),
		m_logger(_logger),
		m_configuration(configuration),
		m_url_prefix(boost::lexical_cast<std::string>(m_configuration.protocol) + "://" + boost::lexical_cast<std::string>(m_configuration.server_endpoint))
	{
		if (m_configuration.protocol == client_configuration::client_protocol_type::http)
		{
			m_logger(fscp::log_level::warning) << "Web client not configured to use HTTPS: your username and password will be readable by anyone !";
		}
		else
		{
			if (m_configuration.disable_peer_verification)
			{
				m_logger(fscp::log_level::warning) << "Web client configured to ignore peer verification: you are vulnerable to man-in-the-middle attacks !";
			}

			if (m_configuration.disable_host_verification)
			{
				m_logger(fscp::log_level::warning) << "Web client configured to ignore host verification: you are vulnerable to man-in-the-middle attacks !";
			}
		}
	}

	boost::shared_ptr<curl> web_client::make_request(const std::string& path) const
	{
		boost::shared_ptr<curl> request = boost::make_shared<curl>();

		request->set_url(m_url_prefix + path);
		request->enable_cookie_support();

		if (m_configuration.disable_peer_verification)
		{
			request->set_ssl_peer_verification(false);
		}

		if (m_configuration.disable_host_verification)
		{
			request->set_ssl_host_verification(false);
		}

		if (!m_configuration.username.empty() || !m_configuration.password.empty())
		{
			request->set_username(m_configuration.username);
			request->set_password(m_configuration.password);
		}

		return request;
	}
}

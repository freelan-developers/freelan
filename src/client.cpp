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

#include <cassert>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <cryptoplus/x509/name.hpp>
#include <cryptoplus/x509/certificate_request.hpp>
#include <cryptoplus/pkey/rsa_key.hpp>
#include <cryptoplus/pkey/pkey.hpp>
#include <cryptoplus/base64.hpp>

#include <kfather/parser.hpp>
#include <kfather/formatter.hpp>

#include "configuration.hpp"
#include "logger.hpp"
#include "logger_stream.hpp"

namespace freelan
{
	using cryptoplus::base64_encode;
	using cryptoplus::base64_decode;

	namespace
	{
		std::string server_protocol_to_scheme(const server_configuration::server_protocol_type& protocol)
		{
			switch (protocol)
			{
				case server_configuration::SP_HTTP:
					return "http://";
				case server_configuration::SP_HTTPS:
					return "https://";
			}

			throw std::runtime_error("Unsupported server protocol.");
		}

		cryptoplus::x509::certificate string_to_certificate(const std::string& str)
		{
			return cryptoplus::x509::certificate::from_der(base64_decode(str));
		}

		std::string certificate_request_to_string(const cryptoplus::x509::certificate_request& csr)
		{
			return base64_encode(csr.write_der());
		}

		bool has_value(const client::values_type& values, const std::string& key, json::value_type& value)
		{
			client::values_type::items_type::const_iterator it = values.items.find(key);

			if (it != values.items.end())
			{
				value = it->second;

				return true;
			}

			return false;
		}

		void assert_has_value(const client::values_type& values, const std::string& key, json::value_type& value)
		{
			if (!has_value(values, key, value))
			{
				throw std::runtime_error("Missing required value \"" + key + "\".");
			}
		}

		template <typename T>
		void assert_has_value(const client::values_type& values, const std::string& key, T& value)
		{
			json::value_type _val;

			assert_has_value(values, key, _val);

			value = json::value_cast<T>(_val);
		}

		void assert_has_value(const client::values_type& values, const std::string& key, cryptoplus::x509::certificate& cert)
		{
			std::string cert_str;

			assert_has_value(values, key, cert_str);

			cert = string_to_certificate(cert_str);
		}

		void assert_has_value(const client::values_type& values, const std::string& key, ipv4_network_address& ep)
		{
			std::string str;

			assert_has_value(values, key, str);

			ep = boost::lexical_cast<ipv4_network_address>(str);
		}

		void assert_has_value(const client::values_type& values, const std::string& key, ipv6_network_address& ep)
		{
			std::string str;

			assert_has_value(values, key, str);

			ep = boost::lexical_cast<ipv6_network_address>(str);
		}
	}

	client::client(const freelan::configuration& configuration, freelan::logger& _logger) :
		m_configuration(configuration),
		m_logger(_logger),
		m_scheme(server_protocol_to_scheme(m_configuration.server.protocol))
	{
		if (m_configuration.server.protocol == server_configuration::SP_HTTP)
		{
			m_logger(LL_WARNING) << "Current server protocol is HTTP. Your password will be sent in cleartext to the server !";
		}

		// Set the timeout
		m_request.set_connect_timeout(boost::posix_time::seconds(5));

		// Set the user agent
		if (m_configuration.server.user_agent.empty())
		{
			m_logger(LL_WARNING) << "Empty user agent specified, taking libcurl's default.";
		}
		else
		{
			m_logger(LL_INFORMATION) << "User agent set to \"" << m_configuration.server.user_agent << "\".";

			m_request.set_user_agent(m_configuration.server.user_agent);
		}

		// Set the HTTP proxy
		if (m_configuration.server.https_proxy)
		{
			if (*m_configuration.server.https_proxy != hostname_endpoint::null())
			{
				m_logger(LL_INFORMATION) << "Setting HTTP(S) proxy to \"" << *m_configuration.server.https_proxy << "\".";
			}
			else
			{
				m_logger(LL_INFORMATION) << "Disabling HTTP(S) proxy.";
			}

			m_request.set_proxy(*m_configuration.server.https_proxy);
		}

		// Disable peer verification if required
		if (m_configuration.server.disable_peer_verification)
		{
			m_logger(LL_WARNING) << "Peer verification disabled ! Connection will be a LOT LESS SECURE.";

			m_request.set_ssl_peer_verification(false);
		}
		else
		{
			if (!m_configuration.server.ca_info.empty())
			{
				m_logger(LL_INFORMATION) << "Setting CA info to \"" << m_configuration.server.ca_info.string() << "\"";

				m_request.set_ca_info(m_configuration.server.ca_info);
			}
		}

		// Disable host verification if required
		if (m_configuration.server.disable_host_verification)
		{
			m_logger(LL_WARNING) << "Host verification disabled ! Connection will be less secure.";

			m_request.set_ssl_host_verification(false);
		}

		// Set the read callback
		m_request.set_write_function(boost::bind(&client::read_data, this, _1));

		// Enable cookie support
		m_request.enable_cookie_support();
	}

	void client::authenticate()
	{
		get_server_information(
		    m_request,
		    m_server_name,
		    m_server_version_major,
		    m_server_version_minor,
		    m_login_url,
		    m_get_authority_certificate_url,
		    m_join_network_url,
		    m_sign_url
		);

		if (m_server_version_major == 1)
		{
			v1_authenticate(m_request, m_login_url);
		}
		else
		{
			m_logger(LL_ERROR) << "Unsupported server version.";

			throw std::runtime_error("Server protocol error.");
		}
	}

	cryptoplus::x509::certificate client::get_authority_certificate()
	{
		if (m_server_version_major == 1)
		{
			return v1_get_authority_certificate(m_request, m_get_authority_certificate_url);
		}
		else
		{
			m_logger(LL_ERROR) << "Unsupported server version.";

			throw std::runtime_error("Server protocol error.");
		}
	}

	network_info client::join_network(const std::string& network)
	{
		if (m_server_version_major == 1)
		{
			return v1_join_network(m_request, m_join_network_url, network);
		}
		else
		{
			m_logger(LL_ERROR) << "Unsupported server version.";

			throw std::runtime_error("Server protocol error.");
		}
	}

	cryptoplus::x509::certificate client::renew_certificate(const cryptoplus::x509::certificate_request& csr)
	{
		if (m_server_version_major == 1)
		{
			return v1_sign_certificate_request(m_request, m_sign_url, csr);
		}
		else
		{
			m_logger(LL_ERROR) << "Unsupported server version.";

			throw std::runtime_error("Server protocol error.");
		}
	}

	void client::perform_request(curl& request, const std::string& url, values_type& values)
	{
		request.set_url(url);

		m_data.clear();

		request.perform();

		const long response_code = request.get_response_code();

		m_logger(LL_DEBUG) << "HTTP response code: " << response_code;
		m_logger(LL_DEBUG) << "Received:\n" << m_data;

		if (response_code != 200)
		{
			m_logger(LL_ERROR) << "Unexpected HTTP response code " << response_code << ".";
			m_logger(LL_ERROR) << "Here is what the server replied:\n" << m_data;

			throw std::runtime_error("HTTP request failed.");
		}
		else
		{
			const std::string content_type = request.get_content_type();

			if (content_type != "application/json")
			{
				m_logger(LL_ERROR) << "Unsupported content type received: " << content_type;

				throw std::runtime_error("Unexpected server error.");
			}
			else
			{
				json::parser parser;

				json::value_type value;

				if (!parser.parse(value, m_data))
				{
					throw std::runtime_error("JSON parsing failed.");
				}
				else
				{
					if (!json::is<values_type>(value))
					{
						throw std::runtime_error("Expected a JSON object.");
					}
					else
					{
						values = json::value_cast<values_type>(value);
					}
				}
			}
		}
	}

	void client::perform_get_request(curl& request, const std::string& url, values_type& values)
	{
		request.set_get();

		request.set_http_header("Accept", "application/json");

		m_logger(LL_DEBUG) << "Sent: GET " << url;

		perform_request(request, url, values);
	}

	void client::perform_post_request(curl& request, const std::string& url, const values_type& parameters, values_type& values)
	{
		request.set_post();

		request.set_http_header("Accept", "application/json");
		request.set_http_header("Content-Type", "application/json");
		request.unset_http_header("Expect");

		json::pretty_print_formatter formatter;
		const std::string json = formatter.format(parameters);

		request.set_copy_post_fields(boost::asio::buffer(json));

		m_logger(LL_DEBUG) << "Sent: POST " << url << "\n" << json;

		perform_request(request, url, values);
	}

	void client::get_server_information(
	    curl& request,
	    std::string& server_name,
	    unsigned int& server_version_major,
	    unsigned int& server_version_minor,
	    std::string& login_url,
	    std::string& get_authority_certificate_url,
	    std::string& join_network_url,
	    std::string& sign_url
	)
	{
		m_logger(LL_INFORMATION) << "Getting server information from " << m_configuration.server.host << "...";

		const std::string url = m_scheme + boost::lexical_cast<std::string>(m_configuration.server.host) + "/api/information";

		request.reset_http_headers();

		values_type values;

		perform_get_request(request, url, values);

		assert_has_value(values, "name", server_name);
		assert_has_value(values, "major", server_version_major);
		assert_has_value(values, "minor", server_version_minor);
		assert_has_value(values, "login_url", login_url);
		assert_has_value(values, "get_authority_certificate_url", get_authority_certificate_url);
		assert_has_value(values, "join_network_url", join_network_url);
		assert_has_value(values, "sign_url", sign_url);

		m_logger(LL_INFORMATION) << "Server version is " << server_name << "/" << server_version_major << "." << server_version_minor;
	}

	void client::v1_authenticate(curl& request, const std::string& login_url)
	{
		const std::string url = m_scheme + boost::lexical_cast<std::string>(m_configuration.server.host) + login_url;

		std::string challenge;

		v1_get_server_login(request, url, challenge);
		v1_post_server_login(request, url, challenge);
	}

	cryptoplus::x509::certificate client::v1_get_authority_certificate(curl& request, const std::string& get_authority_certificate_url)
	{
		const std::string url = m_scheme + boost::lexical_cast<std::string>(m_configuration.server.host) + get_authority_certificate_url;

		m_logger(LL_INFORMATION) << "Requesting authority certificate...";

		request.reset_http_headers();

		values_type values;

		perform_get_request(request, url, values);

		cryptoplus::x509::certificate authority_certificate;

		assert_has_value(values, "authority_certificate", authority_certificate);

		m_logger(LL_INFORMATION) << "Authority certificate received from server.";

		return authority_certificate;
	}

	network_info_v1 client::v1_join_network(curl& request, const std::string& join_network_url, const std::string& network)
	{
		const std::string url = m_scheme + boost::lexical_cast<std::string>(m_configuration.server.host) + join_network_url;

		m_logger(LL_INFORMATION) << "Joining network \"" << network << "\"...";

		request.reset_http_headers();

		values_type parameters;

		//TODO: Implement this properly
		json::array_type endpoints;
		endpoints.items.push_back(json::string_type("0.0.0.0:12000"));

		parameters.items["network"] = network;
		parameters.items["endpoints"] = endpoints;

		values_type values;

		perform_post_request(request, url, parameters, values);

		network_info_v1 ninfo;

		json::array_type users_certificates_array;
		json::array_type users_endpoints_array;

		assert_has_value(values, "ipv4_address_prefix_length", ninfo.ipv4_address_prefix_length);
		assert_has_value(values, "ipv6_address_prefix_length", ninfo.ipv6_address_prefix_length);
		assert_has_value(values, "users_certificates", users_certificates_array);
		assert_has_value(values, "users_endpoints", users_endpoints_array);

		if (!ninfo.ipv4_address_prefix_length.is_null())
		{
			m_logger(LL_DEBUG) << "IPv4 address is " << ninfo.ipv4_address_prefix_length << ".";
		}

		if (!ninfo.ipv6_address_prefix_length.is_null())
		{
			m_logger(LL_DEBUG) << "IPv6 address is " << ninfo.ipv6_address_prefix_length << ".";
		}

		for (json::array_type::items_type::const_iterator it = users_certificates_array.items.begin(); it != users_certificates_array.items.end(); ++it)
		{
			const std::string cert_str = json::value_cast<json::string_type>(*it);

			m_logger(LL_DEBUG) << "Adding " << cert_str << " to the users certificates list.";

			ninfo.users_certificates.push_back(string_to_certificate(cert_str));
		}

		for (json::array_type::items_type::const_iterator it = users_endpoints_array.items.begin(); it != users_endpoints_array.items.end(); ++it)
		{
			const std::string ep = json::value_cast<json::string_type>(*it);

			m_logger(LL_DEBUG) << "Adding " << ep << " to the users endpoints list.";

			ninfo.users_endpoints.push_back(boost::lexical_cast<endpoint>(ep));
		}

		m_logger(LL_INFORMATION) << "Joined network \"" << network << "\" succesfully.";

		return ninfo;
	}

	cryptoplus::x509::certificate client::v1_sign_certificate_request(curl& request, const std::string& sign_url, const cryptoplus::x509::certificate_request& csr)
	{
		const std::string url = m_scheme + boost::lexical_cast<std::string>(m_configuration.server.host) + sign_url;

		m_logger(LL_INFORMATION) << "Sending certificate request...";

		request.reset_http_headers();

		values_type parameters;

		parameters.items["certificate_request"] = certificate_request_to_string(csr);

		values_type values;

		perform_post_request(request, url, parameters, values);

		cryptoplus::x509::certificate certificate;

		assert_has_value(values, "certificate", certificate);

		m_logger(LL_INFORMATION) << "Certificate request was signed.";

		return certificate;
	}

	void client::v1_get_server_login(curl& request, const std::string& url, std::string& challenge)
	{
		request.reset_http_headers();

		values_type values;

		perform_get_request(request, url, values);

		assert_has_value(values, "challenge", challenge);

		m_logger(LL_DEBUG) << "Login challenge is: " << challenge;
	}

	void client::v1_post_server_login(curl& request, const std::string& url, const std::string& challenge)
	{
		m_logger(LL_INFORMATION) << "Authenticating as " << m_configuration.server.username << "...";

		request.reset_http_headers();

		values_type parameters;

		parameters.items["challenge"] = challenge;
		parameters.items["username"] = m_configuration.server.username;
		parameters.items["password"] = m_configuration.server.password;

		values_type values;

		perform_post_request(request, url, parameters, values);

		m_logger(LL_INFORMATION) << "Succesfully authenticated as " << m_configuration.server.username << ".";
	}

	size_t client::read_data(boost::asio::const_buffer buf)
	{
		const char* _data = boost::asio::buffer_cast<const char*>(buf);
		size_t data_len = boost::asio::buffer_size(buf);

		m_data.append(_data, data_len);

		return data_len;
	}
}

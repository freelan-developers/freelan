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
#include <cryptoplus/base64.hpp>

#include <kfather/parser.hpp>
#include <kfather/formatter.hpp>

#include "web_client_error.hpp"

namespace
{
	using fscp::SharedBuffer;

	template <typename BufferType, typename BufferSizeType>
	boost::function<size_t (boost::asio::const_buffer)> get_write_function(BufferType buffer, BufferSizeType count)
	{
		const auto result = [buffer, count] (boost::asio::const_buffer indata) {
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
		};

		return result;
	}

	kfather::array_type to_json(const std::set<asiotap::endpoint>& public_endpoints)
	{
		kfather::array_type result;

		for (auto&& public_endpoint : public_endpoints)
		{
			result.items.push_back(boost::lexical_cast<std::string>(public_endpoint));
		}

		return result;
	}

	kfather::string_type to_json(const fscp::hash_type& hash)
	{
		return cryptoplus::base64_encode(cryptoplus::buffer(&hash.data[0], hash.data.size()));
	}

	kfather::array_type to_json(const std::set<fscp::hash_type>& hashes)
	{
		kfather::array_type result;

		for (auto&& hash : hashes)
		{
			result.items.push_back(to_json(hash));
		}

		return result;
	}

	template <typename ResultType>
	ResultType from_json(const kfather::array_type&);

	template <>
	std::set<asiotap::endpoint> from_json<std::set<asiotap::endpoint>>(const kfather::array_type& endpoints)
	{
		std::set<asiotap::endpoint> result;

		for (auto&& endpoint_obj : endpoints.items)
		{
			const auto endpoint_str = kfather::value_cast<kfather::string_type>(endpoint_obj);

			try
			{
				auto endpoint = boost::lexical_cast<asiotap::endpoint>(endpoint_str);
				result.insert(endpoint);
			}
			catch (const std::exception&)
			{
				// If parsing fail, we discard the value silently.
			}
		}

		return result;
	}

	template <typename ResultType>
	ResultType from_json(const kfather::string_type&);

	template <>
	fscp::hash_type from_json<fscp::hash_type>(const kfather::string_type& hash_b64)
	{
		const auto hash_buf = cryptoplus::base64_decode(hash_b64);

		if (hash_buf.data().size() != sizeof(fscp::hash_type))
		{
			throw std::runtime_error("Invalid hash size");
		}

		fscp::hash_type hash;
		std::copy_n(hash_buf.data().begin(), sizeof(hash.data), hash.data.begin());

		return hash;
	}

	template <typename ResultType>
	ResultType from_json(const kfather::object_type&);

	template <>
	std::map<fscp::hash_type, std::set<asiotap::endpoint>> from_json<std::map<fscp::hash_type, std::set<asiotap::endpoint>>>(const kfather::object_type& contacts)
	{
		std::map<fscp::hash_type, std::set<asiotap::endpoint>> result;

		for (auto&& item : contacts.items)
		{
			try
			{
				const auto hash = from_json<fscp::hash_type>(item.first);
				const auto endpoints = from_json<std::set<asiotap::endpoint>>(kfather::value_cast<kfather::array_type>(item.second));

				result[hash] = endpoints;
			}
			catch (const std::exception&)
			{
				// If parsing fail, we discard the value silently.
			}
		}

		return result;
	}
}

namespace freelan
{
	void web_client::request_certificate(cryptoplus::x509::certificate_request certificate_request, request_certificate_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/request_certificate/");
		const auto data = certificate_request.write_der();

		request->set_http_header("content-type", "application/octet-stream");
		request->set_copy_post_fields(boost::asio::buffer(data.data()));

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

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
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

					const auto content_type = request->get_content_type();

					if (content_type == "application/x-x509-cert")
					{
						try
						{
							cert = cryptoplus::x509::certificate::from_der(buffer_cast<const char*>(buffer), *count);
						}
						catch (const boost::system::system_error& ex)
						{
							ec = ex.code();
						}
					}
					else
					{
						ec = make_error_code(web_client_error::unsupported_content_type);
					}
				}
			}

			handler(ec, cert);
		});
	}

	void web_client::request_ca_certificate(request_certificate_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/request_ca_certificate/");

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

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
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

					const auto content_type = request->get_content_type();

					if (content_type == "application/x-x509-ca-cert")
					{
						try
						{
							cert = cryptoplus::x509::certificate::from_der(buffer_cast<const char*>(buffer), *count);
						}
						catch (const boost::system::system_error& ex)
						{
							ec = ex.code();
						}
					}
					else
					{
						ec = make_error_code(web_client_error::unsupported_content_type);
					}
				}
			}

			handler(ec, cert);
		});
	}

	void web_client::register_(cryptoplus::x509::certificate certificate, registration_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/register/");
		const auto data = certificate.write_der();

		request->set_http_header("content-type", "application/x-x509-cert");
		request->set_copy_post_fields(boost::asio::buffer(data.data()));

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

		m_curl_multi_asio->execute(request, [self, request, buffer, count, handler] (boost::system::error_code ec) {
			using boost::asio::buffer_cast;
			using boost::asio::buffer_size;

			boost::posix_time::ptime expiration_timestamp;

			if (ec)
			{
				self->m_logger(fscp::log_level::error) << "Error while sending HTTP(S) request to " << request->get_effective_url() << ": " << ec.message() << " (" << ec << ")";
			}
			else
			{
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

					const auto content_type = request->get_content_type();

					if (content_type == "application/json")
					{
						self->m_logger(fscp::log_level::debug) << "Received JSON data: " << std::string(buffer_cast<const char*>(buffer), *count);

						kfather::parser parser;
						kfather::value_type result;

						if (!parser.parse(result, buffer_cast<const char*>(buffer), *count))
						{
							ec = make_error_code(web_client_error::invalid_json_stream);
						}
						else
						{
							const kfather::object_type value = kfather::value_cast<kfather::object_type>(result);

							if (kfather::is_falsy(value))
							{
								ec = make_error_code(web_client_error::invalid_json_stream);
							}
							else
							{
								const std::string expiration_timestamp_str = kfather::value_cast<kfather::string_type>(value.get("expiration_timestamp"));

								boost::posix_time::time_input_facet* tif = new boost::posix_time::time_input_facet;
								tif->set_iso_extended_format();
								std::istringstream iss(expiration_timestamp_str);
								iss.imbue(std::locale(std::locale::classic(), tif));
								iss >> expiration_timestamp;

								if (expiration_timestamp.is_not_a_date_time())
								{
									ec = make_error_code(web_client_error::invalid_json_stream);
								}
							}
						}
					}
					else
					{
						ec = make_error_code(web_client_error::unsupported_content_type);
					}
				}
			}

			handler(ec, expiration_timestamp);
		});
	}

	void web_client::unregister(unregistration_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/unregister/");

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

		m_curl_multi_asio->execute(request, [self, request, buffer, count, handler] (boost::system::error_code ec) {

			if (ec)
			{
				self->m_logger(fscp::log_level::error) << "Error while sending HTTP(S) request to " << request->get_effective_url() << ": " << ec.message() << " (" << ec << ")";
			}
			else
			{
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();
				}
			}

			handler(ec);
		});
	}

	void web_client::set_contact_information(const std::set<asiotap::endpoint>& public_endpoints, set_contact_information_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/set_contact_information/");

		std::ostringstream oss;
		kfather::compact_formatter().format(
			oss,
			kfather::object_type {
				{
					{"public_endpoints", to_json(public_endpoints)}
				}
			}
		);

		request->set_http_header("content-type", "application/json");
		request->set_copy_post_fields(boost::asio::buffer(oss.str()));

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

		m_curl_multi_asio->execute(request, [self, request, buffer, count, handler] (boost::system::error_code ec) {
			using boost::asio::buffer_cast;
			using boost::asio::buffer_size;

			std::set<asiotap::endpoint> accepted_endpoints;
			std::set<asiotap::endpoint> rejected_endpoints;

			if (ec)
			{
				self->m_logger(fscp::log_level::error) << "Error while sending HTTP(S) request to " << request->get_effective_url() << ": " << ec.message() << " (" << ec << ")";
			}
			else
			{
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

					const auto content_type = request->get_content_type();

					if (content_type == "application/json")
					{
						self->m_logger(fscp::log_level::debug) << "Received JSON data: " << std::string(buffer_cast<const char*>(buffer), *count);

						kfather::parser parser;
						kfather::value_type result;

						if (!parser.parse(result, buffer_cast<const char*>(buffer), *count))
						{
							ec = make_error_code(web_client_error::invalid_json_stream);
						}
						else
						{
							const kfather::object_type value = kfather::value_cast<kfather::object_type>(result);

							if (kfather::is_falsy(value))
							{
								ec = make_error_code(web_client_error::invalid_json_stream);
							}
							else
							{
								accepted_endpoints = from_json<std::set<asiotap::endpoint>>(value.get<kfather::array_type>("accepted_endpoints"));
								rejected_endpoints = from_json<std::set<asiotap::endpoint>>(value.get<kfather::array_type>("rejected_endpoints"));
							}
						}
					}
					else
					{
						ec = make_error_code(web_client_error::unsupported_content_type);
					}
				}
			}

			handler(ec, accepted_endpoints, rejected_endpoints);
		});
	}

	void web_client::get_contact_information(const std::set<fscp::hash_type>& requested_contacts, get_contact_information_callback handler)
	{
		const auto self = shared_from_this();
		const auto request = make_request("/get_contact_information/");

		std::ostringstream oss;
		kfather::compact_formatter().format(
			oss,
			kfather::object_type {
				{
					{"requested_contacts", to_json(requested_contacts)}
				}
			}
		);

		request->set_http_header("content-type", "application/json");
		request->set_copy_post_fields(boost::asio::buffer(oss.str()));

		const auto buffer = SharedBuffer(8192);
		const boost::shared_ptr<size_t> count(new size_t(0));

		request->set_write_function(get_write_function(buffer, count));

		m_curl_multi_asio->execute(request, [self, request, buffer, count, handler] (boost::system::error_code ec) {
			using boost::asio::buffer_cast;
			using boost::asio::buffer_size;

			std::map<fscp::hash_type, std::set<asiotap::endpoint>> contacts;

			if (ec)
			{
				self->m_logger(fscp::log_level::error) << "Error while sending HTTP(S) request to " << request->get_effective_url() << ": " << ec.message() << " (" << ec << ")";
			}
			else
			{
				if (request->get_response_code() != 200)
				{
					self->m_logger(fscp::log_level::debug) << "Received unexpected HTTP return code: " << request->get_response_code();
					ec = make_error_code(web_client_error::unexpected_response);
				}
				else
				{
					self->m_logger(fscp::log_level::debug) << "Sending HTTP(S) request to " << request->get_effective_url() << ": " << request->get_response_code();

					const auto content_type = request->get_content_type();

					if (content_type == "application/json")
					{
						self->m_logger(fscp::log_level::debug) << "Received JSON data: " << std::string(buffer_cast<const char*>(buffer), *count);

						kfather::parser parser;
						kfather::value_type result;

						if (!parser.parse(result, buffer_cast<const char*>(buffer), *count))
						{
							ec = make_error_code(web_client_error::invalid_json_stream);
						}
						else
						{
							const kfather::object_type value = kfather::value_cast<kfather::object_type>(result);

							if (kfather::is_falsy(value))
							{
								ec = make_error_code(web_client_error::invalid_json_stream);
							}
							else
							{
								contacts = from_json<std::map<fscp::hash_type, std::set<asiotap::endpoint>>>(value.get<kfather::object_type>("contacts"));
							}
						}
					}
					else
					{
						ec = make_error_code(web_client_error::unsupported_content_type);
					}
				}
			}

			handler(ec, contacts);
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
		request->set_user_agent("FreeLAN/2");

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

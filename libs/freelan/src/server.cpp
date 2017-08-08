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

#ifdef USE_MONGOOSE

#include "tools.hpp"

#include <cryptoplus/x509/certificate_request.hpp>
#include <cryptoplus/base64.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <kfather/formatter.hpp>

#include <cassert>

namespace freelan
{
	using kfather::value_cast;

	namespace
	{
		class session_type : public mongooseplus::generic_session, public mongooseplus::basic_session_type
		{
			public:
				session_type(const std::string& _session_id, const std::string& _username) :
					mongooseplus::generic_session(_session_id),
					basic_session_type(_username)
				{
				}
		};

		class external_authentication_handler : public mongooseplus::basic_authentication_handler
		{
			public:

				external_authentication_handler(fscp::logger& _logger, web_server::authentication_handler_type auth_handler) :
					mongooseplus::basic_authentication_handler("freelan"),
					m_logger(_logger),
					m_authentication_handler(auth_handler)
				{}

			protected:

				bool authenticate_from_username_and_password(mongooseplus::request& req, const std::string& username, const std::string& password) const override
				{
					if (!m_authentication_handler)
					{
						m_logger(fscp::log_level::warning) << "No authentication script defined ! Failing authentication for user \"" << username << "\".";

						return false;
					}

					if (!m_authentication_handler(username, password, req.remote(), req.remote_port()))
					{
						m_logger(fscp::log_level::warning) << "Authentication failed for user \"" << username << "\".";

						return false;
					}

					m_logger(fscp::log_level::debug) << "Authentication succeeded for user \"" << username << "\".";

					const auto session = req.get_session<session_type>();

					if (!session || (session->username() != username))
					{
						req.set_session<session_type>(username);
					}

					return true;
				}

			private:
				fscp::logger& m_logger;
				web_server::authentication_handler_type m_authentication_handler;
		};

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

		kfather::object_type to_json(const std::map<fscp::hash_type, std::set<asiotap::endpoint>>& contacts)
		{
			kfather::object_type result;

			for (auto&& contact : contacts)
			{
				result.items[to_json(contact.first)] = to_json(contact.second);
			}

			return result;
		}
	}

	web_server::web_server(fscp::logger& _logger, const freelan::server_configuration& configuration, authentication_handler_type authentication_handler) :
		m_logger(_logger),
		m_authentication_handler(authentication_handler)
	{
		m_logger(fscp::log_level::debug) << "Web server's listen endpoint set to " << configuration.listen_on << ".";
		set_option("listening_port", boost::lexical_cast<std::string>(configuration.listen_on));

		if (configuration.protocol == server_configuration::server_protocol_type::https)
		{
			set_certificate_and_private_key(configuration.server_certificate, configuration.server_private_key);
		}

		register_authenticated_route("/request_certificate/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();

			m_logger(fscp::log_level::debug) << session->username() << " (" << req.remote() << ") requested a certificate.";

			const cryptoplus::x509::certificate_request cert_req = cryptoplus::x509::certificate_request::from_der(req.content(), req.content_size());
			const auto certificate = sign_certificate_request(cert_req, configuration.certification_authority_certificate, configuration.certification_authority_private_key, session->username());
			const auto certificate_buffer = certificate.write_der();

			req.send_header("content-type", "application/x-x509-cert");
			req.send_data(&certificate_buffer.data()[0], certificate_buffer.data().size());

			return request_result::handled;
		});

		register_authenticated_route("/request_ca_certificate/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();

			m_logger(fscp::log_level::debug) << session->username() << " (" << req.remote() << ") requested the CA certificate.";

			const auto certificate_buffer = configuration.certification_authority_certificate.write_der();

			req.send_header("content-type", "application/x-x509-ca-cert");
			req.send_data(&certificate_buffer.data()[0], certificate_buffer.data().size());

			return request_result::handled;
		});

		register_authenticated_route("/register/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();
			auto cinfop = get_client_information(req);
			const bool registered = (cinfop != nullptr);

			if (registered)
			{
				m_logger(fscp::log_level::debug) << session->username() << " (" << req.remote() << ") asked to update his registration.";
			}
			else
			{
				m_logger(fscp::log_level::debug) << session->username() << " (" << req.remote() << ") asked to be registered.";

				cinfop = &m_client_information_map[session->username()];
			}

			const cryptoplus::x509::certificate cert = cryptoplus::x509::certificate::from_der(req.content(), req.content_size());

			cinfop->presentation = fscp::presentation_store(cert, cryptoplus::buffer());
			cinfop->expires_from_now(configuration.registration_validity_duration);

			typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adjustor;

			if (registered)
			{
				m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") registration extended until: " << local_adjustor::utc_to_local(cinfop->expiration_timestamp) << ".";
			}
			else
			{
				m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") registered until: " << local_adjustor::utc_to_local(cinfop->expiration_timestamp) << ".";
			}

			kfather::object_type result;
			result.items["expiration_timestamp"] = boost::posix_time::to_iso_extended_string(cinfop->expiration_timestamp);

			req.send_json(result);

			return request_result::handled;
		});

		register_authenticated_route("/unregister/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();
			const auto cinfop = get_client_information(req);
			const bool registered = (cinfop != nullptr);

			if (registered)
			{
				m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") asked to be unregistered.";

				m_client_information_map.erase(session->username());
			}
			else
			{
				m_logger(fscp::log_level::warning) << session->username() << " (" << req.remote() << ") asked to be unregistered but is not currently registered. Doing nothing.";
			}

			return request_result::handled;
		});

		register_authenticated_route("/set_contact_information/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();
			const auto cinfop = get_client_information(req);

			if (cinfop == nullptr)
			{
				m_logger(fscp::log_level::warning) << session->username() << " (" << req.remote() << ") tried to set his contact information without an active registration. Denying.";

				throw mongooseplus::http_error(mongooseplus::mongooseplus_error::http_400_bad_request) << mongooseplus::error_content_error_info("No active registration");
			}
			else
			{
				auto info = req.json();

				m_logger(fscp::log_level::debug) << "Raw client information: " << kfather::inline_formatter().format(info);

				const auto public_endpoints = kfather::value_cast<kfather::object_type>(info).get<kfather::array_type>("public_endpoints");

				cinfop->endpoints = std::set<asiotap::endpoint>();
				std::set<asiotap::endpoint> rejected_endpoints;

				for (auto&& endpoint_obj : public_endpoints.items)
				{
					const auto endpoint_str = kfather::value_cast<kfather::string_type>(endpoint_obj);

					try
					{
						auto endpoint = boost::lexical_cast<asiotap::endpoint>(endpoint_str);
						endpoint = asiotap::get_default_ip_endpoint(endpoint, req.remote_ip());

						if (asiotap::is_endpoint_complete(endpoint))
						{
							cinfop->endpoints.insert(endpoint);
						}
						else
						{
							m_logger(fscp::log_level::warning) << "Not adding \"" << endpoint << "\" as a public endpoint: the endpoint is not complete.";

							rejected_endpoints.insert(endpoint);
						}
					}
					catch (std::exception& ex)
					{
						m_logger(fscp::log_level::warning) << "Unable to parse \"" << endpoint_str << "\": " << ex.what();
					}
				}

				if (cinfop->endpoints.empty())
				{
					m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") set his contact information and has no public endpoints.";
				}
				else
				{
					std::ostringstream oss;

					for (auto&& ep : cinfop->endpoints)
					{
						oss << ", " << ep;
					}

					m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") set his contact information and can be joined at: " << oss.str().substr(2);
				}

				kfather::object_type result;
				result.items["accepted_endpoints"] = to_json(cinfop->endpoints);
				result.items["rejected_endpoints"] = to_json(rejected_endpoints);

				req.send_json(result);

				return request_result::handled;
			}
		});

		register_authenticated_route("/get_contact_information/", [this, configuration](mongooseplus::request& req) {
			const auto session = req.get_session<session_type>();
			const auto cinfop = get_client_information(req);

			if (cinfop == nullptr)
			{
				m_logger(fscp::log_level::warning) << session->username() << " (" << req.remote() << ") asked for contact information without an active registration. Denying.";

				throw mongooseplus::http_error(mongooseplus::mongooseplus_error::http_400_bad_request) << mongooseplus::error_content_error_info("No active registration");
			}
			else
			{
				auto info = req.json();

				m_logger(fscp::log_level::debug) << "Raw client information: " << kfather::inline_formatter().format(info);

				std::set<fscp::hash_type> requested_contacts;

				for(auto&& hash_obj : kfather::value_cast<kfather::object_type>(info).get<kfather::array_type>("requested_contacts").items)
				{
					const std::string hash_b64 = kfather::value_cast<std::string>(hash_obj);

					try
					{
						const auto hash_buf = cryptoplus::base64_decode(hash_b64);

						if (hash_buf.data().size() != sizeof(fscp::hash_type))
						{
							throw std::runtime_error("Invalid hash size");
						}

						fscp::hash_type hash;
						std::copy_n(hash_buf.data().begin(), sizeof(hash.data), hash.data.begin());
						requested_contacts.insert(hash);
					}
					catch (std::exception& ex)
					{
						m_logger(fscp::log_level::warning) << "Unable to read base 64 encoded DER certificate hash from \"" << hash_b64 << "\": " << ex.what();
					}
				}

				std::map<fscp::hash_type, std::set<asiotap::endpoint>> contacts;

				if (requested_contacts.empty())
				{
					m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") requested all contact information.";

					for (auto&& cinfo_pair : m_client_information_map)
					{
						if (cinfo_pair.first != session->username())
						{
							const auto& hash = cinfo_pair.second.presentation.signature_certificate_hash();

							if (hash)
							{
								contacts[*hash] = cinfo_pair.second.endpoints;
							}
						}
					}
				}
				else
				{
					std::ostringstream oss;

					for (auto&& hash : requested_contacts)
					{
						oss << ", " << hash;
					}

					m_logger(fscp::log_level::information) << session->username() << " (" << req.remote() << ") requested contact information for: " << oss.str().substr(2);

					for (auto&& cinfo_pair : m_client_information_map)
					{
						if (cinfo_pair.first != session->username())
						{
							const auto& hash = cinfo_pair.second.presentation.signature_certificate_hash();

							if (hash && (requested_contacts.find(*hash) != requested_contacts.end()))
							{
								contacts[*hash] = cinfo_pair.second.endpoints;
							}
						}
					}
				}

				kfather::object_type result;

				result.items["contacts"] = to_json(contacts);

				req.send_json(result);

				return request_result::handled;
			}
		});
	}

	web_server::route_type& web_server::register_authenticated_route(route_type&& route)
	{
		return register_route(route).set_authentication_handler<external_authentication_handler>(m_logger, m_authentication_handler);
	}

	web_server::request_result web_server::handle_request(mongooseplus::request& req)
	{
		if (m_logger.level() <= fscp::log_level::debug)
		{
			m_logger(fscp::log_level::information) << "Web server - Received " << req.request_method() << " request from " << req.remote() << " for " << req.uri() << " (" << req.content_size() << " byte(s) content).";
			m_logger(fscp::log_level::debug) << "--- Headers follow ---";

			for (auto&& header : req.get_headers())
			{
				m_logger(fscp::log_level::debug) << header.key() << ": " << header.value();
			}

			m_logger(fscp::log_level::debug) << "--- End of headers ---";
		}

		return mongooseplus::routed_web_server::handle_request(req);
	}

	web_server::request_result web_server::handle_http_error(mongooseplus::request& req)
	{
		m_logger(fscp::log_level::warning) << "Web server - Sending back " << req.status_code() << " to " << req.remote() << ".";

		return mongooseplus::routed_web_server::handle_http_error(req);
	}

	web_server::client_information_type* web_server::get_client_information(mongooseplus::request& req)
	{
		const auto session = req.get_session<session_type>();
		const auto cinfop = m_client_information_map.find(session->username());

		if (cinfop != m_client_information_map.end())
		{
			if (cinfop->second.has_expired())
			{
				m_client_information_map.erase(cinfop);

				m_logger(fscp::log_level::warning) << "Registration for " << req.remote() << " (" << session->username() << ") just expired.";
			}
			else
			{
				return &cinfop->second;
			}
		}

		return nullptr;
	}
}

#endif

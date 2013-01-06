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
 * \file client.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A client implementation.
 */

#ifndef FREELAN_CLIENT_HPP
#define FREELAN_CLIENT_HPP

#include <string>
#include <map>

#include <boost/asio.hpp>

#include <kfather/kfather.hpp>
#include <kfather/value.hpp>

#include "ip_network_address.hpp"
#include "curl.hpp"

namespace cryptoplus
{
	namespace x509
	{
		class certificate;
		class certificate_request;
	}
}

namespace freelan
{
	class configuration;
	class logger;
	
	/**
	 * \brief A network information class.
	 */
	struct network_info
	{
		ipv4_network_address ipv4_address_prefix_length;
		ipv6_network_address ipv6_address_prefix_length;
		std::vector<cryptoplus::x509::certificate> users_certificates;
	};

	/**
	 * \brief The generic network info type.
	 */
	typedef network_info network_info_v1;

	/**
	 * \brief A class that handles connection to a freelan server.
	 */
	class client
	{
		public:

			/**
			 * \brief A values type.
			 */
			typedef json::object_type values_type;

			/**
			 * \brief Create a client instance.
			 * \param configuration The configuration to use.
			 * \param _logger The logger to use.
			 */
			client(const freelan::configuration& configuration, freelan::logger& _logger);

			/**
			 * \brief Perform an authentication.
			 */
			void authenticate();

			/**
			 * \brief Get the authority certificate.
			 */
			cryptoplus::x509::certificate get_authority_certificate();

			/**
			 * \brief Join a network.
			 * \param network The network name.
			 * \return The network authority certificate.
			 */
			network_info join_network(const std::string& network);

			/**
			 * \brief Renew the certificate.
			 */
			cryptoplus::x509::certificate renew_certificate(const cryptoplus::x509::certificate_request& csr);

		private:

			client(const client&);
			client& operator=(const client&);

			void perform_request(curl&, const std::string&, values_type&);
			void perform_get_request(curl&, const std::string&, values_type&);
			void perform_post_request(curl&, const std::string&, const values_type&, values_type&);
			void get_server_information(curl&, std::string&, unsigned int&, unsigned int&, std::string&, std::string&, std::string&, std::string&);

			// Version 1 methods
			void v1_authenticate(curl&, const std::string&);
			cryptoplus::x509::certificate v1_get_authority_certificate(curl&, const std::string&);
			network_info_v1 v1_join_network(curl&, const std::string&, const std::string&);
			cryptoplus::x509::certificate v1_sign_certificate_request(curl&, const std::string&, const cryptoplus::x509::certificate_request&);

			// Version 1 sub-methods
			void v1_get_server_login(curl&, const std::string&, std::string&);
			void v1_post_server_login(curl&, const std::string&, const std::string&);

			size_t read_data(boost::asio::const_buffer buf);

			const configuration& m_configuration;
			logger& m_logger;
			std::string m_server_name;
			unsigned int m_server_version_major;
			unsigned int m_server_version_minor;
			std::string m_login_url;
			std::string m_get_authority_certificate_url;
			std::string m_join_network_url;
			std::string m_sign_url;
			curl m_request;
			const std::string m_scheme;
			std::string m_data;
	};

}

#endif /* FREELAN_CLIENT_HPP */

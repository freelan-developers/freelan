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

namespace cryptoplus
{
	namespace x509
	{
		class certificate_request;
	}
}

namespace freelan
{
	class configuration;
	class logger;
	class curl;
	
	/**
	 * \brief A class that handles connection to a freelan server.
	 */
	class client
	{
		public:

			/**
			 * \brief A values type.
			 */
			typedef std::map<std::string, std::string> values_type;

			/**
			 * \brief Create a client instance.
			 * \param configuration The configuration to use.
			 * \param _logger The logger to use.
			 */
			client(freelan::configuration& configuration, freelan::logger& _logger);

			/**
			 * \brief Connects to the server and perform an authentication.
			 */
			void connect();

		private:

			client(const client&);
			client& operator=(const client&);

			void configure_request(curl&);
			void perform_request(curl&, const std::string&, values_type&);
			void perform_get_request(curl&, const std::string&, values_type&);
			void perform_post_request(curl&, const std::string&, const values_type&, values_type&);
			void get_server_information(curl&, std::string&, unsigned int&, unsigned int&, std::string&, std::string&);

			// Version 1 methods
			void v1_authenticate(curl&, const std::string&);
			void v1_sign_certificate_request(curl&, const std::string&, cryptoplus::x509::certificate_request& csr);

			// Version 1 sub-methods
			void v1_get_server_login(curl&, const std::string&, std::string&);
			void v1_post_server_login(curl&, const std::string&, const std::string&);

			size_t read_data(boost::asio::const_buffer buf);

			configuration& m_configuration;
			logger& m_logger;
			const std::string m_scheme;
			std::string m_data;
	};

}

#endif /* FREELAN_CLIENT_HPP */

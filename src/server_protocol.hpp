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
 * \file server_protocol.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The server protocol utility classes and methods.
 */

#ifndef FREELAN_SERVER_PROTOCOL_HPP
#define FREELAN_SERVER_PROTOCOL_HPP

#include <boost/asio.hpp>

namespace freelan
{
	/**
	 * \brief The server protocol parser class.
	 *
	 * Helps in parsing the server protocol data.
	 *
	 * Raises events as the data gets parsed.
	 */
	class server_protocol_parser
	{
		public:

			/**
			 * \brief Feed some data.
			 * \param buf The received data.
			 * \return The count of bytes that were correctly fed.
			 */
			size_t feed(boost::asio::const_buffer buf);

			/**
			 * \brief Parse the content according to the specified MIME type.
			 * \param mime_type The MIME type.
			 *
			 * Throws a std::runtime_error on error.
			 */
			void parse(const std::string& mime_type);

		private:

			std::string m_data;
	};
}

#endif /* FREELAN_SERVER_PROTOCOL_HPP */

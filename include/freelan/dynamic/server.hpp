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
 * \file server.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#ifndef FREELAN_DYNAMIC_SERVER_HPP
#define FREELAN_DYNAMIC_SERVER_HPP

#include <fscp/server.hpp>

namespace freelan
{
	namespace dynamic
	{
		/**
		 * \brief A dynamic server.
		 *
		 * This class is *NOT* thread-safe !
		 *
		 * Once an instance is running into a io_service, calling any method (except close()) is undefined behavior.
		 */
		class server
		{
			public:

				/**
				 * \brief The endpoint type.
				 */
				typedef fscp::server::ep_type ep_type;

				/**
				 * \brief The send data callback.
				 * \param target The target host.
				 * \param data The data to send.
				 */
				typedef boost::function<void (const ep_type& target, boost::asio::const_buffer data)> send_data_callback;

				/**
				 * \brief Create a new dynamic server.
				 * \param server The FSCP server to use.
				 */
				server(fscp::server& server);

				/**
				 * \brief Set the send data callback.
				 * \param callback The callback.
				 */
				void set_send_data_callback(send_data_callback callback);

				/**
				 * \brief Receive some data/
				 * \param sender The sender.
				 * \param data The data to receive.
				 */
				void receive_data(const ep_type& sender, boost::asio::const_buffer data);

			private: // Generic network stuff

				void send_data(const ep_type&, boost::asio::const_buffer);

				fscp::server& m_server;
				send_data_callback m_send_data_callback;
		};

		inline void server::set_send_data_callback(send_data_callback callback)
		{
			m_send_data_callback = callback;
		}
	}
}

#endif /* FREELAN_DYNAMIC_SERVER_HPP */

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
 * \file core.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The freelan core class.
 */

#ifndef FREELAN_CORE_HPP
#define FREELAN_CORE_HPP

#include <boost/asio.hpp>

#include <asiotap/asiotap.hpp>
#include <fscp/fscp.hpp>

#include "configuration.hpp"

namespace freelan
{
	/**
	 * \brief The core class.
	 */
	class core
	{
		public:
	
			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef fscp::server::cert_type cert_type;

			/**
			 * \brief The identity store type.
			 */
			typedef fscp::identity_store identity_store;

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to bind to.
			 * \param configuration The configuration to use.
			 */
			core(boost::asio::io_service& io_service, const configuration& configuration);

			/**
			 * \brief Get the configuration.
			 * \return The current configuration.
			 */
			const configuration& configuration_() const;
	
			/**
			 * \brief Get the associated tap adapter.
			 * \return The associated tap adapter.
			 */
			asiotap::tap_adapter& tap_adapter();

			/**
			 * \brief Get the associated server.
			 * \return The associated server.
			 */
			fscp::server& server();

		private:
	
			configuration m_configuration;
			asiotap::tap_adapter m_tap_adapter;
			fscp::server m_server;
	};
	
	inline const configuration& core::configuration_() const
	{
		return m_configuration;
	}

	inline asiotap::tap_adapter& core::tap_adapter()
	{
		return m_tap_adapter;
	}

	inline fscp::server& core::server()
	{
		return m_server;
	}
}

#endif /* FREELAN_CORE_HPP */

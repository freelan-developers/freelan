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
 * \file dynamic_contact_list.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A dynamic contact list class.
 */

#ifndef DYNAMIC_CONTACT_LIST_HPP
#define DYNAMIC_CONTACT_LIST_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <map>

#include "constants.hpp"
#include "dynamic_contact.hpp"

namespace freelan
{
	class dynamic_contact_list
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
			 * \brief Check the existence of a contact.
			 * \param cert The certificate of the contact.
			 * \return true if the contact exists, false otherwise.
			 */
			bool has_contact(cert_type cert) { return m_contact_map.find(hash(cert)) != m_contact_map.end(); }

			/**
			 * \brief Get a contact.
			 * \param cert The certificate of the contact.
			 * \return The contact.
			 *
			 * If the contact doesn't exist, an empty contact will be created.
			 *
			 * To check for a contact existence, see has_contact().
			 */
			dynamic_contact& get_contact(cert_type cert) { return m_contact_map[hash(cert)]; }

			/**
			 * \brief Get the candidate endpoint list.
			 * \return The candidate endpoint list.
			 */
			std::vector<ep_type> get_candidate_endpoint_list();

		private:

			typedef std::map<hash_type, dynamic_contact> contact_map_type;

			static hash_type hash(cert_type cert);

			contact_map_type m_contact_map;
	};
}

#endif /* DYNAMIC_CONTACT_LIST_HPP */

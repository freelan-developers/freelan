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
 * \file dynamic_contact_list.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A dynamic contact list class.
 */

#include "dynamic_contact_list.hpp"

#include <cryptoplus/hash/message_digest_context.hpp>

namespace freelan
{
	namespace
	{
		enum dcl_message_type
		{
			DCL_ENDPOINT_REQUEST = 0x00,
			DCL_ENDPOINT = 0x01
		};
	}

	dynamic_contact_list::hash_type dynamic_contact_list::hash(cert_type cert)
	{
		hash_type result;

		const std::vector<unsigned char> der = cert.write_der();

		cryptoplus::hash::message_digest_context mdctx;
		mdctx.initialize(cryptoplus::hash::message_digest_algorithm(NID_sha256));
		mdctx.update(&der[0], der.size());
		mdctx.finalize(result.c_array(), result.size());

		return result;
	}

	std::vector<dynamic_contact_list::ep_type> dynamic_contact_list::get_candidate_endpoint_list()
	{
		std::vector<ep_type> result;

		for (contact_map_type::iterator it = m_contact_map.begin(); it != m_contact_map.end(); ++it)
		{
			if (!it->second.has_associated_endpoint())
			{
				const std::vector<ep_type> v = it->second.get_candidate_endpoint_list();

				result.insert(result.end(), v.begin(), v.end());
			}
		}

		return result;
	}
}

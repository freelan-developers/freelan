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
 * \brief A dynamic contact class.
 */

#ifndef DYNAMIC_CONTACT_HPP
#define DYNAMIC_CONTACT_HPP

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fscp/fscp.hpp>

namespace freelan
{
	class dynamic_contact
	{
		public:

			/**
			 * \brief The default endpoint validity duration.
			 */
			static const boost::posix_time::time_duration ENDPOINT_VALIDITY_DURATION;

			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief Set the associated endpoint.
			 * \param ep The endpoint.
			 */
			void reset(const ep_type& ep) { m_associated_endpoint = ep; }

			/**
			 * \brief Reset the associated endpoint.
			 */
			void reset() { m_associated_endpoint == boost::none; }

			/**
			 * \brief Add a candidate endpoint.
			 * \param ep The candidate endpoint.
			 */
			void add_candidate_endpoint(const ep_type& ep, boost::posix_time::ptime expiration_date = boost::posix_time::second_clock::local_time() + ENDPOINT_VALIDITY_DURATION);

			/**
			 * \brief Get the candidate endpoint list.
			 * \return The candidate endpoint list.
			 */
			const std::vector<ep_type>& get_candidate_endpoint_list();

		private:

			boost::optional<ep_type> m_associated_endpoint;
			std::map<ep_type, boost::posix_time::ptime> m_candidate_endpoint_map;
			std::vector<ep_type> m_candidate_endpoint_list_cache;
	};
}

#endif /* DYNAMIC_CONTACT_HPP */

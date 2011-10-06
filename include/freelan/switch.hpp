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
 * \file switch.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A switch class.
 */

#ifndef SWITCH_HPP
#define SWITCH_HPP

#include <map>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fscp/server.hpp>

namespace freelan
{
	/**
	 * \brief A class that represents a switch.
	 */
	class switch_
	{
		public:

			/**
			 * \brief The default maximum number of entries.
			 */
			static const unsigned int DEFAULT_MAX_ENTRIES = 500;

			/**
			 * \brief The ethernet address type.
			 */
			typedef boost::asio::const_buffer ethernet_address_type;

			/**
			 * \brief The endpoint type.
			 */
			typedef fscp::server::ep_type ep_type;

			/**
			 * \brief The entry type.
			 */
			struct entry_type
			{
				/**
				 * \brief Create a new entry.
				 * \param endpoint The endpoint.
				 * \param date The date.
				 */
				entry_type(ep_type endpoint, boost::posix_time::ptime date = boost::posix_time::second_clock::local_time());

				ep_type endpoint; /**< \brief The associated endpoint. */
				boost::posix_time::ptime date; /**< \brief The entry date. */
			};

			/**
			 * \brief Create a new switch.
			 * \param max_entries The maximum number of entries.
			 */
			switch_(unsigned int max_entries = DEFAULT_MAX_ENTRIES);

			/**
			 * \brief Update an entry.
			 * \param address The ethernet address of the entry.
			 * \param endpoint The associated endpoint.
			 * \return true if the entry was just created.
			 */
			bool update_entry(const ethernet_address_type& address, const ep_type& endpoint);

			/**
			 * \brief Get an entry.
			 * \param address The address of the entry.
			 * \param endpoint The endpoint value to update if an entry is found.
			 * \return true if an entry was found, false otherwise.
			 */
			bool get_entry(const ethernet_address_type& address, ep_type& endpoint);

		private:

			struct ethernet_address_comp
			{
				bool operator() (const ethernet_address_type& lhs, const ethernet_address_type& rhs) const;
			};

			typedef std::map<ethernet_address_type, entry_type, ethernet_address_comp> map_type;

			void remove_older_entry();

			unsigned int m_max_entries;
			map_type m_map;
	};

	inline switch_::entry_type::entry_type(ep_type _endpoint, boost::posix_time::ptime _date) :
		endpoint(_endpoint),
		date(_date)
	{
	}

	inline switch_::switch_(unsigned int max_entries) :
		m_max_entries(max_entries)
	{
	}
}

#endif /* SWITCH_HPP */


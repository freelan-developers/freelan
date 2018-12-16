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
 * \file port_index.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A port index class.
 */

#ifndef PORT_INDEX_HPP
#define PORT_INDEX_HPP

#include <asiotap/asiotap.hpp>

#include <fscp/server.hpp>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

#include <cassert>

namespace freelan
{
	/**
	 * \brief A null port index.
	 */
	class null_port_index_type
	{
		public:

			/**
			 * \brief Create a null port index.
			 */
			null_port_index_type() {};

		private:

			friend bool operator<(const null_port_index_type&, const null_port_index_type&)
			{
				return false;
			}

			friend bool operator==(const null_port_index_type&, const null_port_index_type&)
			{
				return true;
			}

			friend bool operator!=(const null_port_index_type&, const null_port_index_type&)
			{
				return false;
			}

			friend std::ostream& operator<<(std::ostream& os, const null_port_index_type&)
			{
				return os << "null()";
			}
	};

	/**
	 * \brief A tap adapter port index.
	 */
	class tap_adapter_port_index_type
	{
		public:

			/**
			 * \brief Create a new tap_adapter_port_index.
			 * \param _tap_adapter The tap adapter.
			 */
			tap_adapter_port_index_type(boost::shared_ptr<asiotap::tap_adapter> _tap_adapter) :
				m_tap_adapter(_tap_adapter)
			{
				assert(m_tap_adapter);
			}

			/**
			 * \brief Get the associated tap adapter.
			 * \return The associated tap adapter.
			 */
			boost::shared_ptr<asiotap::tap_adapter> tap_adapter() const { return m_tap_adapter; }

		private:

			boost::shared_ptr<asiotap::tap_adapter> m_tap_adapter;

			friend bool operator<(const tap_adapter_port_index_type& lhs, const tap_adapter_port_index_type& rhs)
			{
				return (lhs.m_tap_adapter < rhs.m_tap_adapter);
			}

			friend bool operator==(const tap_adapter_port_index_type& lhs, const tap_adapter_port_index_type& rhs)
			{
				return (lhs.m_tap_adapter == rhs.m_tap_adapter);
			}

			friend bool operator!=(const tap_adapter_port_index_type& lhs, const tap_adapter_port_index_type& rhs)
			{
				return !(lhs.m_tap_adapter == rhs.m_tap_adapter);
			}

			friend std::ostream& operator<<(std::ostream& os, const tap_adapter_port_index_type& idx)
			{
				return os << "tap_adapter(" << *idx.m_tap_adapter << ")";
			}
	};

	/**
	 * \brief An endpoint port index.
	 */
	class endpoint_port_index_type
	{
		public:

			/**
			 * \brief Create a new endpoint_port_index_type.
			 * \param ep The endpoint.
			 */
			endpoint_port_index_type(const fscp::server::ep_type& ep) :
				m_ep(ep)
			{}

			/**
			 * \brief Get the associated tap adapter.
			 * \return The associated tap adapter.
			 */
			fscp::server::ep_type endpoint() const { return m_ep; }

			friend bool operator<(const endpoint_port_index_type& lhs, const endpoint_port_index_type& rhs)
			{
				return (lhs.m_ep < rhs.m_ep);
			}

			friend bool operator==(const endpoint_port_index_type& lhs, const endpoint_port_index_type& rhs)
			{
				return (lhs.m_ep == rhs.m_ep);
			}

			friend bool operator!=(const endpoint_port_index_type& lhs, const endpoint_port_index_type& rhs)
			{
				return !(lhs.m_ep == rhs.m_ep);
			}

			friend std::ostream& operator<<(std::ostream& os, const endpoint_port_index_type& idx)
			{
				return os << "endpoint(" << idx.m_ep << ")";
			}

		private:

			fscp::server::ep_type m_ep;
	};

	/**
	 * \brief The generic port index type.
	 */
	typedef boost::variant<null_port_index_type, tap_adapter_port_index_type, endpoint_port_index_type> port_index_type;

	inline port_index_type make_port_index(boost::shared_ptr<asiotap::tap_adapter> tap_adapter)
	{
		return tap_adapter_port_index_type(tap_adapter);
	}

	inline port_index_type make_port_index(const fscp::server::ep_type& ep)
	{
		return endpoint_port_index_type(ep);
	}
}

#endif /* PORT_INDEX_HPP */


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
 * \file tap_adapter_switch_port.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A tap adapter switch port class.
 */

#ifndef TAP_ADAPTER_SWITCH_PORT_HPP
#define TAP_ADAPTER_SWITCH_PORT_HPP

#include "switch_port.hpp"

#include <asiotap/tap_adapter.hpp>

namespace freelan
{
	/**
	 * \brief A switch port bound to a tap adapter.
	 */
	class tap_adapter_switch_port : public switch_port
	{
		public:

			/**
			 * \brief Create a switch port bound to the specified tap adapter.
			 * \param tap_adapter The tap adapter to bind to. The instance must
			 * remain valid during the lifetime of the tap_adapter_switch_port
			 * instance.
			 */
			tap_adapter_switch_port(asiotap::tap_adapter& tap_adapter);

		protected:

			/**
			 * \brief Send data trough the port.
			 * \param data The data to send trough the port.
			 */
			void write(boost::asio::const_buffer data);

			/**
			 * \brief Check if the instance is equal to another.
			 * \param other The other instance to test for equality.
			 * \return true if the two instances are equal. Two instances of different subtypes are never equal.
			 */
			bool equals(const switch_port& other) const;

			/**
			 * \brief Output the name of the switch port to an output stream.
			 * \param os The output stream.
			 * \return os.
			 */
			std::ostream& output(std::ostream& os) const;

		private:

			asiotap::tap_adapter& m_tap_adapter;

			friend bool operator==(const tap_adapter_switch_port&, const tap_adapter_switch_port&);
	};

	/**
	 * \brief Test two tap_adapter_switch_port for equality.
	 * \param lhs The left argument.
	 * \param rhs The right argument.
	 * \return true if lhs and rhs have the exact same attributes.
	 */
	bool operator==(const tap_adapter_switch_port& lhs, const tap_adapter_switch_port& rhs);

	inline tap_adapter_switch_port::tap_adapter_switch_port(asiotap::tap_adapter& tap_adapter) :
		m_tap_adapter(tap_adapter)
	{
	}

	inline void tap_adapter_switch_port::write(boost::asio::const_buffer data)
	{
		m_tap_adapter.write(data);
	}

	inline std::ostream& tap_adapter_switch_port::output(std::ostream& os) const
	{
		return os << "Tap adapter (" << m_tap_adapter.name() << ")";
	}

	inline bool operator==(const tap_adapter_switch_port& lhs, const tap_adapter_switch_port& rhs)
	{
		return (&lhs.m_tap_adapter == &rhs.m_tap_adapter);
	}
}

#endif /* TAP_ADAPTER_SWITCH_PORT_HPP */


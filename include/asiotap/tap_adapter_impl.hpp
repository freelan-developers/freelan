/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file tap_adapter_impl.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The tap adapter implementation class.
 */

#ifndef ASIOTAP_TAP_ADAPTER_IMPL_HPP
#define ASIOTAP_TAP_ADAPTER_IMPL_HPP

#include "os.hpp"

#ifdef WINDOWS
#include <windows.h>
#else
#endif

#include <string>

namespace asiotap
{
	class tap_adapter_impl
	{
		public:

			/**
			 * \brief Create a new tap_adapter_impl.
			 */
			tap_adapter_impl();

			/**
			 * \brief Destroy a tap_adapter_impl.
			 */
			~tap_adapter_impl();

			/**
			 * \brief Check if the tap adapter is open.
			 * \return true if the tap adapter is open, false otherwise.
			 */
			bool is_open() const;

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter device. On Windows, a GUID is expected.
			 *
			 * If the tap adapter was already opened, it will be closed first.
			 */
			void open(const std::string& name);

			/**
			 * \brief Close the tap adapter.
			 * 
			 * If the tap adapter is already closed, nothing is done.
			 */
			void close();

		private:

			std::string m_name;
			unsigned int m_mtu;
#ifdef WINDOWS
			HANDLE m_handle;
			std::string m_display_name;
			DWORD m_interface_index;
#else
#endif
	};
	
	inline tap_adapter_impl::~tap_adapter_impl()
	{
		close();
	}
}

#endif /* ASIOTAP_TAP_ADAPTER_IMPL_HPP */

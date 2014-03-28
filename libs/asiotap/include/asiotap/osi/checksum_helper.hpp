/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
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
 * \file checksum_helper.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Checksum helper class.
 */

#ifndef ASIOTAP_OSI_CHECKSUM_HELPER_HPP
#define ASIOTAP_OSI_CHECKSUM_HELPER_HPP

#include <boost/asio.hpp>

namespace asiotap
{
	namespace osi
	{
		/**
		 * \brief A checksum helper class.
		 */
		class checksum_helper
		{
			public:

				/**
				 * \brief Create a new checksum helper.
				 */
				checksum_helper();

				/**
				 * \brief Update the checksum.
				 * \param buf The buffer to compute the checksum from.
				 * \param buf_len The size of buf.
				 */
				void update(const uint16_t* buf, size_t buf_len);

				/**
				 * \brief Compute the checksum.
				 * \return The computed checksum.
				 */
				uint32_t compute();

			private:

				uint32_t m_checksum;
				uint8_t m_left;
		};

		inline checksum_helper::checksum_helper() :
			m_checksum(0),
			m_left(0)
		{
		}
	}
}

#endif /* ASIOTAP_OSI_CHECKSUM_HELPER_HPP */


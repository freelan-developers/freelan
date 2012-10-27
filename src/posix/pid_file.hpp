/*
 * freelan - An open, multi-platform software to establish peer-to-peer virtual
 * private networks.
 *
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of freelan.
 *
 * freelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * freelan is distributed in the hope that it will be useful, but
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
 * If you intend to use freelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file pid_file.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A PID file handling class.
 */

#ifndef PID_FILE_HPP
#define PID_FILE_HPP

#include <boost/filesystem/path.hpp>

namespace posix
{
	class pid_file
	{
		public:

			/**
			 * \brief Create a PID file.
			 * \param path The path to the PID file to create.
			 *
			 * If the specified file already exists, creation fails.
			 */
			pid_file(const boost::filesystem::path& path);

			/**
			 * \brief Destroy the PID file.
			 */
			~pid_file();

			/**
			 * \brief Get the associated file descriptor.
			 * \return The associated file descriptor.
			 */
			int file_descriptor() const
			{
				return m_file_descriptor;
			}

			/**
			 * \brief Write the PID to the PID file.
			 */
			void write_pid() const;

		private:

			pid_file(const pid_file&);
			pid_file& operator=(const pid_file&);

			boost::filesystem::path m_file_path;
			int m_file_descriptor;
	};
}

#endif /* PID_FILE_HPP */


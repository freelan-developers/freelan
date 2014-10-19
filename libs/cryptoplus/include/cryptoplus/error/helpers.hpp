/*
 * libcryptoplus - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libcryptoplus.
 *
 * libcryptoplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libcryptoplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libcryptoplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file helpers.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief Error helpers.
 */

#pragma once

#include "error.hpp"

namespace cryptoplus
{
	/**
	 * \brief Throw an exception for the first available cryptographic error in the error queue.
	 */
	inline void throw_error()
	{
		throw boost::system::system_error(make_error_code(error::get_error()));
	}

	/**
	 * \brief Throw an exception for the first available cryptographic error in the error queue if the condition succeeds.
	 * \param condition The condition.
	 */
	inline void throw_error_if(bool condition)
	{
		if (condition) throw_error();
	}

	/**
	 * \brief Throw an exception for the first available cryptographic error in the error queue if the condition fails.
	 * \param condition The condition.
	 */
	inline void throw_error_if_not(bool condition)
	{
		if (!condition) throw_error();
	}

	/**
	 * \brief Throw an exception for the first available cryptographic error in the error queue if the specified pointer is NULL.
	 * \param ptr The pointer to test.
	 */
	inline void throw_error_if_not(const void* ptr)
	{
		if (ptr == NULL) throw_error();
	}
}

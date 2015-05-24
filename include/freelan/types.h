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
 * \file types.h
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Value-types using throughout FreeLAN.
 */

#ifndef FREELAN_TYPES_H
#define FREELAN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "error.h"

/* API BEGIN */

struct IPv4Address;
struct IPv6Address;

/**
 * \brief Create an IPv4Address instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Address* freelan_IPv4Address_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an IPv4Address instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Address_to_string(struct ErrorContext* ectx, const struct IPv4Address* inst);

/**
 * \brief Delete an IPv4Address instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Address_free(struct IPv4Address* inst);

/**
 * \brief Compare two IPv4Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Address_less_than(const struct IPv4Address* lhs, const struct IPv4Address* rhs);

/**
 * \brief Compare two IPv4Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Address_equal(const struct IPv4Address* lhs, const struct IPv4Address* rhs);

/**
 * \brief Create an IPv6Address instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv6 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Address* freelan_IPv6Address_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an IPv6Address instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Address_to_string(struct ErrorContext* ectx, const struct IPv6Address* inst);

/**
 * \brief Delete an IPv6Address instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Address_free(struct IPv6Address* inst);

/**
 * \brief Compare two IPv6Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Address_less_than(const struct IPv6Address* lhs, const struct IPv6Address* rhs);

/**
 * \brief Compare two IPv6Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Address_equal(const struct IPv6Address* lhs, const struct IPv6Address* rhs);

/* API END */

#ifdef __cplusplus
}
#endif

#endif

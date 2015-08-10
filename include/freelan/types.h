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

struct freelan_EthernetAddress;
struct freelan_IPv4Address;
struct freelan_IPv6Address;
struct freelan_Hostname;
struct freelan_PortNumber;
struct freelan_IPv4PrefixLength;
struct freelan_IPv6PrefixLength;
struct freelan_IPv4Endpoint;
struct freelan_IPv6Endpoint;
struct freelan_HostnameEndpoint;
struct freelan_IPv4Route;
struct freelan_IPv6Route;
struct freelan_IPAddress;
struct freelan_IPRoute;
struct freelan_Host;
struct freelan_IPEndpoint;
struct freelan_HostEndpoint;

/**
 * \brief Create an freelan_EthernetAddress instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_EthernetAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_EthernetAddress* freelan_EthernetAddress_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_EthernetAddress instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_EthernetAddress_to_string(struct freelan_ErrorContext* ectx, const struct freelan_EthernetAddress* inst);

/**
 * \brief Clone an existing freelan_EthernetAddress instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4 address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_EthernetAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_EthernetAddress* freelan_EthernetAddress_clone(struct freelan_ErrorContext* ectx, const struct freelan_EthernetAddress* inst);

/**
 * \brief Delete an freelan_EthernetAddress instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_EthernetAddress_free(struct freelan_EthernetAddress* inst);

/**
 * \brief Compare two freelan_EthernetAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_EthernetAddress_less_than(const struct freelan_EthernetAddress* lhs, const struct freelan_EthernetAddress* rhs);

/**
 * \brief Compare two freelan_EthernetAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_EthernetAddress_equal(const struct freelan_EthernetAddress* lhs, const struct freelan_EthernetAddress* rhs);

/**
 * \brief Create an freelan_IPv4Address instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Address* freelan_IPv4Address_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_IPv4Address instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Address_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Address* inst);

/**
 * \brief Clone an existing freelan_IPv4Address instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4 address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Address* freelan_IPv4Address_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Address* inst);

/**
 * \brief Delete an freelan_IPv4Address instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Address_free(struct freelan_IPv4Address* inst);

/**
 * \brief Compare two freelan_IPv4Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Address_less_than(const struct freelan_IPv4Address* lhs, const struct freelan_IPv4Address* rhs);

/**
 * \brief Compare two freelan_IPv4Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Address_equal(const struct freelan_IPv4Address* lhs, const struct freelan_IPv4Address* rhs);

/**
 * \brief Create an freelan_IPv6Address instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv6 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Address* freelan_IPv6Address_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_IPv6Address instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Address_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Address* inst);

/**
 * \brief Clone an existing freelan_IPv6Address instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv6 address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Address* freelan_IPv6Address_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Address* inst);

/**
 * \brief Delete an freelan_IPv6Address instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Address_free(struct freelan_IPv6Address* inst);

/**
 * \brief Compare two freelan_IPv6Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Address_less_than(const struct freelan_IPv6Address* lhs, const struct freelan_IPv6Address* rhs);

/**
 * \brief Compare two freelan_IPv6Address instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Address_equal(const struct freelan_IPv6Address* lhs, const struct freelan_IPv6Address* rhs);

/**
 * \brief Create a freelan_Hostname instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_Hostname instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Hostname* freelan_Hostname_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_Hostname instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_Hostname_to_string(struct freelan_ErrorContext* ectx, const struct freelan_Hostname* inst);

/**
 * \brief Clone an existing freelan_Hostname instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_Hostname instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Hostname* freelan_Hostname_clone(struct freelan_ErrorContext* ectx, const struct freelan_Hostname* inst);

/**
 * \brief Delete an freelan_Hostname instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_Hostname_free(struct freelan_Hostname* inst);

/**
 * \brief Compare two freelan_Hostname instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_Hostname_less_than(const struct freelan_Hostname* lhs, const struct freelan_Hostname* rhs);

/**
 * \brief Compare two freelan_Hostname instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_Hostname_equal(const struct freelan_Hostname* lhs, const struct freelan_Hostname* rhs);

/**
 * \brief Create a freelan_PortNumber instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_PortNumber instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_PortNumber* freelan_PortNumber_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_PortNumber instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_PortNumber_to_string(struct freelan_ErrorContext* ectx, const struct freelan_PortNumber* inst);

/**
 * \brief Clone an existing freelan_PortNumber instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_PortNumber instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_PortNumber* freelan_PortNumber_clone(struct freelan_ErrorContext* ectx, const struct freelan_PortNumber* inst);

/**
 * \brief Delete an freelan_PortNumber instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_PortNumber_free(struct freelan_PortNumber* inst);

/**
 * \brief Compare two freelan_PortNumber instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_PortNumber_less_than(const struct freelan_PortNumber* lhs, const struct freelan_PortNumber* rhs);

/**
 * \brief Compare two freelan_PortNumber instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_PortNumber_equal(const struct freelan_PortNumber* lhs, const struct freelan_PortNumber* rhs);

/**
 * \brief Create a freelan_IPv4PrefixLength instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv4PrefixLength instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4PrefixLength_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_IPv4PrefixLength* freelan_IPv4PrefixLength_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_IPv4PrefixLength
 * instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4PrefixLength_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv4PrefixLength* inst);

/**
 * \brief Clone an existing freelan_IPv4PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv4PrefixLength instance. On error, a null
 * pointer is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4PrefixLength_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_IPv4PrefixLength* freelan_IPv4PrefixLength_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv4PrefixLength* inst);

/**
 * \brief Delete an freelan_IPv4PrefixLength instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4PrefixLength_free(struct freelan_IPv4PrefixLength* inst);

/**
 * \brief Compare two freelan_IPv4PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4PrefixLength_less_than(const struct freelan_IPv4PrefixLength* lhs, const struct freelan_IPv4PrefixLength* rhs);

/**
 * \brief Compare two freelan_IPv4PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4PrefixLength_equal(const struct freelan_IPv4PrefixLength* lhs, const struct freelan_IPv4PrefixLength* rhs);

/**
 * \brief Create a freelan_IPv6PrefixLength instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv6PrefixLength instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6PrefixLength_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_IPv6PrefixLength* freelan_IPv6PrefixLength_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an freelan_IPv6PrefixLength
 * instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6PrefixLength_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv6PrefixLength* inst);

/**
 * \brief Clone an existing freelan_IPv6PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv6PrefixLength instance. On error, a null
 * pointer is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6PrefixLength_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_IPv6PrefixLength* freelan_IPv6PrefixLength_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv6PrefixLength* inst);

/**
 * \brief Delete an freelan_IPv6PrefixLength instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6PrefixLength_free(struct freelan_IPv6PrefixLength* inst);

/**
 * \brief Compare two freelan_IPv6PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6PrefixLength_less_than(const struct freelan_IPv6PrefixLength* lhs, const struct freelan_IPv6PrefixLength* rhs);

/**
 * \brief Compare two freelan_IPv6PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6PrefixLength_equal(const struct freelan_IPv6PrefixLength* lhs, const struct freelan_IPv6PrefixLength* rhs);

/**
 * \brief Create a freelan_IPv4Endpoint instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv4Endpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv4Endpoint* freelan_IPv4Endpoint_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create a freelan_IPv4Endpoint instance from its parts.
 * \param ip_address The IP address component. The value is copied so the
 * instance needs not stay alive.
 * \param port_number The port number. The value is copied so the instance
 * needs not stay alive.
 * \return The freelan_IPv4Endpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv4Endpoint* freelan_IPv4Endpoint_from_parts(const struct freelan_IPv4Address* ip_address, const struct freelan_PortNumber* port_number);

/**
 * \brief Get the string representation of an freelan_IPv4Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Endpoint_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Endpoint* inst);

/**
 * \brief Clone an existing freelan_IPv4Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv4Endpoint instance. On error, a null pointer
 * is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv4Endpoint* freelan_IPv4Endpoint_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Endpoint* inst);

/**
 * \brief Delete an freelan_IPv4Endpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Endpoint_free(struct freelan_IPv4Endpoint* inst);

/**
 * \brief Compare two freelan_IPv4Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Endpoint_less_than(const struct freelan_IPv4Endpoint* lhs, const struct freelan_IPv4Endpoint* rhs);

/**
 * \brief Compare two freelan_IPv4Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Endpoint_equal(const struct freelan_IPv4Endpoint* lhs, const struct freelan_IPv4Endpoint* rhs);

/**
 * \brief Get the freelan_IPv4Address contained in a freelan_IPv4Endpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv4Address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Address* freelan_IPv4Endpoint_get_IPv4Address(const struct freelan_IPv4Endpoint* inst);

/**
 * \brief Get the freelan_PortNumber contained in a freelan_IPv4Endpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_PortNumber instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_PortNumber* freelan_IPv4Endpoint_get_PortNumber(const struct freelan_IPv4Endpoint* inst);

/**
 * \brief Create a freelan_IPv6Endpoint instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv6Endpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv6Endpoint* freelan_IPv6Endpoint_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create a freelan_IPv6Endpoint instance from its parts.
 * \param ip_address The IP address component. The value is copied so the
 * instance needs not stay alive.
 * \param port_number The port number. The value is copied so the instance
 * needs not stay alive.
 * \return The freelan_IPv6Endpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv6Endpoint* freelan_IPv6Endpoint_from_parts(const struct freelan_IPv6Address* ip_address, const struct freelan_PortNumber* port_number);

/**
 * \brief Get the string representation of an freelan_IPv6Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Endpoint_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Endpoint* inst);

/**
 * \brief Clone an existing freelan_IPv6Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv6Endpoint instance. On error, a null pointer
 * is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6Endpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_IPv6Endpoint* freelan_IPv6Endpoint_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Endpoint* inst);

/**
 * \brief Delete an freelan_IPv6Endpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Endpoint_free(struct freelan_IPv6Endpoint* inst);

/**
 * \brief Compare two freelan_IPv6Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Endpoint_less_than(const struct freelan_IPv6Endpoint* lhs, const struct freelan_IPv6Endpoint* rhs);

/**
 * \brief Compare two freelan_IPv6Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Endpoint_equal(const struct freelan_IPv6Endpoint* lhs, const struct freelan_IPv6Endpoint* rhs);

/**
 * \brief Get the freelan_IPv6Address contained in a freelan_IPv6Endpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv6Address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Address* freelan_IPv6Endpoint_get_IPv6Address(const struct freelan_IPv6Endpoint* inst);

/**
 * \brief Get the freelan_PortNumber contained in a freelan_IPv6Endpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_PortNumber instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_PortNumber* freelan_IPv6Endpoint_get_PortNumber(const struct freelan_IPv6Endpoint* inst);

/**
 * \brief Create a freelan_HostnameEndpoint instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_HostnameEndpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostnameEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_HostnameEndpoint* freelan_HostnameEndpoint_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create a freelan_HostnameEndpoint instance from its parts.
 * \param hostname The hostname component. The value is copied so the instance
 * needs not stay alive.
 * \param port_number The port number. The value is copied so the instance
 * needs not stay alive.
 * \return The freelan_HostnameEndpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostnameEndpoint_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_HostnameEndpoint* freelan_HostnameEndpoint_from_parts(const struct freelan_Hostname* hostname, const struct freelan_PortNumber* port_number);

/**
 * \brief Get the string representation of an freelan_HostnameEndpoint
 * instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_HostnameEndpoint_to_string(struct freelan_ErrorContext* ectx, const struct freelan_HostnameEndpoint* inst);

/**
 * \brief Clone an existing freelan_HostnameEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_HostnameEndpoint instance. On error, a null
 * pointer is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostnameEndpoint_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_HostnameEndpoint* freelan_HostnameEndpoint_clone(struct freelan_ErrorContext* ectx, const struct freelan_HostnameEndpoint* inst);

/**
 * \brief Delete an freelan_HostnameEndpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_HostnameEndpoint_free(struct freelan_HostnameEndpoint* inst);

/**
 * \brief Compare two freelan_HostnameEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_HostnameEndpoint_less_than(const struct freelan_HostnameEndpoint* lhs, const struct freelan_HostnameEndpoint* rhs);

/**
 * \brief Compare two freelan_HostnameEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_HostnameEndpoint_equal(const struct freelan_HostnameEndpoint* lhs, const struct freelan_HostnameEndpoint* rhs);

/**
 * \brief Get the freelan_Hostname contained in a freelan_HostnameEndpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_Hostname instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Hostname* freelan_HostnameEndpoint_get_Hostname(const struct freelan_HostnameEndpoint* inst);

/**
 * \brief Get the freelan_PortNumber contained in a freelan_HostnameEndpoint
 * instance.
 * \param inst The instance.
 * \return The freelan_PortNumber instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_PortNumber* freelan_HostnameEndpoint_get_PortNumber(const struct freelan_HostnameEndpoint* inst);

/**
 * \brief Create a freelan_IPv4Route instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv4Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Route* freelan_IPv4Route_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create a freelan_IPv4Route instance from its parts.
 * \param ip_address The IP address component. The value is copied so the
 * instance needs not stay alive.
 * \param prefix_length The prefix length. The value is copied so the instance
 * needs not stay alive.
 * \param gateway The gateway component. The value is copied so the instance
 * needs not stay alive. If null is specified, then the instance has no
 * gateway.
 * \return The freelan_IPv4Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Route* freelan_IPv4Route_from_parts(const struct freelan_IPv4Address* ip_address, const struct freelan_IPv4PrefixLength* prefix_length, const struct freelan_IPv4Address* gateway);

/**
 * \brief Get the string representation of an freelan_IPv4Route instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Route_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Route* inst);

/**
 * \brief Clone an existing freelan_IPv4Route instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv4Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Route* freelan_IPv4Route_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv4Route* inst);

/**
 * \brief Delete an freelan_IPv4Route instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Route_free(struct freelan_IPv4Route* inst);

/**
 * \brief Compare two freelan_IPv4Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Route_less_than(const struct freelan_IPv4Route* lhs, const struct freelan_IPv4Route* rhs);

/**
 * \brief Compare two freelan_IPv4Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Route_equal(const struct freelan_IPv4Route* lhs, const struct freelan_IPv4Route* rhs);

/**
 * \brief Get the freelan_IPv4Address contained in a freelan_IPv4Route
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv4Address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Address* freelan_IPv4Route_get_IPv4Address(const struct freelan_IPv4Route* inst);

/**
 * \brief Get the freelan_IPv4PrefixLength contained in a freelan_IPv4Route
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv4PrefixLength instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv4PrefixLength_free() on the returned instance when it is no
 * longer needed.
 */
FREELAN_API struct freelan_IPv4PrefixLength* freelan_IPv4Route_get_IPv4PrefixLength(const struct freelan_IPv4Route* inst);

/**
 * \brief Get the freelan_IPv4Address gateway contained in a freelan_IPv4Route
 * instance, if one is present.
 * \param inst The instance.
 * \return The freelan_IPv4Address instance. If the IP route has no gateway, a
 * null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv4Address* freelan_IPv4Route_get_IPv4Address_gateway(const struct freelan_IPv4Route* inst);

/**
 * \brief Create a freelan_IPv6Route instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The freelan_IPv6Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Route* freelan_IPv6Route_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create a freelan_IPv6Route instance from its parts.
 * \param ip_address The IP address component. The value is copied so the
 * instance needs not stay alive.
 * \param prefix_length The prefix length. The value is copied so the instance
 * needs not stay alive.
 * \param gateway The gateway component. The value is copied so the instance
 * needs not stay alive. If null is specified, then the instance has no
 * gateway.
 * \return The freelan_IPv6Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Route* freelan_IPv6Route_from_parts(const struct freelan_IPv6Address* ip_address, const struct freelan_IPv6PrefixLength* prefix_length, const struct freelan_IPv6Address* gateway);

/**
 * \brief Get the string representation of an freelan_IPv6Route instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Route_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Route* inst);

/**
 * \brief Clone an existing freelan_IPv6Route instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPv6Route instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Route* freelan_IPv6Route_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPv6Route* inst);

/**
 * \brief Delete an freelan_IPv6Route instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Route_free(struct freelan_IPv6Route* inst);

/**
 * \brief Compare two freelan_IPv6Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Route_less_than(const struct freelan_IPv6Route* lhs, const struct freelan_IPv6Route* rhs);

/**
 * \brief Compare two freelan_IPv6Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Route_equal(const struct freelan_IPv6Route* lhs, const struct freelan_IPv6Route* rhs);

/**
 * \brief Get the freelan_IPv6Address contained in a freelan_IPv6Route
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv6Address instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Address* freelan_IPv6Route_get_IPv6Address(const struct freelan_IPv6Route* inst);

/**
 * \brief Get the freelan_IPv6PrefixLength contained in a freelan_IPv6Route
 * instance.
 * \param inst The instance.
 * \return The freelan_IPv6PrefixLength instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_IPv6PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6PrefixLength* freelan_IPv6Route_get_IPv6PrefixLength(const struct freelan_IPv6Route* inst);

/**
 * \brief Get the freelan_IPv6Address gateway contained in a freelan_IPv6Route
 * instance, if one is present.
 * \param inst The instance.
 * \return The freelan_IPv6Address instance. If the IP route has no gateway, a
 * null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPv6Address* freelan_IPv6Route_get_IPv6Address_gateway(const struct freelan_IPv6Route* inst);

/**
 * \brief Create an freelan_IPAddress instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPAddress* freelan_IPAddress_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create an freelan_IPAddress instance from an freelan_IPv4Address.
 * \param value The freelan_IPv4Address instance. Cannot be NULL.
 * \return The IP address instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPAddress* freelan_IPAddress_from_IPv4Address(const struct freelan_IPv4Address* value);

/**
 * \brief Create an freelan_IPAddress instance from an freelan_IPv6Address.
 * \param value The freelan_IPv6Address instance. Cannot be NULL.
 * \return The IP address instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPAddress* freelan_IPAddress_from_IPv6Address(const struct freelan_IPv6Address* value);

/**
 * \brief Get the freelan_IPv4Address instance contained in the specified
 * freelan_IPAddress instance.
 * \param inst The freelan_IPAddress instance to get the value from.
 * \return The freelan_IPv4Address instance, or NULL if \c inst is not an
 * freelan_IPv4Address.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Address_clone().
 */
FREELAN_API const struct freelan_IPv4Address* freelan_IPAddress_as_IPv4Address(const struct freelan_IPAddress* inst);

/**
 * \brief Get the freelan_IPv6Address instance contained in the specified
 * freelan_IPAddress instance.
 * \param inst The freelan_IPAddress instance to get the value from.
 * \return The freelan_IPv6Address instance, or NULL if \c inst is not an
 * freelan_IPv6Address.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Address_clone().
 */
FREELAN_API const struct freelan_IPv6Address* freelan_IPAddress_as_IPv6Address(const struct freelan_IPAddress* inst);

/**
 * \brief Get the string representation of an freelan_IPAddress instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPAddress_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPAddress* inst);

/**
 * \brief Clone an existing freelan_IPAddress instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPAddress instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPAddress* freelan_IPAddress_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPAddress* inst);

/**
 * \brief Delete an freelan_IPAddress instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPAddress_free(struct freelan_IPAddress* inst);

/**
 * \brief Compare two freelan_IPAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPAddress_less_than(const struct freelan_IPAddress* lhs, const struct freelan_IPAddress* rhs);

/**
 * \brief Compare two freelan_IPAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPAddress_equal(const struct freelan_IPAddress* lhs, const struct freelan_IPAddress* rhs);

/**
 * \brief Create an freelan_IPRoute instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPRoute* freelan_IPRoute_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create an freelan_IPRoute instance from an freelan_IPv4Route.
 * \param value The freelan_IPv4Route instance. Cannot be NULL.
 * \return The IP route instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPRoute* freelan_IPRoute_from_IPv4Route(const struct freelan_IPv4Route* value);

/**
 * \brief Create an freelan_IPRoute instance from an freelan_IPv6Route.
 * \param value The freelan_IPv6Route instance. Cannot be NULL.
 * \return The IP route instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPRoute* freelan_IPRoute_from_IPv6Route(const struct freelan_IPv6Route* value);

/**
 * \brief Get the freelan_IPv4Route instance contained in the specified
 * freelan_IPRoute instance.
 * \param inst The freelan_IPRoute instance to get the value from.
 * \return The freelan_IPv4Route instance, or NULL if \c inst is not an
 * freelan_IPv4Route.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Route_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Route_clone().
 */
FREELAN_API const struct freelan_IPv4Route* freelan_IPRoute_as_IPv4Route(const struct freelan_IPRoute* inst);

/**
 * \brief Get the freelan_IPv6Route instance contained in the specified
 * freelan_IPRoute instance.
 * \param inst The freelan_IPRoute instance to get the value from.
 * \return The freelan_IPv6Route instance, or NULL if \c inst is not an
 * freelan_IPv6Route.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Route_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Route_clone().
 */
FREELAN_API const struct freelan_IPv6Route* freelan_IPRoute_as_IPv6Route(const struct freelan_IPRoute* inst);

/**
 * \brief Get the string representation of an freelan_IPRoute instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPRoute_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPRoute* inst);

/**
 * \brief Clone an existing freelan_IPRoute instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPRoute instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPRoute* freelan_IPRoute_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPRoute* inst);

/**
 * \brief Delete an freelan_IPRoute instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPRoute_free(struct freelan_IPRoute* inst);

/**
 * \brief Compare two freelan_IPRoute instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPRoute_less_than(const struct freelan_IPRoute* lhs, const struct freelan_IPRoute* rhs);

/**
 * \brief Compare two freelan_IPRoute instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPRoute_equal(const struct freelan_IPRoute* lhs, const struct freelan_IPRoute* rhs);

/**
 * \brief Create an freelan_Host instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The host instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Host* freelan_Host_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create an freelan_Host instance from an freelan_IPv4Address.
 * \param value The freelan_IPv4Address instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Host* freelan_Host_from_IPv4Address(const struct freelan_IPv4Address* value);

/**
 * \brief Create an freelan_Host instance from an freelan_IPv6Address.
 * \param value The freelan_IPv6Address instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Host* freelan_Host_from_IPv6Address(const struct freelan_IPv6Address* value);

/**
 * \brief Create an freelan_Host instance from a freelan_Hostname.
 * \param value The freelan_Hostname instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Host* freelan_Host_from_Hostname(const struct freelan_Hostname* value);

/**
 * \brief Get the freelan_IPv4Address instance contained in the specified
 * freelan_Host instance.
 * \param inst The freelan_Host instance to get the value from.
 * \return The freelan_IPv4Address instance, or NULL if \c inst is not an
 * freelan_IPv4Address.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Address_clone().
 */
FREELAN_API const struct freelan_IPv4Address* freelan_Host_as_IPv4Address(const struct freelan_Host* inst);

/**
 * \brief Get the freelan_IPv6Address instance contained in the specified
 * freelan_Host instance.
 * \param inst The freelan_Host instance to get the value from.
 * \return The freelan_IPv6Address instance, or NULL if \c inst is not an
 * freelan_IPv6Address.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Address_clone().
 */
FREELAN_API const struct freelan_IPv6Address* freelan_Host_as_IPv6Address(const struct freelan_Host* inst);

/**
 * \brief Get the freelan_Hostname instance contained in the specified
 * freelan_Host instance.
 * \param inst The freelan_Host instance to get the value from.
 * \return The freelan_Hostname instance, or NULL if \c inst is not a
 * freelan_Hostname.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_Hostname_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_Hostname_clone().
 */
FREELAN_API const struct freelan_Hostname* freelan_Host_as_Hostname(const struct freelan_Host* inst);

/**
 * \brief Get the string representation of an freelan_Host instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_Host_to_string(struct freelan_ErrorContext* ectx, const struct freelan_Host* inst);

/**
 * \brief Clone an existing freelan_Host instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_Host instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_Host* freelan_Host_clone(struct freelan_ErrorContext* ectx, const struct freelan_Host* inst);

/**
 * \brief Delete an freelan_Host instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_Host_free(struct freelan_Host* inst);

/**
 * \brief Compare two freelan_Host instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_Host_less_than(const struct freelan_Host* lhs, const struct freelan_Host* rhs);

/**
 * \brief Compare two freelan_Host instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_Host_equal(const struct freelan_Host* lhs, const struct freelan_Host* rhs);

/**
 * \brief Create an freelan_IPEndpoint instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPEndpoint* freelan_IPEndpoint_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create an freelan_IPEndpoint instance from an freelan_IPv4Endpoint.
 * \param value The freelan_IPv4Endpoint instance. Cannot be NULL.
 * \return The IP endpoint instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPEndpoint* freelan_IPEndpoint_from_IPv4Endpoint(const struct freelan_IPv4Endpoint* value);

/**
 * \brief Create an freelan_IPEndpoint instance from an freelan_IPv6Endpoint.
 * \param value The freelan_IPv6Endpoint instance. Cannot be NULL.
 * \return The IP endpoint instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPEndpoint* freelan_IPEndpoint_from_IPv6Endpoint(const struct freelan_IPv6Endpoint* value);

/**
 * \brief Get the freelan_IPv4Endpoint instance contained in the specified
 * freelan_IPEndpoint instance.
 * \param inst The freelan_IPEndpoint instance to get the value from.
 * \return The freelan_IPv4Endpoint instance, or NULL if \c inst is not an
 * freelan_IPv4Endpoint.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Endpoint_clone().
 */
FREELAN_API const struct freelan_IPv4Endpoint* freelan_IPEndpoint_as_IPv4Endpoint(const struct freelan_IPEndpoint* inst);

/**
 * \brief Get the freelan_IPv6Endpoint instance contained in the specified
 * freelan_IPEndpoint instance.
 * \param inst The freelan_IPEndpoint instance to get the value from.
 * \return The freelan_IPv6Endpoint instance, or NULL if \c inst is not an
 * freelan_IPv6Endpoint.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Endpoint_clone().
 */
FREELAN_API const struct freelan_IPv6Endpoint* freelan_IPEndpoint_as_IPv6Endpoint(const struct freelan_IPEndpoint* inst);

/**
 * \brief Get the string representation of an freelan_IPEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPEndpoint_to_string(struct freelan_ErrorContext* ectx, const struct freelan_IPEndpoint* inst);

/**
 * \brief Clone an existing freelan_IPEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_IPEndpoint instance. On error, a null pointer is
 * returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct freelan_IPEndpoint* freelan_IPEndpoint_clone(struct freelan_ErrorContext* ectx, const struct freelan_IPEndpoint* inst);

/**
 * \brief Delete an freelan_IPEndpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPEndpoint_free(struct freelan_IPEndpoint* inst);

/**
 * \brief Compare two freelan_IPEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPEndpoint_less_than(const struct freelan_IPEndpoint* lhs, const struct freelan_IPEndpoint* rhs);

/**
 * \brief Compare two freelan_IPEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPEndpoint_equal(const struct freelan_IPEndpoint* lhs, const struct freelan_IPEndpoint* rhs);

/**
 * \brief Create an freelan_HostEndpoint instance from its string
 * representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The host endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostEndpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_HostEndpoint* freelan_HostEndpoint_from_string(struct freelan_ErrorContext* ectx, const char* str);

/**
 * \brief Create an freelan_HostEndpoint instance from an freelan_IPv4Endpoint.
 * \param value The freelan_IPv4Endpoint instance. Cannot be NULL.
 * \return The host endpoint instance.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostEndpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_HostEndpoint* freelan_HostEndpoint_from_IPv4Endpoint(const struct freelan_IPv4Endpoint* value);

/**
 * \brief Create an freelan_HostEndpoint instance from an freelan_IPv6Endpoint.
 * \param value The freelan_IPv6Endpoint instance. Cannot be NULL.
 * \return The host endpoint instance.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostEndpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_HostEndpoint* freelan_HostEndpoint_from_IPv6Endpoint(const struct freelan_IPv6Endpoint* value);

/**
 * \brief Create an freelan_HostEndpoint instance from a
 * freelan_HostnameEndpoint.
 * \param value The freelan_HostnameEndpoint instance. Cannot be NULL.
 * \return The host endpoint instance.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostEndpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_HostEndpoint* freelan_HostEndpoint_from_HostnameEndpoint(const struct freelan_HostnameEndpoint* value);

/**
 * \brief Get the freelan_IPv4Endpoint instance contained in the specified
 * freelan_HostEndpoint instance.
 * \param inst The freelan_HostEndpoint instance to get the value from.
 * \return The freelan_IPv4Endpoint instance, or NULL if \c inst is not an
 * freelan_IPv4Endpoint.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Endpoint_clone().
 */
FREELAN_API const struct freelan_IPv4Endpoint* freelan_HostEndpoint_as_IPv4Endpoint(const struct freelan_HostEndpoint* inst);

/**
 * \brief Get the freelan_IPv6Endpoint instance contained in the specified
 * freelan_HostEndpoint instance.
 * \param inst The freelan_HostEndpoint instance to get the value from.
 * \return The freelan_IPv6Endpoint instance, or NULL if \c inst is not an
 * freelan_IPv6Endpoint.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Endpoint_clone().
 */
FREELAN_API const struct freelan_IPv6Endpoint* freelan_HostEndpoint_as_IPv6Endpoint(const struct freelan_HostEndpoint* inst);

/**
 * \brief Get the freelan_HostnameEndpoint instance contained in the specified
 * freelan_HostEndpoint instance.
 * \param inst The freelan_HostEndpoint instance to get the value from.
 * \return The freelan_HostnameEndpoint instance, or NULL if \c inst is not an
 * freelan_HostnameEndpoint.
 * The returned instance has the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_HostnameEndpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_HostnameEndpoint_clone().
 */
FREELAN_API const struct freelan_HostnameEndpoint* freelan_HostEndpoint_as_HostnameEndpoint(const struct freelan_HostEndpoint* inst);

/**
 * \brief Get the string representation of an freelan_HostEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_HostEndpoint_to_string(struct freelan_ErrorContext* ectx, const struct freelan_HostEndpoint* inst);

/**
 * \brief Clone an existing freelan_HostEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned freelan_HostEndpoint instance. On error, a null pointer
 * is returned.
 *
 * \warning The caller is responsible for calling \c
 * freelan_HostEndpoint_free() on the returned instance when it is no longer
 * needed.
 */
FREELAN_API struct freelan_HostEndpoint* freelan_HostEndpoint_clone(struct freelan_ErrorContext* ectx, const struct freelan_HostEndpoint* inst);

/**
 * \brief Delete an freelan_HostEndpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_HostEndpoint_free(struct freelan_HostEndpoint* inst);

/**
 * \brief Compare two freelan_HostEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_HostEndpoint_less_than(const struct freelan_HostEndpoint* lhs, const struct freelan_HostEndpoint* rhs);

/**
 * \brief Compare two freelan_HostEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_HostEndpoint_equal(const struct freelan_HostEndpoint* lhs, const struct freelan_HostEndpoint* rhs);

/* API END */

#ifdef __cplusplus
}
#endif

#endif

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
struct Hostname;
struct PortNumber;
struct IPv4PrefixLength;
struct IPv6PrefixLength;
struct IPv4Endpoint;
struct IPv6Endpoint;
struct HostnameEndpoint;
struct IPv4Route;
struct IPv6Route;
struct IPAddress;
struct IPRoute;
struct Host;
struct IPEndpoint;

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
 * \brief Clone an existing IPv4Address instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Address* freelan_IPv4Address_clone(struct ErrorContext* ectx, const struct IPv4Address* inst);

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
 * \brief Clone an existing IPv6Address instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv6 address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Address* freelan_IPv6Address_clone(struct ErrorContext* ectx, const struct IPv6Address* inst);

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

/**
 * \brief Create a Hostname instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The Hostname instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Hostname* freelan_Hostname_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an Hostname instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_Hostname_to_string(struct ErrorContext* ectx, const struct Hostname* inst);

/**
 * \brief Clone an existing Hostname instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned Hostname instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Hostname* freelan_Hostname_clone(struct ErrorContext* ectx, const struct Hostname* inst);

/**
 * \brief Delete an Hostname instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_Hostname_free(struct Hostname* inst);

/**
 * \brief Compare two Hostname instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_Hostname_less_than(const struct Hostname* lhs, const struct Hostname* rhs);

/**
 * \brief Compare two Hostname instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_Hostname_equal(const struct Hostname* lhs, const struct Hostname* rhs);

/**
 * \brief Create a PortNumber instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The PortNumber instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct PortNumber* freelan_PortNumber_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an PortNumber instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_PortNumber_to_string(struct ErrorContext* ectx, const struct PortNumber* inst);

/**
 * \brief Clone an existing PortNumber instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned PortNumber instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct PortNumber* freelan_PortNumber_clone(struct ErrorContext* ectx, const struct PortNumber* inst);

/**
 * \brief Delete an PortNumber instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_PortNumber_free(struct PortNumber* inst);

/**
 * \brief Compare two PortNumber instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_PortNumber_less_than(const struct PortNumber* lhs, const struct PortNumber* rhs);

/**
 * \brief Compare two PortNumber instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_PortNumber_equal(const struct PortNumber* lhs, const struct PortNumber* rhs);

/**
 * \brief Create a IPv4PrefixLength instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4PrefixLength* freelan_IPv4PrefixLength_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an IPv4PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4PrefixLength_to_string(struct ErrorContext* ectx, const struct IPv4PrefixLength* inst);

/**
 * \brief Clone an existing IPv4PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4PrefixLength* freelan_IPv4PrefixLength_clone(struct ErrorContext* ectx, const struct IPv4PrefixLength* inst);

/**
 * \brief Delete an IPv4PrefixLength instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4PrefixLength_free(struct IPv4PrefixLength* inst);

/**
 * \brief Compare two IPv4PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4PrefixLength_less_than(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);

/**
 * \brief Compare two IPv4PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4PrefixLength_equal(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);

/**
 * \brief Create a IPv6PrefixLength instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv6PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6PrefixLength* freelan_IPv6PrefixLength_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Get the string representation of an IPv6PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6PrefixLength_to_string(struct ErrorContext* ectx, const struct IPv6PrefixLength* inst);

/**
 * \brief Clone an existing IPv6PrefixLength instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv6PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6PrefixLength* freelan_IPv6PrefixLength_clone(struct ErrorContext* ectx, const struct IPv6PrefixLength* inst);

/**
 * \brief Delete an IPv6PrefixLength instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6PrefixLength_free(struct IPv6PrefixLength* inst);

/**
 * \brief Compare two IPv6PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6PrefixLength_less_than(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);

/**
 * \brief Compare two IPv6PrefixLength instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6PrefixLength_equal(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);

/**
 * \brief Create a IPv4Endpoint instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Endpoint* freelan_IPv4Endpoint_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create a IPv4Endpoint instance from its parts.
 * \param ip_address The IP address component. The value is copied so the instance needs not stay alive.
 * \param port_number The port number. The value is copied so the instance needs not stay alive.
 * \return The IPv4Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Endpoint* freelan_IPv4Endpoint_from_parts(const struct IPv4Address* ip_address, const struct PortNumber* port_number);

/**
 * \brief Get the string representation of an IPv4Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Endpoint_to_string(struct ErrorContext* ectx, const struct IPv4Endpoint* inst);

/**
 * \brief Clone an existing IPv4Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Endpoint* freelan_IPv4Endpoint_clone(struct ErrorContext* ectx, const struct IPv4Endpoint* inst);

/**
 * \brief Delete an IPv4Endpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Endpoint_free(struct IPv4Endpoint* inst);

/**
 * \brief Compare two IPv4Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Endpoint_less_than(const struct IPv4Endpoint* lhs, const struct IPv4Endpoint* rhs);

/**
 * \brief Compare two IPv4Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Endpoint_equal(const struct IPv4Endpoint* lhs, const struct IPv4Endpoint* rhs);

/**
 * \brief Get the IPv4Address contained in a IPv4Endpoint instance.
 * \param inst The instance.
 * \return The IPv4Address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Address* freelan_IPv4Endpoint_get_IPv4Address(const struct IPv4Endpoint* inst);

/**
 * \brief Get the PortNumber contained in a IPv4Endpoint instance.
 * \param inst The instance.
 * \return The PortNumber instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct PortNumber* freelan_IPv4Endpoint_get_PortNumber(const struct IPv4Endpoint* inst);

/**
 * \brief Create a IPv6Endpoint instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv6Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Endpoint* freelan_IPv6Endpoint_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create a IPv6Endpoint instance from its parts.
 * \param ip_address The IP address component. The value is copied so the instance needs not stay alive.
 * \param port_number The port number. The value is copied so the instance needs not stay alive.
 * \return The IPv6Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Endpoint* freelan_IPv6Endpoint_from_parts(const struct IPv6Address* ip_address, const struct PortNumber* port_number);

/**
 * \brief Get the string representation of an IPv6Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Endpoint_to_string(struct ErrorContext* ectx, const struct IPv6Endpoint* inst);

/**
 * \brief Clone an existing IPv6Endpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv6Endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Endpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Endpoint* freelan_IPv6Endpoint_clone(struct ErrorContext* ectx, const struct IPv6Endpoint* inst);

/**
 * \brief Delete an IPv6Endpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Endpoint_free(struct IPv6Endpoint* inst);

/**
 * \brief Compare two IPv6Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Endpoint_less_than(const struct IPv6Endpoint* lhs, const struct IPv6Endpoint* rhs);

/**
 * \brief Compare two IPv6Endpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Endpoint_equal(const struct IPv6Endpoint* lhs, const struct IPv6Endpoint* rhs);

/**
 * \brief Get the IPv6Address contained in a IPv6Endpoint instance.
 * \param inst The instance.
 * \return The IPv6Address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Address* freelan_IPv6Endpoint_get_IPv6Address(const struct IPv6Endpoint* inst);

/**
 * \brief Get the PortNumber contained in a IPv6Endpoint instance.
 * \param inst The instance.
 * \return The PortNumber instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct PortNumber* freelan_IPv6Endpoint_get_PortNumber(const struct IPv6Endpoint* inst);

/**
 * \brief Create a HostnameEndpoint instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The HostnameEndpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_HostnameEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct HostnameEndpoint* freelan_HostnameEndpoint_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create a HostnameEndpoint instance from its parts.
 * \param hostname The hostname component. The value is copied so the instance needs not stay alive.
 * \param port_number The port number. The value is copied so the instance needs not stay alive.
 * \return The HostnameEndpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_HostnameEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct HostnameEndpoint* freelan_HostnameEndpoint_from_parts(const struct Hostname* hostname, const struct PortNumber* port_number);

/**
 * \brief Get the string representation of an HostnameEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_HostnameEndpoint_to_string(struct ErrorContext* ectx, const struct HostnameEndpoint* inst);

/**
 * \brief Clone an existing HostnameEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned HostnameEndpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_HostnameEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct HostnameEndpoint* freelan_HostnameEndpoint_clone(struct ErrorContext* ectx, const struct HostnameEndpoint* inst);

/**
 * \brief Delete an HostnameEndpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_HostnameEndpoint_free(struct HostnameEndpoint* inst);

/**
 * \brief Compare two HostnameEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_HostnameEndpoint_less_than(const struct HostnameEndpoint* lhs, const struct HostnameEndpoint* rhs);

/**
 * \brief Compare two HostnameEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_HostnameEndpoint_equal(const struct HostnameEndpoint* lhs, const struct HostnameEndpoint* rhs);

/**
 * \brief Get the Hostname contained in a HostnameEndpoint instance.
 * \param inst The instance.
 * \return The Hostname instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Hostname_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Hostname* freelan_HostnameEndpoint_get_Hostname(const struct HostnameEndpoint* inst);

/**
 * \brief Get the PortNumber contained in a HostnameEndpoint instance.
 * \param inst The instance.
 * \return The PortNumber instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_PortNumber_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct PortNumber* freelan_HostnameEndpoint_get_PortNumber(const struct HostnameEndpoint* inst);

/**
 * \brief Create a IPv4Route instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv4Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Route* freelan_IPv4Route_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create a IPv4Route instance from its parts.
 * \param ip_address The IP address component. The value is copied so the instance needs not stay alive.
 * \param prefix_length The prefix length. The value is copied so the instance needs not stay alive.
 * \param gateway The gateway component. The value is copied so the instance needs not stay alive. If null is specified, then the instance has no gateway.
 * \return The IPv4Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Route* freelan_IPv4Route_from_parts(const struct IPv4Address* ip_address, const struct IPv4PrefixLength* prefix_length, const struct IPv4Address* gateway);

/**
 * \brief Get the string representation of an IPv4Route instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv4Route_to_string(struct ErrorContext* ectx, const struct IPv4Route* inst);

/**
 * \brief Clone an existing IPv4Route instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv4Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Route* freelan_IPv4Route_clone(struct ErrorContext* ectx, const struct IPv4Route* inst);

/**
 * \brief Delete an IPv4Route instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv4Route_free(struct IPv4Route* inst);

/**
 * \brief Compare two IPv4Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv4Route_less_than(const struct IPv4Route* lhs, const struct IPv4Route* rhs);

/**
 * \brief Compare two IPv4Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv4Route_equal(const struct IPv4Route* lhs, const struct IPv4Route* rhs);

/**
 * \brief Get the IPv4Address contained in a IPv4Route instance.
 * \param inst The instance.
 * \return The IPv4Address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Address* freelan_IPv4Route_get_IPv4Address(const struct IPv4Route* inst);

/**
 * \brief Get the IPv4PrefixLength contained in a IPv4Route instance.
 * \param inst The instance.
 * \return The IPv4PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4PrefixLength* freelan_IPv4Route_get_IPv4PrefixLength(const struct IPv4Route* inst);

/**
 * \brief Get the IPv4Address gateway contained in a IPv4Route instance, if one is present.
 * \param inst The instance.
 * \return The IPv4Address instance. If the IP route has no gateway, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv4Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv4Address* freelan_IPv4Route_get_IPv4Address_gateway(const struct IPv4Route* inst);

/**
 * \brief Create a IPv6Route instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IPv6Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Route* freelan_IPv6Route_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create a IPv6Route instance from its parts.
 * \param ip_address The IP address component. The value is copied so the instance needs not stay alive.
 * \param prefix_length The prefix length. The value is copied so the instance needs not stay alive.
 * \param gateway The gateway component. The value is copied so the instance needs not stay alive. If null is specified, then the instance has no gateway.
 * \return The IPv6Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Route* freelan_IPv6Route_from_parts(const struct IPv6Address* ip_address, const struct IPv6PrefixLength* prefix_length, const struct IPv6Address* gateway);

/**
 * \brief Get the string representation of an IPv6Route instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPv6Route_to_string(struct ErrorContext* ectx, const struct IPv6Route* inst);

/**
 * \brief Clone an existing IPv6Route instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPv6Route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Route_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Route* freelan_IPv6Route_clone(struct ErrorContext* ectx, const struct IPv6Route* inst);

/**
 * \brief Delete an IPv6Route instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPv6Route_free(struct IPv6Route* inst);

/**
 * \brief Compare two IPv6Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPv6Route_less_than(const struct IPv6Route* lhs, const struct IPv6Route* rhs);

/**
 * \brief Compare two IPv6Route instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPv6Route_equal(const struct IPv6Route* lhs, const struct IPv6Route* rhs);

/**
 * \brief Get the IPv6Address contained in a IPv6Route instance.
 * \param inst The instance.
 * \return The IPv6Address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Address* freelan_IPv6Route_get_IPv6Address(const struct IPv6Route* inst);

/**
 * \brief Get the IPv6PrefixLength contained in a IPv6Route instance.
 * \param inst The instance.
 * \return The IPv6PrefixLength instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6PrefixLength_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6PrefixLength* freelan_IPv6Route_get_IPv6PrefixLength(const struct IPv6Route* inst);

/**
 * \brief Get the IPv6Address gateway contained in a IPv6Route instance, if one is present.
 * \param inst The instance.
 * \return The IPv6Address instance. If the IP route has no gateway, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPv6Address_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPv6Address* freelan_IPv6Route_get_IPv6Address_gateway(const struct IPv6Route* inst);

/**
 * \brief Create an IPAddress instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP address instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPAddress* freelan_IPAddress_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create an IPAddress instance from an IPv4Address.
 * \param value The IPv4Address instance. Cannot be NULL.
 * \return The IP address instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPAddress* freelan_IPAddress_from_IPv4Address(const struct IPv4Address* value);

/**
 * \brief Create an IPAddress instance from an IPv6Address.
 * \param value The IPv6Address instance. Cannot be NULL.
 * \return The IP address instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPAddress* freelan_IPAddress_from_IPv6Address(const struct IPv6Address* value);

/**
 * \brief Get the IPv4Address instance contained in the specified IPAddress instance.
 * \param inst The IPAddress instance to get the value from.
 * \return The IPv4Address instance, or NULL if \c inst is not an IPv4Address.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Address_clone().
 */
FREELAN_API const struct IPv4Address* freelan_IPAddress_as_IPv4Address(const struct IPAddress* inst);

/**
 * \brief Get the IPv6Address instance contained in the specified IPAddress instance.
 * \param inst The IPAddress instance to get the value from.
 * \return The IPv6Address instance, or NULL if \c inst is not an IPv6Address.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Address_clone().
 */
FREELAN_API const struct IPv6Address* freelan_IPAddress_as_IPv6Address(const struct IPAddress* inst);

/**
 * \brief Get the string representation of an IPAddress instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPAddress_to_string(struct ErrorContext* ectx, const struct IPAddress* inst);

/**
 * \brief Clone an existing IPAddress instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPAddress instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPAddress_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPAddress* freelan_IPAddress_clone(struct ErrorContext* ectx, const struct IPAddress* inst);

/**
 * \brief Delete an IPAddress instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPAddress_free(struct IPAddress* inst);

/**
 * \brief Compare two IPAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPAddress_less_than(const struct IPAddress* lhs, const struct IPAddress* rhs);

/**
 * \brief Compare two IPAddress instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPAddress_equal(const struct IPAddress* lhs, const struct IPAddress* rhs);

/**
 * \brief Create an IPRoute instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP route instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPRoute* freelan_IPRoute_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create an IPRoute instance from an IPv4Route.
 * \param value The IPv4Route instance. Cannot be NULL.
 * \return The IP route instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPRoute* freelan_IPRoute_from_IPv4Route(const struct IPv4Route* value);

/**
 * \brief Create an IPRoute instance from an IPv6Route.
 * \param value The IPv6Route instance. Cannot be NULL.
 * \return The IP route instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPRoute* freelan_IPRoute_from_IPv6Route(const struct IPv6Route* value);

/**
 * \brief Get the IPv4Route instance contained in the specified IPRoute instance.
 * \param inst The IPRoute instance to get the value from.
 * \return The IPv4Route instance, or NULL if \c inst is not an IPv4Route.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Route_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Route_clone().
 */
FREELAN_API const struct IPv4Route* freelan_IPRoute_as_IPv4Route(const struct IPRoute* inst);

/**
 * \brief Get the IPv6Route instance contained in the specified IPRoute instance.
 * \param inst The IPRoute instance to get the value from.
 * \return The IPv6Route instance, or NULL if \c inst is not an IPv6Route.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Route_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Route_clone().
 */
FREELAN_API const struct IPv6Route* freelan_IPRoute_as_IPv6Route(const struct IPRoute* inst);

/**
 * \brief Get the string representation of an IPRoute instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPRoute_to_string(struct ErrorContext* ectx, const struct IPRoute* inst);

/**
 * \brief Clone an existing IPRoute instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPRoute instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPRoute_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPRoute* freelan_IPRoute_clone(struct ErrorContext* ectx, const struct IPRoute* inst);

/**
 * \brief Delete an IPRoute instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPRoute_free(struct IPRoute* inst);

/**
 * \brief Compare two IPRoute instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPRoute_less_than(const struct IPRoute* lhs, const struct IPRoute* rhs);

/**
 * \brief Compare two IPRoute instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPRoute_equal(const struct IPRoute* lhs, const struct IPRoute* rhs);

/**
 * \brief Create an Host instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The host instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Host* freelan_Host_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create an Host instance from an IPv4Address.
 * \param value The IPv4Address instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Host* freelan_Host_from_IPv4Address(const struct IPv4Address* value);

/**
 * \brief Create an Host instance from an IPv6Address.
 * \param value The IPv6Address instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Host* freelan_Host_from_IPv6Address(const struct IPv6Address* value);

/**
 * \brief Create an Host instance from a Hostname.
 * \param value The Hostname instance. Cannot be NULL.
 * \return The host instance.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Host* freelan_Host_from_Hostname(const struct Hostname* value);

/**
 * \brief Get the IPv4Address instance contained in the specified Host instance.
 * \param inst The Host instance to get the value from.
 * \return The IPv4Address instance, or NULL if \c inst is not an IPv4Address.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Address_clone().
 */
FREELAN_API const struct IPv4Address* freelan_Host_as_IPv4Address(const struct Host* inst);

/**
 * \brief Get the IPv6Address instance contained in the specified Host instance.
 * \param inst The Host instance to get the value from.
 * \return The IPv6Address instance, or NULL if \c inst is not an IPv6Address.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Address_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Address_clone().
 */
FREELAN_API const struct IPv6Address* freelan_Host_as_IPv6Address(const struct Host* inst);

/**
 * \brief Get the Hostname instance contained in the specified Host instance.
 * \param inst The Host instance to get the value from.
 * \return The Hostname instance, or NULL if \c inst is not a Hostname.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_Hostname_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_Hostname_clone().
 */
FREELAN_API const struct Hostname* freelan_Host_as_Hostname(const struct Host* inst);

/**
 * \brief Get the string representation of an Host instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_Host_to_string(struct ErrorContext* ectx, const struct Host* inst);

/**
 * \brief Clone an existing Host instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned Host instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_Host_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct Host* freelan_Host_clone(struct ErrorContext* ectx, const struct Host* inst);

/**
 * \brief Delete an Host instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_Host_free(struct Host* inst);

/**
 * \brief Compare two Host instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_Host_less_than(const struct Host* lhs, const struct Host* rhs);

/**
 * \brief Compare two Host instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_Host_equal(const struct Host* lhs, const struct Host* rhs);

/**
 * \brief Create an IPEndpoint instance from its string representation.
 * \param ectx The error context.
 * \param str The string representation.
 * \return The IP endpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPEndpoint* freelan_IPEndpoint_from_string(struct ErrorContext* ectx, const char* str);

/**
 * \brief Create an IPEndpoint instance from an IPv4Endpoint.
 * \param value The IPv4Endpoint instance. Cannot be NULL.
 * \return The IP endpoint instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPEndpoint* freelan_IPEndpoint_from_IPv4Endpoint(const struct IPv4Endpoint* value);

/**
 * \brief Create an IPEndpoint instance from an IPv6Endpoint.
 * \param value The IPv6Endpoint instance. Cannot be NULL.
 * \return The IP endpoint instance.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPEndpoint* freelan_IPEndpoint_from_IPv6Endpoint(const struct IPv6Endpoint* value);

/**
 * \brief Get the IPv4Endpoint instance contained in the specified IPEndpoint instance.
 * \param inst The IPEndpoint instance to get the value from.
 * \return The IPv4Endpoint instance, or NULL if \c inst is not an IPv4Endpoint.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv4Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv4Endpoint_clone().
 */
FREELAN_API const struct IPv4Endpoint* freelan_IPEndpoint_as_IPv4Endpoint(const struct IPEndpoint* inst);

/**
 * \brief Get the IPv6Endpoint instance contained in the specified IPEndpoint instance.
 * \param inst The IPEndpoint instance to get the value from.
 * \return The IPv6Endpoint instance, or NULL if \c inst is not an IPv6Endpoint.
 * The returned instance as the same lifetime as \c inst.
 *
 * \warning The caller must *NOT* call \c freelan_IPv6Endpoint_free()
 * on the returned instance. If you need to keep around the returned value
 * longer, look at \c freelan_IPv6Endpoint_clone().
 */
FREELAN_API const struct IPv6Endpoint* freelan_IPEndpoint_as_IPv6Endpoint(const struct IPEndpoint* inst);

/**
 * \brief Get the string representation of an IPEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance.
 * \return The string representation, as a NULL terminated string.
 *
 * \warning The caller is responsible for calling \c freelan_free() on the
 * returned string when it is no longer needed.
 */
FREELAN_API char* freelan_IPEndpoint_to_string(struct ErrorContext* ectx, const struct IPEndpoint* inst);

/**
 * \brief Clone an existing IPEndpoint instance.
 * \param ectx The error context.
 * \param inst The instance to clone
 * \return The cloned IPEndpoint instance. On error, a null pointer is returned.
 *
 * \warning The caller is responsible for calling \c freelan_IPEndpoint_free()
 * on the returned instance when it is no longer needed.
 */
FREELAN_API struct IPEndpoint* freelan_IPEndpoint_clone(struct ErrorContext* ectx, const struct IPEndpoint* inst);

/**
 * \brief Delete an IPEndpoint instance.
 * \param inst The instance.
 */
FREELAN_API void freelan_IPEndpoint_free(struct IPEndpoint* inst);

/**
 * \brief Compare two IPEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs < rhs.
 */
FREELAN_API int freelan_IPEndpoint_less_than(const struct IPEndpoint* lhs, const struct IPEndpoint* rhs);

/**
 * \brief Compare two IPEndpoint instances.
 * \param lhs The left instance.
 * \param rhs The right instance.
 * \return A non-zero value if the lhs == rhs.
 */
FREELAN_API int freelan_IPEndpoint_equal(const struct IPEndpoint* lhs, const struct IPEndpoint* rhs);

/* API END */

#ifdef __cplusplus
}
#endif

#endif

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

#include "types.h"

#include <cassert>

#include "memory.h"

#include "memory.hpp"
#include "error.hpp"
#include "ipv4_address.hpp"
#include "ipv6_address.hpp"
#include "hostname.hpp"
#include "port_number.hpp"
#include "ipv4_prefix_length.hpp"
#include "ipv6_prefix_length.hpp"
#include "ipv4_endpoint.hpp"
#include "ipv6_endpoint.hpp"
#include "hostname_endpoint.hpp"
#include "ipv4_route.hpp"
#include "ipv6_route.hpp"

namespace {
	template <typename Type, typename InternalType>
	Type* from_string_generic(struct ErrorContext* ectx, const char* str) {
		assert(str);

		FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx);

		return reinterpret_cast<Type*>(
			FREELAN_NEW InternalType(InternalType::from_string(str))
		);

		FREELAN_END_USE_ERROR_CONTEXT(ectx);

		return nullptr;
	}

	template <typename Type, typename InternalType>
	char* to_string_generic(struct ErrorContext* ectx, const Type* inst) {
		assert(inst);

		const auto value = reinterpret_cast<const InternalType*>(inst);

		FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx);

		return ::freelan_strdup(value->to_string().c_str());

		FREELAN_END_USE_ERROR_CONTEXT(ectx);

		return nullptr;
	}

	template <typename Type, typename InternalType>
	int less_than_generic(const Type* lhs, const Type* rhs) {
		assert(lhs);
		assert(rhs);

		const auto ilhs = *reinterpret_cast<const InternalType*>(lhs);
		const auto irhs = *reinterpret_cast<const InternalType*>(rhs);

		return (ilhs < irhs) ? 1 : 0;
	}

	template <typename Type, typename InternalType>
	int equal_generic(const Type* lhs, const Type* rhs) {
		assert(lhs);
		assert(rhs);

		const auto ilhs = *reinterpret_cast<const InternalType*>(lhs);
		const auto irhs = *reinterpret_cast<const InternalType*>(rhs);

		return (ilhs == irhs) ? 1 : 0;
	}

	template <typename Type, typename InternalType, typename LeftPartType, typename LeftInternalPartType, typename RightPartType, typename RightInternalPartType>
	Type* from_parts_generic(const LeftPartType* lhs, const RightPartType* rhs) {
		assert(lhs);
		assert(rhs);

		const auto ilhs = *reinterpret_cast<const LeftInternalPartType*>(lhs);
		const auto irhs = *reinterpret_cast<const RightInternalPartType*>(rhs);

		return reinterpret_cast<Type*>(
			FREELAN_NEW InternalType(ilhs, irhs)
		);
	}

	template <typename Type, typename InternalType, typename LeftPartType, typename LeftInternalPartType, typename RightPartType, typename RightInternalPartType>
	Type* from_parts_with_optional_generic(const LeftPartType* lhs, const RightPartType* rhs, const LeftPartType* optional = nullptr) {
		assert(lhs);
		assert(rhs);

		const auto ilhs = *reinterpret_cast<const LeftInternalPartType*>(lhs);
		const auto irhs = *reinterpret_cast<const RightInternalPartType*>(rhs);

		if (optional != nullptr) {
			const auto ioptional = *reinterpret_cast<const LeftInternalPartType*>(optional);

			return reinterpret_cast<Type*>(
				FREELAN_NEW InternalType(ilhs, irhs, ioptional)
			);
		} else {
			return reinterpret_cast<Type*>(
				FREELAN_NEW InternalType(ilhs, irhs)
			);
		}
	}

	template <typename Type, typename InternalType, typename IPAddressType, typename InternalIPAddressType>
	IPAddressType* get_ip_address_generic(const Type* inst) {
		assert(inst);

		const auto value = *reinterpret_cast<const InternalType*>(inst);

		return reinterpret_cast<IPAddressType*>(
			FREELAN_NEW InternalIPAddressType(value.get_ip_address())
		);
	}

	template <typename Type, typename InternalType, typename PortNumberType, typename InternalPortNumberType>
	PortNumberType* get_port_number_generic(const Type* inst) {
		assert(inst);

		const auto value = *reinterpret_cast<const InternalType*>(inst);

		return reinterpret_cast<PortNumberType*>(
			FREELAN_NEW InternalPortNumberType(value.get_port_number())
		);
	}

	template <typename Type, typename InternalType, typename HostnameType, typename InternalHostnameType>
	HostnameType* get_hostname_generic(const Type* inst) {
		assert(inst);

		const auto value = *reinterpret_cast<const InternalType*>(inst);

		return reinterpret_cast<HostnameType*>(
			FREELAN_NEW InternalHostnameType(value.get_hostname())
		);
	}

	template <typename Type, typename InternalType, typename IPPrefixLengthType, typename InternalIPPrefixLengthType>
	IPPrefixLengthType* get_prefix_length_generic(const Type* inst) {
		assert(inst);

		const auto value = *reinterpret_cast<const InternalType*>(inst);

		return reinterpret_cast<IPPrefixLengthType*>(
			FREELAN_NEW InternalIPPrefixLengthType(value.get_prefix_length())
		);
	}

	template <typename Type, typename InternalType, typename IPAddressType, typename InternalIPAddressType>
	IPAddressType* get_gateway_generic(const Type* inst) {
		assert(inst);

		const auto value = *reinterpret_cast<const InternalType*>(inst);

		if (!value.has_gateway()) {
			return nullptr;
		}

		return reinterpret_cast<IPAddressType*>(
			FREELAN_NEW InternalIPAddressType(value.get_gateway())
		);
	}
}

/*
 * I'm usually not too fond of C macros, especially those that span accross
 * several lines and try to do smart things. In this case, the alternative is
 * having duplicate and exactly similar function definitions which is error
 * prone and ridiculous.
 */

#define IMPLEMENT_from_string(TYPE) \
struct TYPE* freelan_ ## TYPE ## _from_string(struct ErrorContext* ectx, const char* str) { return from_string_generic<TYPE, freelan::TYPE>(ectx, str); }

#define IMPLEMENT_to_string(TYPE) \
char* freelan_ ## TYPE ## _to_string(struct ErrorContext* ectx, const struct TYPE* inst) { return to_string_generic<TYPE, freelan::TYPE>(ectx, inst); }

#define IMPLEMENT_free(TYPE) \
void freelan_ ## TYPE ## _free(struct TYPE* inst) { FREELAN_DELETE reinterpret_cast<freelan::TYPE*>(inst); }

#define IMPLEMENT_less_than(TYPE) \
int freelan_ ## TYPE ## _less_than(const struct TYPE* lhs, const struct TYPE* rhs) { return less_than_generic<TYPE, freelan::TYPE>(lhs, rhs); }

#define IMPLEMENT_equal(TYPE) \
int freelan_ ## TYPE ## _equal(const struct TYPE* lhs, const struct TYPE* rhs) { return equal_generic<TYPE, freelan::TYPE>(lhs, rhs); }

#define IMPLEMENT_from_parts(TYPE,LTYPE,RTYPE) \
struct TYPE* freelan_ ## TYPE ## _from_parts(const struct LTYPE* lhs, const struct RTYPE* rhs) { return from_parts_generic<TYPE, freelan::TYPE, LTYPE, freelan::LTYPE, RTYPE, freelan::RTYPE>(lhs, rhs); }

#define IMPLEMENT_from_parts_with_optional(TYPE,LTYPE,RTYPE) \
struct TYPE* freelan_ ## TYPE ## _from_parts(const struct LTYPE* lhs, const struct RTYPE* rhs, const struct LTYPE* optional) { return from_parts_with_optional_generic<TYPE, freelan::TYPE, LTYPE, freelan::LTYPE, RTYPE, freelan::RTYPE>(lhs, rhs, optional); }

#define IMPLEMENT_get_ip_address(TYPE,IATYPE) \
struct IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct TYPE* inst) { return get_ip_address_generic<TYPE, freelan::TYPE, IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_port_number(TYPE,PNTYPE) \
struct PNTYPE* freelan_ ## TYPE ## _get_ ## PNTYPE (const struct TYPE* inst) { return get_port_number_generic<TYPE, freelan::TYPE, PNTYPE, freelan::PNTYPE>(inst); }

#define IMPLEMENT_get_hostname(TYPE,IATYPE) \
struct IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct TYPE* inst) { return get_hostname_generic<TYPE, freelan::TYPE, IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_prefix_length(TYPE,IATYPE) \
struct IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct TYPE* inst) { return get_prefix_length_generic<TYPE, freelan::TYPE, IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_gateway(TYPE,IATYPE) \
struct IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE ## _gateway (const struct TYPE* inst) { return get_gateway_generic<TYPE, freelan::TYPE, IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_string(TYPE) \
IMPLEMENT_to_string(TYPE) \
IMPLEMENT_free(TYPE) \
IMPLEMENT_less_than(TYPE) \
IMPLEMENT_equal(TYPE)

#define IMPLEMENT_composite_type(TYPE,LTYPE,RTYPE) \
IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_parts(TYPE,LTYPE,RTYPE)

#define IMPLEMENT_extended_composite_type(TYPE,LTYPE,RTYPE) \
IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_parts_with_optional(TYPE,LTYPE,RTYPE)

/* Simple types */
IMPLEMENT_complete_type(IPv4Address)
IMPLEMENT_complete_type(IPv6Address)
IMPLEMENT_complete_type(Hostname)
IMPLEMENT_complete_type(PortNumber)
IMPLEMENT_complete_type(IPv4PrefixLength)
IMPLEMENT_complete_type(IPv6PrefixLength)

/* Composite types */
IMPLEMENT_composite_type(IPv4Endpoint, IPv4Address, PortNumber)
IMPLEMENT_get_ip_address(IPv4Endpoint, IPv4Address)
IMPLEMENT_get_port_number(IPv4Endpoint, PortNumber)

IMPLEMENT_composite_type(IPv6Endpoint, IPv6Address, PortNumber)
IMPLEMENT_get_ip_address(IPv6Endpoint, IPv6Address)
IMPLEMENT_get_port_number(IPv6Endpoint, PortNumber)

IMPLEMENT_composite_type(HostnameEndpoint, Hostname, PortNumber)
IMPLEMENT_get_hostname(HostnameEndpoint, Hostname)
IMPLEMENT_get_port_number(HostnameEndpoint, PortNumber)

IMPLEMENT_extended_composite_type(IPv4Route, IPv4Address, IPv4PrefixLength)
IMPLEMENT_get_ip_address(IPv4Route, IPv4Address)
IMPLEMENT_get_prefix_length(IPv4Route, IPv4PrefixLength)
IMPLEMENT_get_gateway(IPv4Route, IPv4Address)

IMPLEMENT_extended_composite_type(IPv6Route, IPv6Address, IPv6PrefixLength)
IMPLEMENT_get_ip_address(IPv6Route, IPv6Address)
IMPLEMENT_get_prefix_length(IPv6Route, IPv6PrefixLength)
IMPLEMENT_get_gateway(IPv6Route, IPv6Address)

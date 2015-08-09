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
#include "types/ipv4_address.hpp"
#include "types/ipv6_address.hpp"
#include "types/hostname.hpp"
#include "types/port_number.hpp"
#include "types/ipv4_prefix_length.hpp"
#include "types/ipv6_prefix_length.hpp"
#include "types/ipv4_endpoint.hpp"
#include "types/ipv6_endpoint.hpp"
#include "types/hostname_endpoint.hpp"
#include "types/ipv4_route.hpp"
#include "types/ipv6_route.hpp"
#include "types/ip_address.hpp"
#include "types/ip_route.hpp"
#include "types/host.hpp"
#include "types/ip_endpoint.hpp"
#include "types/host_endpoint.hpp"

namespace {
	template <typename Type, typename InternalType>
	Type* from_string_generic(struct freelan_ErrorContext* ectx, const char* str) {
		assert(str);

		FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx);

		return reinterpret_cast<Type*>(
			FREELAN_NEW InternalType(InternalType::from_string(str))
		);

		FREELAN_END_USE_ERROR_CONTEXT(ectx);

		return nullptr;
	}

	template <typename Type, typename InternalType>
	char* to_string_generic(struct freelan_ErrorContext* ectx, const Type* inst) {
		assert(inst);

		const auto value = reinterpret_cast<const InternalType*>(inst);

		FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx);

		return ::freelan_strdup(value->to_string().c_str());

		FREELAN_END_USE_ERROR_CONTEXT(ectx);

		return nullptr;
	}

	template <typename Type, typename InternalType>
	Type* clone_generic(struct freelan_ErrorContext* ectx, const Type* inst) {
		assert(inst);

		FREELAN_BEGIN_USE_ERROR_CONTEXT(ectx);

		return reinterpret_cast<Type*>(
			FREELAN_NEW InternalType(*reinterpret_cast<const InternalType*>(inst))
		);

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

	template <typename Type, typename InternalType, typename VariantType, typename InternalVariantType>
	Type* from_variant_type_generic(const VariantType* value) {
		assert(value);

		return reinterpret_cast<Type*>(
			FREELAN_NEW InternalType(*reinterpret_cast<const InternalVariantType*>(value))
		);
	}

	template <typename Type, typename InternalType, typename VariantType, typename InternalVariantType>
	const VariantType* as_variant_type_generic(const Type* inst) {
		assert(inst);

		const InternalType& _inst = *reinterpret_cast<const InternalType*>(inst);
		const InternalVariantType* value = _inst.template as<InternalVariantType>();

		return reinterpret_cast<const VariantType*>(value);
	}
}

/*
 * I'm usually not too fond of C macros, especially those that span accross
 * several lines and try to do smart things. In this case, the alternative is
 * having duplicate and exactly similar function definitions which is error
 * prone and ridiculous.
 */

#define IMPLEMENT_from_string(TYPE) \
struct freelan_ ## TYPE* freelan_ ## TYPE ## _from_string(struct freelan_ErrorContext* ectx, const char* str) { return from_string_generic<freelan_ ## TYPE, freelan::TYPE>(ectx, str); }

#define IMPLEMENT_to_string(TYPE) \
char* freelan_ ## TYPE ## _to_string(struct freelan_ErrorContext* ectx, const struct freelan_ ## TYPE* inst) { return to_string_generic<freelan_ ## TYPE, freelan::TYPE>(ectx, inst); }

#define IMPLEMENT_clone(TYPE) \
struct freelan_ ## TYPE* freelan_ ## TYPE ## _clone(struct freelan_ErrorContext* ectx, const struct freelan_ ## TYPE* inst) { return clone_generic<freelan_ ## TYPE, freelan::TYPE>(ectx, inst); }

#define IMPLEMENT_free(TYPE) \
void freelan_ ## TYPE ## _free(struct freelan_ ## TYPE* inst) { FREELAN_DELETE reinterpret_cast<freelan::TYPE*>(inst); }

#define IMPLEMENT_less_than(TYPE) \
int freelan_ ## TYPE ## _less_than(const struct freelan_ ## TYPE* lhs, const struct freelan_ ## TYPE* rhs) { return less_than_generic<freelan_ ## TYPE, freelan::TYPE>(lhs, rhs); }

#define IMPLEMENT_equal(TYPE) \
int freelan_ ## TYPE ## _equal(const struct freelan_ ## TYPE* lhs, const struct freelan_ ## TYPE* rhs) { return equal_generic<freelan_ ## TYPE, freelan::TYPE>(lhs, rhs); }

#define IMPLEMENT_from_parts(TYPE,LTYPE,RTYPE) \
struct freelan_ ## TYPE* freelan_ ## TYPE ## _from_parts(const struct freelan_ ## LTYPE* lhs, const struct freelan_ ## RTYPE* rhs) { return from_parts_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## LTYPE, freelan::LTYPE, freelan_ ## RTYPE, freelan::RTYPE>(lhs, rhs); }

#define IMPLEMENT_from_parts_with_optional(TYPE,LTYPE,RTYPE) \
struct freelan_ ## TYPE* freelan_ ## TYPE ## _from_parts(const struct freelan_ ## LTYPE* lhs, const struct freelan_ ## RTYPE* rhs, const struct freelan_ ## LTYPE* optional) { return from_parts_with_optional_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## LTYPE, freelan::LTYPE, freelan_ ## RTYPE, freelan::RTYPE>(lhs, rhs, optional); }

#define IMPLEMENT_get_ip_address(TYPE,IATYPE) \
struct freelan_ ## IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct freelan_ ## TYPE* inst) { return get_ip_address_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_port_number(TYPE,PNTYPE) \
struct freelan_ ## PNTYPE* freelan_ ## TYPE ## _get_ ## PNTYPE (const struct freelan_ ## TYPE* inst) { return get_port_number_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## PNTYPE, freelan::PNTYPE>(inst); }

#define IMPLEMENT_get_hostname(TYPE,IATYPE) \
struct freelan_ ## IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct freelan_ ## TYPE* inst) { return get_hostname_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_prefix_length(TYPE,IATYPE) \
struct freelan_ ## IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE (const struct freelan_ ## TYPE* inst) { return get_prefix_length_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_get_gateway(TYPE,IATYPE) \
struct freelan_ ## IATYPE* freelan_ ## TYPE ## _get_ ## IATYPE ## _gateway (const struct freelan_ ## TYPE* inst) { return get_gateway_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## IATYPE, freelan::IATYPE>(inst); }

#define IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_string(TYPE) \
IMPLEMENT_to_string(TYPE) \
IMPLEMENT_clone(TYPE) \
IMPLEMENT_free(TYPE) \
IMPLEMENT_less_than(TYPE) \
IMPLEMENT_equal(TYPE)

#define IMPLEMENT_composite_type(TYPE,LTYPE,RTYPE) \
IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_parts(TYPE,LTYPE,RTYPE)

#define IMPLEMENT_extended_composite_type(TYPE,LTYPE,RTYPE) \
IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_parts_with_optional(TYPE,LTYPE,RTYPE)

#define IMPLEMENT_variant(TYPE,VARIANT_TYPE) \
struct freelan_ ## TYPE* freelan_ ## TYPE ## _from_ ## VARIANT_TYPE (const struct freelan_ ## VARIANT_TYPE* value) { return from_variant_type_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## VARIANT_TYPE, freelan::VARIANT_TYPE>(value); } \
const struct freelan_ ## VARIANT_TYPE* freelan_ ## TYPE ## _as_ ## VARIANT_TYPE (const struct freelan_ ## TYPE* inst) { return as_variant_type_generic<freelan_ ## TYPE, freelan::TYPE, freelan_ ## VARIANT_TYPE, freelan::VARIANT_TYPE>(inst); }

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

/* Variant types */
IMPLEMENT_complete_type(IPAddress)
IMPLEMENT_variant(IPAddress, IPv4Address)
IMPLEMENT_variant(IPAddress, IPv6Address)

IMPLEMENT_complete_type(IPRoute)
IMPLEMENT_variant(IPRoute, IPv4Route)
IMPLEMENT_variant(IPRoute, IPv6Route)

IMPLEMENT_complete_type(Host)
IMPLEMENT_variant(Host, IPv4Address)
IMPLEMENT_variant(Host, IPv6Address)
IMPLEMENT_variant(Host, Hostname)

IMPLEMENT_complete_type(IPEndpoint)
IMPLEMENT_variant(IPEndpoint, IPv4Endpoint)
IMPLEMENT_variant(IPEndpoint, IPv6Endpoint)

IMPLEMENT_complete_type(HostEndpoint)
IMPLEMENT_variant(HostEndpoint, IPv4Endpoint)
IMPLEMENT_variant(HostEndpoint, IPv6Endpoint)
IMPLEMENT_variant(HostEndpoint, HostnameEndpoint)

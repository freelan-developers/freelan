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

#define IMPLEMENT_complete_type(TYPE) \
IMPLEMENT_from_string(TYPE) \
IMPLEMENT_to_string(TYPE) \
IMPLEMENT_free(TYPE) \
IMPLEMENT_less_than(TYPE) \
IMPLEMENT_equal(TYPE) \

IMPLEMENT_complete_type(IPv4Address)
IMPLEMENT_complete_type(IPv6Address)
IMPLEMENT_complete_type(Hostname)
IMPLEMENT_complete_type(PortNumber)
IMPLEMENT_complete_type(IPv4PrefixLength)
IMPLEMENT_complete_type(IPv6PrefixLength)

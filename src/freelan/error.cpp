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

#include "error.h"

#include "memory.hpp"
#include "error.hpp"

FREELAN_API struct freelan_ErrorContext* freelan_acquire_error_context(void) {
	try {
		freelan::ErrorContext* const ectx = FREELAN_NEW freelan::ErrorContext {};

		return reinterpret_cast<freelan_ErrorContext*>(ectx);
	} catch (std::bad_alloc&) {
		return nullptr;
	}
}

FREELAN_API void freelan_release_error_context(struct freelan_ErrorContext* ectx) {
	FREELAN_DELETE reinterpret_cast<freelan::ErrorContext*>(ectx);
}

FREELAN_API void freelan_error_context_reset(struct freelan_ErrorContext* ectx) {
	reinterpret_cast<freelan::ErrorContext*>(ectx)->reset();
}

FREELAN_API const char* freelan_error_context_get_error_category(const struct freelan_ErrorContext* ectx) {
	const auto& ec = reinterpret_cast<const freelan::ErrorContext*>(ectx)->error_code();

	if (ec) {
		return ec.category().name();
	} else {
		return nullptr;
	}
}

FREELAN_API int freelan_error_context_get_error_code(const struct freelan_ErrorContext* ectx) {
	const auto& ec = reinterpret_cast<const freelan::ErrorContext*>(ectx)->error_code();

	if (ec) {
		return ec.value();
	} else {
		return 0;
	}
}

FREELAN_API const char* freelan_error_context_get_error_description(const struct freelan_ErrorContext* _ectx) {
	const auto& ectx = *reinterpret_cast<const freelan::ErrorContext*>(_ectx);

	if (ectx.error_code()) {
		return ectx.description().c_str();
	} else {
		return nullptr;
	}
}

FREELAN_API const char* freelan_error_context_get_error_file(const struct freelan_ErrorContext* _ectx) {
	const auto& ectx = *reinterpret_cast<const freelan::ErrorContext*>(_ectx);

	if (ectx.error_code()) {
		return ectx.file();
	} else {
		return nullptr;
	}
}

FREELAN_API unsigned int freelan_error_context_get_error_line(const struct freelan_ErrorContext* _ectx) {
	const auto& ectx = *reinterpret_cast<const freelan::ErrorContext*>(_ectx);

	if (ectx.error_code()) {
		return ectx.line();
	} else {
		return 0;
	}
}

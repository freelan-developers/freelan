/*
 * libexecuteplus - A portable execution library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libexecuteplus.
 *
 * libexecuteplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libexecuteplus is distributed in the hope that it will be useful, but
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
 * If you intend to use libexecuteplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file os.hpp
 * \brief Macro to know the operating system.
 *
 * - UNIX: Unix-like operating system.\n
 * - LINUX: Linux operating system.\n
 * - SUNOS: Sun operating system.\n
 * - MACINTOSH: Macintosh or MacOS operating system.\n
 * - WINDOWS: MS Windows operating system.\n
 * - MSDOS: MS DOS operating system.\n
 *
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \author Sebastien VINCENT <sebastien.vincent@freelan.org>
 */

#pragma once

/*
 * Extract the "MACINTOSH" flag from the compiler.
 */
#if defined(__APPLE__)
#define UNIX
#define MACINTOSH
#endif

/*
 * Extract the "SUNOS" flag from the compiler.
 */
#if defined(sun)
#define UNIX
#define SUNOS
#endif

/*
 * Extract the "UNIX" flag from compiler.
 */

#ifdef __linux__
#define UNIX
#define LINUX
#endif

/*
 * Extract the "BSD" flag from compiler.
 */

#if defined(BSD) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define OS_BSD
#define UNIX
#endif

/*
 * Extract the "MSDOS" flag from the compiler.
 */
#ifdef __MSDOS__
#define MSDOS
#undef UNIX
#endif

/*
 * Extract the "WINDOWS" flag from the compiler.
 */
#if defined(_Windows) || defined(__WINDOWS__) || \
	defined(__WIN32__) || defined(WIN32) || \
defined(__WINNT__) || defined(__NT__) || \
defined(_WIN32) || defined(_WIN64)
#define WINDOWS
#ifdef _MSC_VER
#define MSV
#if defined(DEBUG) || defined(DEBUGTRACE)
#ifdef NDEBUG
#undef NDEBUG
#endif
#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
#else
#undef MSV
#endif
#undef UNIX
#undef MSDOS
#endif

/*
 * Remove the WINDOWS flag when using MACINTOSH.
 */
#ifdef MACINTOSH
#undef WINDOWS
#endif

/*
 * Assume UNIX if not Windows, Macintosh or MSDOS.
 */
#if !defined(WINDOWS) && !defined(MACINTOSH) && !defined(MSDOS)
#define UNIX
#endif

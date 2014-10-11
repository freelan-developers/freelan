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
 * \file colors.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Colors utility functions.
 */

#ifndef COLORS_HPP
#define COLORS_HPP

#include <fscp/logger.hpp>

#include <string>
#include <cassert>

static const std::string COLOR_RESET = "\x1b[0m";
static const std::string COLOR_BOLD = "\x1b[1m";
static const std::string COLOR_DIM = "\x1b[1m";
static const std::string COLOR_UNDERLINE = "\x1b[4m";
static const std::string COLOR_BLINK = "\x1b[5m";
static const std::string COLOR_REVERSE = "\x1b[7m";
static const std::string COLOR_CONCEALED = "\x1b[8m";
static const std::string COLOR_FG_BLACK = "\x1b[30m";
static const std::string COLOR_FG_RED = "\x1b[31m";
static const std::string COLOR_FG_GREEN = "\x1b[32m";
static const std::string COLOR_FG_YELLOW = "\x1b[33m";
static const std::string COLOR_FG_BLUE = "\x1b[34m";
static const std::string COLOR_FG_MAGENTA = "\x1b[35m";
static const std::string COLOR_FG_CYAN = "\x1b[36m";
static const std::string COLOR_FG_WHITE = "\x1b[37m";
static const std::string COLOR_BG_BLACK = "\x1b[40m";
static const std::string COLOR_BG_RED = "\x1b[41m";
static const std::string COLOR_BG_GREEN = "\x1b[42m";
static const std::string COLOR_BG_YELLOW = "\x1b[43m";
static const std::string COLOR_BG_BLUE = "\x1b[44m";
static const std::string COLOR_BG_MAGENTA = "\x1b[45m";
static const std::string COLOR_BG_CYAN = "\x1b[46m";
static const std::string COLOR_BG_WHITE = "\x1b[47m";

inline std::string log_level_to_color(fscp::log_level level)
{
	switch (level)
	{
		case fscp::log_level::trace:
			return COLOR_DIM + COLOR_FG_WHITE;
		case fscp::log_level::debug:
			return COLOR_FG_WHITE;
		case fscp::log_level::information:
			return COLOR_FG_BLUE;
		case fscp::log_level::important:
			return COLOR_BOLD + COLOR_FG_GREEN;
		case fscp::log_level::warning:
			return COLOR_FG_YELLOW;
		case fscp::log_level::error:
			return COLOR_FG_RED;
		case fscp::log_level::fatal:
			return COLOR_BOLD + COLOR_REVERSE + COLOR_FG_RED;
	}

	assert(false);
	return std::string();
}

#endif /* COLORS_HPP */

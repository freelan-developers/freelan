/*
 * libfreelan - A C++ library to establish peer-to-peer virtual private
 * networks.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libfreelan.
 *
 * libfreelan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfreelan is distributed in the hope that it will be useful, but
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
 * If you intend to use libfreelan in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file server_protocol.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The server protocol utility classes and methods.
 */

#include "server_protocol.hpp"

#include <cassert>
#include <stdexcept>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace freelan
{
	size_t server_protocol_parser::feed(boost::asio::const_buffer buf)
	{
		const char* _data = boost::asio::buffer_cast<const char*>(buf);
		size_t data_len = boost::asio::buffer_size(buf);

		m_data.append(_data, data_len);

		return data_len;
	}
	
	void server_protocol_parser::parse(const std::string& mime_type)
	{
		m_values.clear();

		if (mime_type.empty())
		{
			throw std::runtime_error("Unable to parse: no MIME type specified.");
		}
		else if (mime_type == "application/json")
		{
			parse_json();
		}
		else
		{
			throw std::runtime_error("An unsupported MIME type was specified: " + mime_type);
		}
	}

	void server_protocol_parser::parse_json()
	{
		rapidjson::Document document;

		document.Parse<0>(m_data.c_str());

		if (document.HasParseError())
		{
			throw std::runtime_error("JSON syntax parse error.");
		}

		if (!document.IsObject())
		{
			throw std::runtime_error("JSON document parse error: root must be an object.");
		}

		for (rapidjson::Document::ConstMemberIterator it = document.MemberBegin(); it != document.MemberEnd(); ++it)
		{
			const char* name = it->name.GetString();

			if (!it->value.IsString())
			{
				throw std::runtime_error("JSON document parse error: values must be strings (" + std::string(name) + ").");
			}

			const char* value = it->value.GetString();

			m_values[name] = value;
		}
	}
}

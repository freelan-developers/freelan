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

#include "message.hpp"

#include <cassert>

#include <boost/asio.hpp>

namespace freelan {
    namespace {
        void write_value(void* buf, size_t& offset, uint8_t value) {
            static_cast<char*>(buf)[offset] = value;

            offset += sizeof(value);
        }

        void write_value(void* buf, size_t& offset, uint16_t value) {
            *reinterpret_cast<uint16_t*>(&static_cast<char*>(buf)[offset]) = htons(value);

            offset += sizeof(value);
        }

        template <typename Type, typename OriginalType>
        void write_value(void* buf, size_t& offset, OriginalType value) {
            write_value(buf, offset, static_cast<Type>(value));
        }

        void write_buffer(void* buf, size_t& offset, const void* sbuf, size_t sbuf_len) {
            std::memcpy(&static_cast<char*>(buf)[offset], sbuf, sbuf_len);

            offset += sbuf_len;
        }
    }

    size_t write_fscp_message(void* buf, size_t buf_len, FSCPMessageType type, const void* payload, size_t payload_len, unsigned int version) {
        assert(buf);
        assert(payload);

        // If the payload is so big that it can't be represented in the message, fail.
        if (payload_len >= (1 << (sizeof(uint16_t) * 8))) {
            return 0;
        }

        // version | type | length
        const size_t resulting_size = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + payload_len;

        // If the output buffer is too small, fail.
        if (buf_len < resulting_size) {
            return 0;
        }

        size_t offset = 0;
        write_value<uint8_t>(buf, offset, version);
        write_value<uint8_t>(buf, offset, type);
        write_value<uint16_t>(buf, offset, payload_len);
        write_buffer(buf, offset, payload, payload_len);

        return resulting_size;
    }

    size_t write_fscp_hello_request_message(void* buf, size_t buf_len, uint32_t unique_number) {
        const uint32_t payload = htonl(unique_number);

        return write_fscp_message(buf, buf_len, FSCPMessageType::HELLO_REQUEST, &payload, sizeof(payload));
    }

    size_t write_fscp_hello_response_message(void* buf, size_t buf_len, uint32_t unique_number) {
        const uint32_t payload = htonl(unique_number);

        return write_fscp_message(buf, buf_len, FSCPMessageType::HELLO_RESPONSE, &payload, sizeof(payload));
    }
}

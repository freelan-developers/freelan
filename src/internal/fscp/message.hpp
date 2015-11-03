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
 * \file message.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief FSCP messages routines.
 */

#pragma once

#include <cstdlib>

#include "constants.hpp"

namespace freelan {
    /**
     * \brief Write a FSCP message to the specified buffer.
     * \param buf The buffer to write to. If buf is NULL, the call will return the minimal needed size for the write to succeed with the actual parameters.
     * \param buf_len The length of buf.
     * \param type The type of the FSCP message to write.
     * \param payload The payload to write. Cannot be NULL.
     * \param payload_len The length of payload.
     * \param version The version of the protocol. The default should be okay in most cases.
     * \return The total number of bytes written to buf. If the write fails, 0 is returned and buf is unchanged.
     */
    size_t write_fscp_message(void* buf, size_t buf_len, FSCPMessageType type, const void* payload, size_t payload_len, unsigned int version = FSCP_VERSION);

    /**
     * \brief Read a FSCP message from the specified buffer.
     * \param buf The buffer to read from. Cannot be NULL.
     * \param buf_len Thr length of buf.
     * \param type The type of FSCP message.
     * \param payload The payload of the FSCP message.
     * \param payload_len The length of the payload.
     * \param version If not null, *version will be set to the protocol version for the message.
     * \return true if the message was correctly formatted, false otherwise. If false is returned, the value of the specified attributes is unspecified.
     */
    bool read_fscp_message(const void* buf, size_t buf_len, FSCPMessageType& type, const void*& payload, size_t& payload_len, unsigned int* version = nullptr);

    size_t write_fscp_hello_request_message(void* buf, size_t buf_len, uint32_t unique_number);
    bool read_fscp_hello_request_message(const void* buf, size_t buf_len, uint32_t& unique_number);

    size_t write_fscp_hello_response_message(void* buf, size_t buf_len, uint32_t unique_number);
    bool read_fscp_hello_response_message(const void* buf, size_t buf_len, uint32_t& unique_number);
}

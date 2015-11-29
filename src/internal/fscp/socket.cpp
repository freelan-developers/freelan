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

#include "socket.hpp"

namespace freelan {
    void Socket::open(const Endpoint& listen_endpoint) {
        m_socket.open(listen_endpoint.protocol());

        if (listen_endpoint.address().is_v6())
        {
            // We accept both IPv4 and IPv6 addresses
            m_socket.set_option(boost::asio::ip::v6_only(false));
        }

        m_socket.bind(listen_endpoint);

        async_read();
    }

    void Socket::close() {
        m_socket.close();
    }

    void Socket::async_read() {
        auto buffer = std::make_shared<std::vector<char>>(65536);
        auto endpoint = std::make_shared<Endpoint>();

        m_socket.async_receive_from(boost::asio::buffer(*buffer), *endpoint, [this, buffer, endpoint](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec != boost::asio::error::operation_aborted) {
                // The read completed, let's start another one right away.
                async_read();

                process_received_buffer(*endpoint, &(*buffer)[0], bytes_transferred);
            }
        });
    }

    void Socket::process_received_buffer(const Endpoint& endpoint, const void* buf, size_t buf_len) {
        assert(buf);

        FSCPMessageType type {};
        const void* payload = nullptr;
        size_t payload_len = 0;

        if (read_fscp_message(buf, buf_len, type, payload, payload_len)) {
            switch (type) {
                case FSCPMessageType::HELLO_REQUEST: {
                    uint32_t unique_number = 0;

                    if (read_fscp_hello_request_message(payload, payload_len, unique_number)) {
                        const auto buffer = std::make_shared<std::vector<uint8_t>>(write_fscp_hello_response_message(unique_number));
                        assert(!buffer->empty());

                        m_write_queue.async_write(boost::asio::buffer(*buffer), endpoint, [buffer](const boost::system::error_code&, std::size_t) {});
                    }
                    break;
                }
                case FSCPMessageType::HELLO_RESPONSE: {
                    uint32_t unique_number = 0;

                    if (read_fscp_hello_response_message(payload, payload_len, unique_number)) {
                        const auto handler = m_endpoint_context_map.cancel_greet_response(endpoint, unique_number);

                        if (handler) {
                            handler(boost::system::error_code());
                        }
                    }
                    break;
                }
            }
        }
    }
}

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
 * \file socket.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A FSCP socket.
 */

#pragma once

#include <map>
#include <memory>
#include <queue>
#include <cassert>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>

#include "../async_utils.hpp"

#include "message.hpp"
#include "endpoint_context.hpp"

namespace freelan {
    class Socket {
        public:
            typedef boost::asio::ip::udp::socket::endpoint_type Endpoint;

            Socket(boost::asio::io_service& io_service) :
                m_socket(io_service),
                m_write_queue(m_socket)
            {}

            void open(const Endpoint& listen_endpoint);
            void close();

            template<typename Handler>
            void async_greet(const Endpoint& destination, Handler handler, const boost::posix_time::time_duration& timeout) {
                const auto unique_number = m_endpoint_context_map.register_greet_response_handler(destination, handler);
                const auto buffer = std::make_shared<std::vector<uint8_t>>(write_fscp_hello_request_message(unique_number));
                assert(!buffer->empty());

                m_write_queue.async_write(boost::asio::buffer(*buffer), destination, [this, buffer, destination, unique_number, timeout](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                    if (ec) {
                        const auto handler = m_endpoint_context_map.unregister_greet_response_handler(destination, unique_number);
                        handler(ec);
                    } else {
                        m_endpoint_context_map.async_wait_greet_response(destination, m_socket.get_io_service(), unique_number, timeout);
                    }
                });
            }

        private:
            void async_read();
            void process_received_buffer(const Endpoint& endpoint, const void* buf, size_t buf_len);

            class WriteQueue {
                public:
                    WriteQueue(boost::asio::ip::udp::socket& socket) :
                        m_socket(socket),
                        m_strand(socket.get_io_service())
                    {}

                    template <typename ConstBufferSequence, typename WriteHandler>
                    void async_write(const ConstBufferSequence& buffers, const Endpoint& destination, WriteHandler handler) {
                        const auto write_operation_completion = [this, handler](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                            m_strand.dispatch([this]() {
                                m_pending.pop();

                                // If the pending write list is not empty, trigger another write.
                                if (!m_pending.empty()) {
                                    m_pending.front()();
                                }
                            });

                            handler(ec, bytes_transferred);
                        };

                        const auto write_operation = [this, buffers, destination, write_operation_completion]() {
                            m_socket.async_send_to(buffers, destination, write_operation_completion);
                        };

                        m_strand.dispatch([this, write_operation]() {
                            if (m_pending.empty()) {
                                // No pending write, we can start one.
                                m_pending.push(write_operation);
                                write_operation();
                            } else {
                                // A write is pending. Let's queue the write operation.
                                m_pending.push(write_operation);
                            }
                        });
                    }

                private:
                    boost::asio::ip::udp::socket& m_socket;
                    boost::asio::strand m_strand;
                    std::queue<std::function<void()>> m_pending;
            };

            boost::asio::ip::udp::socket m_socket;
            WriteQueue m_write_queue;
            EndpointContextMap m_endpoint_context_map;
    };
}

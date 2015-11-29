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
 * \file endpoint_context.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief A FSCP endpoint context.
 */

#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freelan {
    class EndpointContext {
        public:
            typedef std::function<void(const boost::system::error_code& ec)> GreetResponseHandler;

            EndpointContext() :
                m_hello_request_counter(0)
            {}

        private:
            uint32_t register_greet_response_handler(GreetResponseHandler handler) {
                const auto unique_number = m_hello_request_counter++;
                m_greet_response_contexts[unique_number] = GreetResponseContext(handler);
                return unique_number;
            }

            GreetResponseHandler unregister_greet_response_handler(uint32_t unique_number) {
                const auto context = m_greet_response_contexts.find(unique_number);

                if (context == m_greet_response_contexts.end()) {
                    return nullptr;
                }

                const auto handler = context->second.handler;
                m_greet_response_contexts.erase(context);
                return handler;
            }

            template <typename TimeoutHandler>
            void async_wait_greet_response(boost::asio::io_service& io_service, uint32_t unique_number, const boost::posix_time::time_duration& timeout, TimeoutHandler timeout_handler) {
                auto context = m_greet_response_contexts.find(unique_number);
                assert(context != m_greet_response_contexts.end());
                assert(!context->second.timer);

                context->second.timer = std::make_shared<boost::asio::deadline_timer>(io_service, timeout);
                context->second.timer->async_wait([timeout_handler](const boost::system::error_code& ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        timeout_handler();
                    }
                });
            }

            GreetResponseHandler cancel_greet_response(uint32_t unique_number) {
                auto context = m_greet_response_contexts.find(unique_number);

                if (context == m_greet_response_contexts.end()) {
                    return nullptr;
                }

                assert(context->second.timer);

                auto timer = context->second.timer;
                const auto handler = context->second.handler;

                if (timer->cancel() == 0) {
                    // The timer couldn't get cancelled: it fired already or will soon.
                    // We don't return the handler as there is no point for the user in calling it.
                    return nullptr;
                }

                m_greet_response_contexts.erase(context);

                return handler;
            }

            struct GreetResponseContext {
                GreetResponseContext(GreetResponseHandler _handler = GreetResponseHandler()) :
                    timer(),
                    handler(_handler)
                {}

                std::shared_ptr<boost::asio::deadline_timer> timer;
                GreetResponseHandler handler;
            };

            std::atomic<uint32_t> m_hello_request_counter;
            std::map<uint32_t, GreetResponseContext> m_greet_response_contexts;

            friend class EndpointContextMap;
    };

    class EndpointContextMap {
        public:
            typedef boost::asio::ip::udp::socket::endpoint_type Endpoint;

            uint32_t register_greet_response_handler(const Endpoint& destination, EndpointContext::GreetResponseHandler handler) {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto& endpoint_context = m_endpoint_contexts[destination];
                return endpoint_context.register_greet_response_handler(handler);
            }

            EndpointContext::GreetResponseHandler unregister_greet_response_handler(const Endpoint& destination, uint32_t unique_number) {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto& endpoint_context = m_endpoint_contexts[destination];
                return endpoint_context.unregister_greet_response_handler(unique_number);
            }

            void async_wait_greet_response(const Endpoint& destination, boost::asio::io_service& io_service, uint32_t unique_number, const boost::posix_time::time_duration& timeout) {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto& endpoint_context = m_endpoint_contexts[destination];
                endpoint_context.async_wait_greet_response(io_service, unique_number, timeout, [this, destination, unique_number]() {
                    const auto handler = unregister_greet_response_handler(destination, unique_number);

                    if (handler) {
                        handler(boost::asio::error::timed_out);
                    }
                });
            }

            EndpointContext::GreetResponseHandler cancel_greet_response(const Endpoint& destination, uint32_t unique_number) {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto& endpoint_context = m_endpoint_contexts[destination];
                return endpoint_context.cancel_greet_response(unique_number);
            }


        private:
            mutable std::mutex m_mutex;
            std::map<Endpoint, EndpointContext> m_endpoint_contexts;
    };
}

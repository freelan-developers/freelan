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
 * \file async_utils.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Asynchronous I/O utility functions and classes.
 */

#pragma once

#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace freelan {
    template <typename Operation, typename Handler, typename OperationHandler>
    void start_operation(
        boost::asio::io_service& io_service,
        Operation operation,
        Handler handler,
        OperationHandler operation_handler,
        const boost::posix_time::time_duration& timeout,
        const boost::system::error_code& expiration_ec = boost::asio::error::timed_out
    ) {
        const auto wrapper_handler = [&io_service, handler, operation_handler, timeout, expiration_ec](const boost::system::error_code& ec) {
            if (ec) {
                // The operation failed immediately. Ignore the operation handler and report the failure directly.
                handler(ec);
            }
            else {
                // The operation succeeded. Create a timer and report it to the user.
                const auto timer = std::make_shared<boost::asio::deadline_timer>(io_service);
                timer->expires_from_now(timeout);
                timer->async_wait([timer, handler, expiration_ec](const boost::system::error_code& ec2) {
                    if (ec2 != boost::asio::error::operation_aborted) {
                        handler(expiration_ec);
                    }
                });

                const auto stop_operation = [timer, handler](const boost::system::error_code& ec2) -> bool {
                    if (timer->cancel() > 0) {
                        handler(ec2);

                        return true;
                    }
                    else {
                        // The timer's callback was already executed or will soon be. We failed.
                        return false;
                    }
                };

                operation_handler(stop_operation);
            }
        };

        operation(wrapper_handler);
    }
}

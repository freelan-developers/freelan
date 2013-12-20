/*
 * libfscp - C++ portable OpenSSL cryptographic wrapper library.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libfscp.
 *
 * libfscp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libfscp is distributed in the hope that it will be useful, but
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
 * If you intend to use libfscp in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file server.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The server class.
 */

#ifndef FSCP_SERVER_HPP_2
#define FSCP_SERVER_HPP_2

#include "identity_store.hpp"
#include "memory_pool.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <stdint.h>

namespace fscp
{
	/**
	 * \brief A FSCP server.
	 */
	class server2
	{
		public:

			/**
			 * \brief The endpoint type.
			 */
			typedef boost::asio::ip::udp::endpoint ep_type;

			/**
			 * \brief The certificate type.
			 */
			typedef cryptoplus::x509::certificate cert_type;

			/**
			 * \brief A simple operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&)> simple_handler_type;

			/**
			 * \brief A duration operation handler.
			 */
			typedef boost::function<void (const boost::system::error_code&, const boost::posix_time::time_duration& duration)> duration_handler_type;

			/**
			 * \brief Create a new FSCP server.
			 * \param io_service The Boost Asio io_service instance to associate with the server.
			 * \param identity The identity store.
			 */
			server2(boost::asio::io_service& io_service, const identity_store& identity);

			/**
			 * \brief Get the associated io_service.
			 * \return The associated io_service.
			 */
			boost::asio::io_service& get_io_service()
			{
				return m_socket.get_io_service();
			}

			/**
			 * \brief Open the server.
			 * \param listen_endpoint The listen endpoint.
			 */
			void open(const ep_type& listen_endpoint);

			/**
			 * \brief Close the server.
			 *
			 * This method can be called from another thread.
			 */
			void close();

			/**
			 * \brief Greet an host.
			 * \param target The target to greet.
			 * \param handler The handler to call when a reply was received, an error occured or the request timed out.
			 * \param timeout The maximum time to wait for a reply.
			 */
			void async_greet(const ep_type& target, duration_handler_type handler, const boost::posix_time::time_duration& timeout = boost::posix_time::seconds(3));

			/**
			 * \brief Cancel all pending greetings.
			 */
			void cancel_all_greetings();

		private:

			template <typename MutableBufferSequence, typename ReadHandler>
			void async_receive_from(const MutableBufferSequence& data, ep_type& sender, ReadHandler handler)
			{
				m_socket_strand.post(boost::bind(&boost::asio::ip::udp::socket::async_receive_from<MutableBufferSequence, ReadHandler>, &m_socket, data, boost::ref(sender), 0, handler));
			}

			ep_type to_socket_format(const server2::ep_type& ep);

			template <typename ConstBufferSequence, typename WriteHandler>
			void async_send_to(const ConstBufferSequence& data, const ep_type& target, WriteHandler handler)
			{
				m_socket_strand.post(boost::bind(&boost::asio::ip::udp::socket::async_send_to<ConstBufferSequence, WriteHandler>, &m_socket, data, to_socket_format(target), 0, handler));
			}

			identity_store m_identity_store;

			boost::asio::ip::udp::socket m_socket;
			boost::asio::strand m_socket_strand;

		private:

			/**
			 * @brief Represents a context for an endpoint.
			 */
			class ep_hello_context_type
			{
				public:

					/**
					 * @brief Generate an unique number.
					 * @warning The first invocation of this function is *NOT* thread-safe in C++03.
					 */
					static uint32_t generate_unique_number();

					/**
					 * @brief Create a new context.
					 */
					ep_hello_context_type();

					/**
					 * @brief Returns the current hello unique number and generates a new one.
					 * @return The current hello unique number.
					 */
					uint32_t next_hello_unique_number();

					/**
					 * @brief Asynchronously waits for a hello reply.
					 * @param io_service The io_service instance to use for the wait.
					 * @param hello_unique_number The unique hello number.
					 * @param timeout The time to wait for the reply.
					 * @param handler The handler to call upon timeout or cancellation.
					 */
					template <typename WaitHandler>
					void async_wait_reply(boost::asio::io_service& io_service, uint32_t hello_unique_number, const boost::posix_time::time_duration& timeout, WaitHandler handler);

					/**
					 * @brief Cancel a hello reply wait timer.
					 * @param hello_unique_number The hello reply number.
					 * @param success Whether the cancel is the result of a received reply.
					 * @return true if the timer was cancelled or false if it was too late to do so.
					 */
					bool cancel_reply_wait(uint32_t hello_unique_number, bool success);

					/**
					 * @brief Cancel all pending hello request wait timers.
					 *
					 * This call is similar to calling cancel_reply_wait(<num>, false) for all hello unique numbers.
					 */
					void cancel_all_reply_wait();

					/**
					 * @brief Remove a hello reply wait from the pending list.
					 * @param hello_unique_number The hello reply number.
					 * @param duration A variable whose value after the call will be the time elapsed since the creation of the request.
					 * @return The success status of the request.
					 */
					bool remove_reply_wait(uint32_t hello_unique_number, boost::posix_time::time_duration& duration);

				private:

					struct pending_request_status
					{
						pending_request_status() :
							timer(),
							start_date(boost::posix_time::microsec_clock::universal_time()),
							success(false)
						{}

						pending_request_status(boost::shared_ptr<boost::asio::deadline_timer> _timer) :
							timer(_timer),
							start_date(boost::posix_time::microsec_clock::universal_time()),
							success(false)
						{}

						boost::shared_ptr<boost::asio::deadline_timer> timer;
						boost::posix_time::ptime start_date;
						bool success;
					};

					typedef std::map<uint32_t, pending_request_status> pending_requests_map;

					uint32_t m_current_hello_unique_number;
					pending_requests_map m_pending_requests;
			};

			typedef memory_pool<16> greet_memory_pool;
			typedef std::map<ep_type, ep_hello_context_type> ep_hello_context_map;

			void do_greet(const ep_type&, duration_handler_type, const boost::posix_time::time_duration&);
			void do_greet_handler(const ep_type&, uint32_t, duration_handler_type, const boost::posix_time::time_duration&, const boost::system::error_code&, size_t);
			void do_greet_timeout(const ep_type&, uint32_t, duration_handler_type, const boost::system::error_code&);
			void do_cancel_all_greetings();

			ep_hello_context_map m_ep_hello_contexts;
			boost::asio::strand m_greet_strand;
			greet_memory_pool m_greet_memory_pool;
	};
}

#endif /* FSCP_SERVER_HPP */

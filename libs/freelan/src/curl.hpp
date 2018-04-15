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
 * \file curl.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The cURL structures and functions.
 */

#ifndef FREELAN_CURL_HPP
#define FREELAN_CURL_HPP

#include <curl/curl.h>

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <asiotap/types/endpoint.hpp>

namespace freelan
{

	/**
	 * \brief A CURL slist.
	 */
	class curl_list
	{
		public:

			/**
			 * \brief Create a CURL slist.
			 */
			curl_list();

			/**
			 * \brief The move constructor.
			 * \param other The other instance to move from.
			 */
			curl_list(curl_list&& other) :
				m_slist(std::move(other.m_slist))
			{
			}

			/**
			* \brief The move assignment operator.
			* \param other The other instance to move from.
			* \return The current instance.
			*/
			curl_list& operator=(curl_list&& other)
			{
				m_slist = std::move(other.m_slist);

				return *this;
			}

			/**
			 * \brief Append a value to the list.
			 */
			void append(const std::string& value);

			/**
			 * \brief Reset the list.
			 */
			void reset();

			/**
			 * \brief Get the raw pointer for the slist.
			 * \return The raw pointer.
			 */
			struct curl_slist* raw() const;

		private:

			std::unique_ptr<curl_slist, void (*)(curl_slist*)> m_slist;

			friend class curl;
	};

	typedef curl_socket_t (*curl_open_socket_callback)(void*, curlsocktype, struct curl_sockaddr*);
	typedef int (*curl_close_socket_callback)(void*, curl_socket_t);

	/**
	 * \brief A CURL wrapper class.
	 */
	class curl
	{
		public:

			/**
			 * \brief A debug function type.
			 */
			typedef boost::function<void (curl_infotype, boost::asio::mutable_buffer)> debug_function_t;

			/**
			 * \brief A write function type.
			 */
			typedef boost::function<size_t (boost::asio::const_buffer)> write_function_t;

			/**
			 * \brief A default write function.
			 * \param data The data.
			 * \return The size of data, in bytes.
			 */
			static size_t default_write_function(boost::asio::const_buffer data)
			{
				return boost::asio::buffer_size(data);
			}

			/**
			 * \brief Create a CURL.
			 */
			curl();

			/**
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 */
			CURL* raw() const
			{
				return m_curl.get();
			}

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, void* value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, const void* value) { set_option(option, const_cast<void*>(value)); }

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, long int value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, curl_debug_callback value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, curl_write_callback value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, curl_open_socket_callback value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLoption option, curl_close_socket_callback value);

			/**
			 * \brief Set the HTTP(S) proxy to use.
			 * \param proxy The proxy.
			 */
			void set_proxy(const asiotap::endpoint& proxy);

			/**
			 * \brief Set a debug function.
			 * \param func The debug function.
			 */
			void set_debug_function(debug_function_t func);

			/**
			 * \brief Set the write function.
			 * \param func The write function.
			 */
			void set_write_function(write_function_t func);

			/**
			 * \brief Set the user agent.
			 * \param user_agent The user agent to set.
			 */
			void set_user_agent(const std::string& user_agent);

			/**
			 * \brief Set the url.
			 * \param url The url.
			 */
			void set_url(const std::string& url);

			/**
			 * \brief Enable or disable the SSL peer verification method.
			 * \param state The state.
			 */
			void set_ssl_peer_verification(bool state);

			/**
			 * \brief Enable or disable the SSL host verification method.
			 * \param state The state.
			 */
			void set_ssl_host_verification(bool state);

			/**
			 * \brief Set the cainfo path.
			 * \param ca_info The cainfo path..
			 */
			void set_ca_info(const boost::filesystem::path& ca_info);

			/**
			 * \brief Set the connect timeout.
			 * \param timeout The timeout.
			 */
			void set_connect_timeout(const boost::posix_time::time_duration& timeout);

			/**
			* \brief Set the timeout.
			* \param timeout The timeout.
			*/
			void set_timeout(const boost::posix_time::time_duration& timeout);

			/**
			 * \brief Set a HTTP header.
			 * \param header The header.
			 * \param value The value.
			 */
			void set_http_header(const std::string& header, const std::string& value = "\"\"");

			/**
			 * \brief Unset a HTTP header.
			 * \param header The header.
			 */
			void unset_http_header(const std::string& header);

			/**
			 * \brief Reset all HTTP headers.
			 */
			void reset_http_headers();

			/**
			 * \brief Set the GET method for the request.
			 */
			void set_get();

			/**
			 * \brief Set the POST method for the request.
			 */
			void set_post();

			/**
			 * \brief Sets the post fields.
			 * \param buf The buffer containing the data.
			 *
			 * buf must remain valid until to curl instance gets destroyed or
			 * set_post_fields() gets called again.
			 */
			void set_post_fields(boost::asio::const_buffer buf);

			/**
			 * \brief Sets the post fields.
			 * \param buf The buffer containing the data.
			 *
			 * The specified data is copied by the curl library so it may be modified
			 * or deleted once the call is complete.
			 */
			void set_copy_post_fields(boost::asio::const_buffer buf);

			/**
			 * \brief Set the cookie file.
			 * \param file The file.
			 */
			void set_cookie_file(const std::string& file);

			/**
			 * \brief Enable cookies support.
			 */
			void enable_cookie_support();

			/**
			 * \brief Set the username for authentication.
			 * \param username The username.
			 */
			void set_username(const std::string& username);

			/**
			 * \brief Set the password for authentication.
			 * \param password The password.
			 */
			void set_password(const std::string& password);

			/**
			 * \brief Escape the specified string using url encoding.
			 * \param url The url to encode.
			 * \return The encoded url.
			 */
			std::string escape(const std::string& url);

			/**
			 * \brief Unescape the specified encoded string url encoding.
			 * \param encoded The encoded string.
			 * \return The decoded string.
			 */
			std::string unescape(const std::string& encoded);

			/**
			 * \brief Perform the cURL request.
			 *
			 * On error, an exception is thrown.
			 */
			void perform();

			/**
			 * \brief Get the effective url.
			 * \return The effective url.
			 */
			std::string get_effective_url();

			/**
			 * \brief Get the last response code.
			 * \return The last response code.
			 */
			long get_response_code();

			/**
			 * \brief Get the content length for downloads.
			 * \return The content length or -1, if no content length is specified.
			 */
			ptrdiff_t get_content_length_download();

			/**
			 * \brief Get the content length for uploads.
			 * \return The content length or -1, if no content length is specified.
			 */
			ptrdiff_t get_content_length_upload();

			/**
			 * \brief Get the content type.
			 * \return The content type or an empty string if no content type is specified.
			 */
			std::string get_content_type();

			/**
			 * \brief Get the system error.
			 * \return The system error.
			 */
			boost::system::error_code get_system_error();

		private:

			static int debug_function(CURL*, curl_infotype, char*, size_t, void*);
			static size_t write_function(char*, size_t, size_t, void*);

			std::unique_ptr<CURL, void (*)(CURL*)> m_curl;
			curl_list m_http_headers;
			debug_function_t m_debug_function;
			write_function_t m_write_function;

			friend class curl_multi;
	};

	class curl_association;

	/**
	 * \brief A CURLM wrapper class.
	 */
	class curl_multi : public boost::enable_shared_from_this<curl_multi>
	{
		public:

			/**
			 * \brief Create a CURLM.
			 */
			static boost::shared_ptr<curl_multi> create()
			{
				return boost::shared_ptr<curl_multi>(new curl_multi());
			}

			/**
			 * \brief Destroy a CURLM.
			 */
			virtual ~curl_multi() {};

			/**
			 * \brief Get the raw pointer.
			 * \return The raw pointer.
			 */
			CURLM* raw() const
			{
				return m_curlm.get();
			}

			/**
			 * \brief Add a handle to this CURLM.
			 * \param handle The handle to add.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			virtual void add_handle(boost::shared_ptr<curl> handle);

			/**
			 * \brief Get a handle from this CURLM.
			 * \param easy_handle The CURL handle to get.
			 * \return The curl instance that was first added.
			 */
			virtual boost::shared_ptr<curl> get_handle(CURL* easy_handle) const;

			/**
			 * \brief Remove a handle from this CURLM.
			 * \param easy_handle The CURL handle to remove.
			 * \return The curl instance that was first added.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			virtual boost::shared_ptr<curl> remove_handle(CURL* easy_handle);

			/**
			 * \brief Clear all the handles from this CURLM.
			 * \return All the curl instance previously handled.
			 */
			virtual std::vector<boost::shared_ptr<curl>> clear();

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLMoption option, void* value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLMoption option, long int value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLMoption option, curl_multi_timer_callback value);

			/**
			 * \brief Set an option.
			 * \param option The option.
			 * \param value The option value.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void set_option(CURLMoption option, curl_socket_callback value);

			/**
			 * \brief Perform a socket action.
			 */
			void socket_action(curl_socket_t sockfd, int ev_bitmask, int* running_handles);

			/**
			 * \brief Read information from the curl multi handle.
			 * \param count_left A pointer to an integer that must receive the count of remaining messages.
			 * \return A pointer to the information structure if any, or nullptr otherwise.
			 */
			CURLMsg* info_read(int* count_left = nullptr);

		protected:
			curl_multi();

			virtual void before_associate_handle(boost::shared_ptr<curl>)
			{
			}

			virtual void after_associate_handle(boost::shared_ptr<curl>)
			{
			}

			virtual void before_disassociate_handle(boost::shared_ptr<curl>)
			{
			}

			virtual void after_disassociate_handle(boost::shared_ptr<curl>)
			{
			}

		private:

			std::unique_ptr<CURLM, void (*)(CURLM*)> m_curlm;
			std::map<CURL*, std::unique_ptr<curl_association>> m_associations;

			friend class curl_association;
	};

	/**
	 * An association class between a curl and a curl multi.
	 */
	class curl_association
	{
		public:
			curl_association(boost::shared_ptr<curl_multi> _curl_multi, boost::shared_ptr<curl> _curl);
			~curl_association();

			boost::shared_ptr<curl_multi> get_curl_multi() const
			{
				return m_curl_multi;
			}

			boost::shared_ptr<curl> get_curl() const
			{
				return m_curl;
			}

		private:
			curl_association(const curl_association&) = delete;
			curl_association& operator=(const curl_association&) = delete;

			boost::shared_ptr<curl_multi> m_curl_multi;
			boost::shared_ptr<curl> m_curl;
	};

	/**
	 * \brief A CURL multi wrapper class compatible with Boost ASIO.
	 */
	class curl_multi_asio : public curl_multi
	{
		public:
			/**
			 * \brief The connection complete callback.
			 */
			typedef boost::function<void (const boost::system::error_code&)> connection_complete_callback;

			/**
			 * Create a new instance.
			 */
			static boost::shared_ptr<curl_multi_asio> create(boost::asio::io_service& io_service)
			{
				return boost::shared_ptr<curl_multi_asio>(new curl_multi_asio(io_service));
			}

			/**
			 * \brief The destructor.
			 */
			~curl_multi_asio();

			boost::shared_ptr<curl_multi_asio> shared_from_this()
			{
				return boost::static_pointer_cast<curl_multi_asio>(curl_multi::shared_from_this());
			}

			/**
			 * \brief Post a handle to this CURLM, asynchronously.
			 * \param handle The handle to add.
			 * \param handler The handler to call upon completion. This handler will *NOT* be
			 * called once the handle was added but when the handle associated operation
			 * completes.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void execute(boost::shared_ptr<curl> handle, connection_complete_callback handler = connection_complete_callback());

			/**
			 * \brief Clear all the handles from this CURLM, asynchronously.
			 */
			void async_clear(boost::function<void ()> handler = boost::function<void ()>());

		protected:

			void before_associate_handle(boost::shared_ptr<curl>) override;
			void after_disassociate_handle(boost::shared_ptr<curl>) override;

		private:

			class curl_socket : private boost::asio::ip::tcp::socket, public enable_shared_from_this<curl_socket>
			{
				public:
					template <typename... Values>
					static boost::shared_ptr<curl_socket> create(Values&&... values)
					{
						return boost::shared_ptr<curl_socket>(new curl_socket(std::forward<Values>(values)...));
					}

					using boost::asio::ip::tcp::socket::cancel;
					using boost::asio::ip::tcp::socket::native_handle;

					int current_action() const
					{
						return m_current_action;
					}

					void set_current_action(int action)
					{
						m_current_action = action;
					}

					void trigger(curl_multi_asio& _curl_multi_asio);
					void trigger_read(curl_multi_asio& _curl_multi_asio);
					void trigger_write(curl_multi_asio& _curl_multi_asio);

				private:
					template <typename... Values>
					curl_socket(Values&&... values) :
						boost::asio::ip::tcp::socket(std::forward<Values>(values)...),
						m_current_action(0),
						m_read_operation_pending(false),
						m_write_operation_pending(false)
					{
					}

					int m_current_action;
					bool m_read_operation_pending;
					bool m_write_operation_pending;
			};

			static int static_timer_callback(CURLM*, long, void*);
			static int static_socket_callback(CURL*, curl_socket_t, int, void*, void*);
			static curl_socket_t open_socket_callback(void* _curl_multi_asio, curlsocktype purpose, struct curl_sockaddr* address);
			static int close_socket_callback(void* _curl_multi_asio, curl_socket_t item);

			curl_multi_asio(boost::asio::io_service& io_service);
			void timer_callback(const boost::system::error_code& ec);
			void socket_callback(const boost::system::error_code& ec, boost::shared_ptr<curl_socket> socket);
			void check_info();

			boost::asio::io_service& m_io_service;
#if BOOST_ASIO_VERSION >= 101200 // Boost 1.66+
			boost::asio::io_context::strand m_strand;
#else
			boost::asio::strand m_strand;
#endif
			boost::asio::deadline_timer m_timer;
			std::map<boost::shared_ptr<curl>, connection_complete_callback> m_handler_map;
			std::map<boost::shared_ptr<curl>, CURLcode> m_result_map;
			std::map<curl_socket_t, boost::shared_ptr<curl_socket>> m_socket_map;
	};
}

#endif /* FREELAN_CURL_HPP */

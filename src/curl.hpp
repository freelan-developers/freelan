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

#include "endpoint.hpp"

namespace freelan
{
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
			 * \brief Create a CURL.
			 */
			curl();

			/**
			 * \brief Destroy a CURL.
			 */
			~curl();

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
			 * \brief Set the HTTP(S) proxy to use.
			 * \param proxy The proxy.
			 */
			void set_proxy(const endpoint& proxy);

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

		private:

			static int debug_function(CURL*, curl_infotype, char*, size_t, void*);
			static size_t write_function(char*, size_t, size_t, void*);

			curl(const curl&);
			curl& operator=(const curl&);

			CURL* m_curl;
			debug_function_t m_debug_function;
			write_function_t m_write_function;
			
			friend class curl_multi;
	};

	/**
	 * \brief A CURLM wrapper class.
	 */
	class curl_multi
	{
		public:

			/**
			 * \brief Create a CURLM.
			 */
			curl_multi();

			/**
			 * \brief Destroy a CURLM.
			 */
			~curl_multi();

			/**
			 * \brief Add a handle to this CURLM.
			 * \param handle The handle to add.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void add_handle(const curl& handle);

			/**
			 * \brief Remove a handle from this CURLM.
			 * \param handle The handle to remove. Must have been added with
			 * curl_multi::add_handle() first or the behavior is undefined.
			 *
			 * On error, a std::runtime_error is raised.
			 */
			void remove_handle(const curl& handle);

		private:

			curl_multi(const curl_multi&);
			curl_multi& operator=(const curl_multi&);

			CURLM* m_curlm;
	};
}

#endif /* FREELAN_CURL_HPP */

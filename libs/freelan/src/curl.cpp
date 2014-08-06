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
 * \file curl.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The curl structures and functions.
 */

#include "curl.hpp"

#include <cassert>
#include <stdexcept>

namespace freelan
{
	namespace
	{
		void throw_if_curl_error(CURLcode errorcode)
		{
			if (errorcode != CURLE_OK)
			{
				throw std::runtime_error(curl_easy_strerror(errorcode));
			}
		}

		void throw_if_curlm_error(CURLMcode errorcode)
		{
			if (errorcode != CURLM_OK)
			{
				throw std::runtime_error(curl_multi_strerror(errorcode));
			}
		}
	}

	curl_list::curl_list() :
		m_slist(NULL)
	{
	}

	curl_list::~curl_list()
	{
		reset();
	}

	void curl_list::append(const std::string& value)
	{
		struct curl_slist* const new_slist = curl_slist_append(m_slist, value.c_str());

		if (!new_slist)
		{
			throw std::runtime_error("Unable to append a value to the list");
		}

		m_slist = new_slist;
	}

	void curl_list::reset()
	{
		if (m_slist)
		{
			curl_slist_free_all(m_slist);
		}
	}

	struct curl_slist* curl_list::raw() const
	{
		return m_slist;
	}

	curl::curl() :
		m_curl(curl_easy_init(), [] (CURL* p) { if (p) curl_easy_cleanup(p); }),
		m_debug_function()
	{
		if (!m_curl)
		{
			throw std::runtime_error("Unable to allocate a CURL structure");
		}
	}

	void curl::set_option(CURLoption option, void* value)
	{
		throw_if_curl_error(curl_easy_setopt(m_curl.get(), option, value));
	}

	void curl::set_option(CURLoption option, long int value)
	{
		throw_if_curl_error(curl_easy_setopt(m_curl.get(), option, value));
	}

	void curl::set_option(CURLoption option, curl_debug_callback value)
	{
		throw_if_curl_error(curl_easy_setopt(m_curl.get(), option, value));
	}

	void curl::set_option(CURLoption option, curl_write_callback value)
	{
		throw_if_curl_error(curl_easy_setopt(m_curl.get(), option, value));
	}

	void curl::set_proxy(const asiotap::endpoint& proxy)
	{
		if (proxy != asiotap::hostname_endpoint::null())
		{
			set_option(CURLOPT_PROXY, static_cast<const void*>(boost::lexical_cast<std::string>(proxy).c_str()));
		}
		else
		{
			set_option(CURLOPT_PROXY, static_cast<const void*>(NULL));
		}
	}

	void curl::set_debug_function(debug_function_t func)
	{
		m_debug_function = func;

		if (m_debug_function)
		{
			set_option(CURLOPT_DEBUGFUNCTION, &curl::debug_function);
			set_option(CURLOPT_DEBUGDATA, &m_debug_function);
		}
		else
		{
			set_option(CURLOPT_DEBUGFUNCTION, static_cast<void*>(NULL));
			set_option(CURLOPT_DEBUGDATA, static_cast<void*>(NULL));
		}
	}

	void curl::set_write_function(write_function_t func)
	{
		m_write_function = func;

		if (m_write_function)
		{
			set_option(CURLOPT_WRITEFUNCTION, &curl::write_function);
			set_option(CURLOPT_WRITEDATA, &m_write_function);
		}
		else
		{
			set_option(CURLOPT_WRITEFUNCTION, static_cast<void*>(NULL));
			set_option(CURLOPT_WRITEDATA, static_cast<void*>(NULL));
		}
	}

	void curl::set_user_agent(const std::string& user_agent)
	{
		set_option(CURLOPT_USERAGENT, static_cast<const void*>(user_agent.c_str()));
	}

	void curl::set_url(const std::string& url)
	{
		set_option(CURLOPT_URL, static_cast<const void*>(url.c_str()));
	}

	void curl::set_ssl_peer_verification(bool state)
	{
		set_option(CURLOPT_SSL_VERIFYPEER, state ? 1L : 0L);
	}

	void curl::set_ssl_host_verification(bool state)
	{
		set_option(CURLOPT_SSL_VERIFYHOST, state ? 2L : 0L);
	}

	void curl::set_ca_info(const boost::filesystem::path& ca_info)
	{
		if (ca_info.empty())
		{
			set_option(CURLOPT_CAINFO, static_cast<const void*>(NULL));
		}
		else
		{
			set_option(CURLOPT_CAINFO, static_cast<const void*>(ca_info.string().c_str()));
		}
	}

	void curl::set_connect_timeout(const boost::posix_time::time_duration& timeout)
	{
		set_option(CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(timeout.total_milliseconds()));
	}

	void curl::set_http_header(const std::string& header, const std::string& value)
	{
		m_http_headers.append(header + ": " + value);

		set_option(CURLOPT_HTTPHEADER, static_cast<const void*>(m_http_headers.raw()));
	}

	void curl::unset_http_header(const std::string& header)
	{
		m_http_headers.append(header + ":");

		set_option(CURLOPT_HTTPHEADER, static_cast<const void*>(m_http_headers.raw()));
	}

	void curl::reset_http_headers()
	{
		m_http_headers = curl_list();

		set_option(CURLOPT_HTTPHEADER, static_cast<const void*>(m_http_headers.raw()));
	}

	void curl::set_get()
	{
		set_option(CURLOPT_HTTPGET, 1L);
	}

	void curl::set_post()
	{
		set_option(CURLOPT_POST, 1L);
	}

	void curl::set_post_fields(boost::asio::const_buffer buf)
	{
		set_option(CURLOPT_POSTFIELDSIZE_LARGE, static_cast<long>(boost::asio::buffer_size(buf)));
		set_option(CURLOPT_POSTFIELDS, boost::asio::buffer_cast<const void*>(buf));
	}

	void curl::set_copy_post_fields(boost::asio::const_buffer buf)
	{
		set_option(CURLOPT_POSTFIELDSIZE_LARGE, static_cast<long>(boost::asio::buffer_size(buf)));
		set_option(CURLOPT_COPYPOSTFIELDS, boost::asio::buffer_cast<const void*>(buf));
	}

	void curl::set_cookie_file(const std::string& file)
	{
		set_option(CURLOPT_COOKIEFILE, file.c_str());
	}

	void curl::enable_cookie_support()
	{
		set_cookie_file("");
	}

	void curl::set_username(const std::string& username)
	{
		set_option(CURLOPT_USERNAME, username.c_str());
	}

	void curl::set_password(const std::string& password)
	{
		set_option(CURLOPT_PASSWORD, password.c_str());
	}

	std::string curl::escape(const std::string& url)
	{
		char* rstr = curl_easy_escape(m_curl.get(), url.c_str(), static_cast<int>(url.size()));

		if (!rstr)
		{
			throw std::bad_alloc();
		}

		boost::shared_ptr<char> str(rstr, curl_free);

		return std::string(str.get());
	}

	std::string curl::unescape(const std::string& encoded)
	{
		int len = 0;
		char* rstr = curl_easy_unescape(m_curl.get(), encoded.c_str(), static_cast<int>(encoded.size()), &len);

		if (!rstr)
		{
			throw std::bad_alloc();
		}

		boost::shared_ptr<char> str(rstr, curl_free);

		return std::string(str.get(), static_cast<size_t>(len));
	}

	void curl::perform()
	{
		throw_if_curl_error(curl_easy_perform(m_curl.get()));
	}

	long curl::get_response_code()
	{
		long response_code = 0;

		throw_if_curl_error(curl_easy_getinfo(m_curl.get(), CURLINFO_RESPONSE_CODE, &response_code));

		return response_code;
	}

	ptrdiff_t curl::get_content_length_download()
	{
		double content_length = 0.0;

		throw_if_curl_error(curl_easy_getinfo(m_curl.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length));

		return (content_length >= 0) ? static_cast<ptrdiff_t>(content_length) : -1;
	}

	ptrdiff_t curl::get_content_length_upload()
	{
		double content_length = 0.0;

		throw_if_curl_error(curl_easy_getinfo(m_curl.get(), CURLINFO_CONTENT_LENGTH_UPLOAD, &content_length));

		return (content_length >= 0) ? static_cast<ptrdiff_t>(content_length) : -1;
	}

	std::string curl::get_content_type()
	{
		char* content_type = NULL;

		throw_if_curl_error(curl_easy_getinfo(m_curl.get(), CURLINFO_CONTENT_TYPE, &content_type));

		return content_type ? content_type : "";
	}

	int curl::debug_function(CURL*, curl_infotype infotype, char* data, size_t datalen, void* context)
	{
		assert(context);

		debug_function_t& func = *static_cast<debug_function_t*>(context);

		func(infotype, boost::asio::buffer(data, datalen));

		return 0;
	}

	size_t curl::write_function(char* data, size_t size, size_t nmemb, void* context)
	{
		assert(context);

		write_function_t& func = *static_cast<write_function_t*>(context);

		return func(boost::asio::buffer(data, size * nmemb));
	}

	curl_multi::curl_multi() :
		m_curlm(curl_multi_init(), [](CURLM* p){ if (p) curl_multi_cleanup(p); })
	{
		if (!m_curlm)
		{
			throw std::runtime_error("Unable to allocate a CURLM structure");
		}
	}

	void curl_multi::add_handle(const curl& handle)
	{
		throw_if_curlm_error(curl_multi_add_handle(m_curlm.get(), handle.m_curl.get()));
	}

	void curl_multi::remove_handle(const curl& handle)
	{
		throw_if_curlm_error(curl_multi_remove_handle(m_curlm.get(), handle.m_curl.get()));
	}
}

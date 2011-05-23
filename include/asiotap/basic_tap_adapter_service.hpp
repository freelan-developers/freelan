/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien Kauffmann <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
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
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file basic_tap_adapter_service.hpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief The basic tap adapter service class.
 */

#ifndef ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP
#define ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "tap_adapter_impl.hpp"

namespace asiotap
{
	template <typename TapAdapterImplementation = tap_adapter_impl>
	class basic_tap_adatper_service : public boost::asio::io_service::service
	{
		public:

			/**
			 * \brief The implementation pointer type.
			 */
			typedef boost::shared_ptr<TapAdapterImplementation> implementation_type;

			/**
			 * \brief The service identifier.
			 */
			static boost::asio::io_service::id id;

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to register to.
			 */
			explicit basic_tap_adatper_service(boost::asio::io_service &io_service);

			/**
			 * \brief Construct an implementation.
			 * \param impl The implementation to construct.
			 */
			void construct(implementation_type& impl);

			/**
			 * \brief Destroy an implementation.
			 * \param impl The implementation to destroy.
			 */
			void destroy(implementation_type& impl);

			/**
			 * \brief Process to an asynchronous read on the specified implementation.
			 * \param impl The implementation on which to perform the read.
			 * \param buffer The buffer.
			 * \param handler The handler.
			 */
			template<typename ReadHandler>
			void async_read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, ReadHandler handler);

		private:

			template <typename ReadHandler>
			class read_operation
			{
				public:

					read_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::mutable_buffer& buffer, ReadHandler handler);
					void operator()() const;

				private:

					boost::weak_ptr<TapAdapterImplementation> m_impl;
					boost::asio::io_service& m_io_service;
					boost::asio::io_service::work m_work;
					const boost::asio::mutable_buffer& m_buffer;
					ReadHandler m_handler;
			};

			void shutdown_service();
	};

	template <typename TapAdapterImplementation>
	boost::asio::io_service::id basic_tap_adatper_service<TapAdapterImplementation>::id;
	
	template <typename TapAdapterImplementation>
	inline basic_tap_adatper_service<TapAdapterImplementation>::basic_tap_adatper_service(boost::asio::io_service &_io_service) :
		boost::asio::io_service::service(_io_service)
	{
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adatper_service<TapAdapterImplementation>::construct(implementation_type& impl)
	{
		impl.reset(new TapAdapterImplementation());
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adatper_service<TapAdapterImplementation>::destroy(implementation_type& impl)
	{
		impl.reset();
	}
	
	template<typename TapAdapterImplementation> template<typename ReadHandler>
	inline void basic_tap_adatper_service<TapAdapterImplementation>::async_read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, ReadHandler handler)
	{

	}

	template <typename TapAdapterImplementation>
	template <typename ReadHandler>
	inline basic_tap_adatper_service<TapAdapterImplementation>::read_operation<ReadHandler>::read_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::mutable_buffer& buffer, ReadHandler handler) :
		m_impl(impl),
		m_io_service(io_service),
		m_work(m_io_service),
		m_buffer(buffer),
		m_handler(handler)
	{
	}

	template <typename TapAdapterImplementation>
	template <typename ReadHandler>
	inline void basic_tap_adatper_service<TapAdapterImplementation>::read_operation<ReadHandler>::operator()() const
	{
		implementation_type impl = m_impl.lock();

		if (impl)
		{
			unsigned char* data = boost::asio::buffer_cast<unsigned char*>(m_buffer);
			size_t data_len = boost::asio::buffer_size(m_buffer);

			impl->begin_read(data, data_len);

			try
			{
				size_t cnt = impl->end_read(boost::posix_time::time_duration());
				boost::system::error_code ec;

				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ec, cnt));
			}
			catch (boost::system::system_error& ex)
			{
				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ex, 0));
			}
		} else
		{
			this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, boost::asio::error::operation_aborted, 0));
		}
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adatper_service<TapAdapterImplementation>::shutdown_service()
	{
	}
}

#endif /* ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP */

/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
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
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The basic tap adapter service class.
 */

#ifndef ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP
#define ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

#include "tap_adapter_impl.hpp"

namespace asiotap
{
	template <typename TapAdapterImplementation = tap_adapter_impl>
	class basic_tap_adapter_service : public boost::asio::io_service::service
	{
		public:

			/**
			 * \brief The implementation type.
			 */
			typedef TapAdapterImplementation raw_implementation_type;

			/**
			 * \brief The adapter type.
			 */
			typedef typename raw_implementation_type::adapter_type adapter_type;

			/**
			 * \brief The implementation pointer type.
			 */
			typedef typename boost::shared_ptr<raw_implementation_type> implementation_type;

			/**
			 * \brief Enumerate the available tap adapter on the system.
			 * \return The identifiers and names of the available tap adapters on the system.
			 */
			static std::map<std::string, std::string> enumerate();

			/**
			 * \brief The service identifier.
			 */
			static boost::asio::io_service::id id;

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to register to.
			 */
			explicit basic_tap_adapter_service(boost::asio::io_service &io_service);

			/**
			 * \brief The destructor.
			 */
			~basic_tap_adapter_service();

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
			 * \brief Open the specified implementation.
			 * \param impl The implementation that must be opened.
			 * \param name The name of the device to open.
			 * \param mtu The mtu of the device. Specify 0 to get an automatic value.
			 * \param type The adapter type.
			 */
			void open(implementation_type& impl, const std::string& name, unsigned int mtu, adapter_type type);

			/**
			 * \brief Close the specified implementation.
			 * \param impl The implementation to close.
			 */
			void close(implementation_type& impl);

			/**
			 * \brief Process to an asynchronous read on the specified implementation.
			 * \param impl The implementation on which to perform the read.
			 * \param buffer The buffer.
			 * \param handler The handler.
			 */
			template<typename ReadHandler>
			void async_read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, ReadHandler handler);

			/**
			 * \brief Process to an asynchronous write on the specified implementation.
			 * \param impl The implementation on which to perform the write.
			 * \param buffer The buffer.
			 * \param handler The handler.
			 */
			template<typename WriteHandler>
			void async_write(implementation_type& impl, const boost::asio::const_buffer& buffer, WriteHandler handler);

			/**
			 * \brief Process to a synchronous read on the specified implementation.
			 * \param impl The implementation on which to perform the read.
			 * \param buffer The buffer.
			 * \param ec The returned error, if any.
			 * \return The count of bytes read.
			 */
			size_t read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec);

			/**
			 * \brief Process to a synchronous write on the specified implementation.
			 * \param impl The implementation on which to perform the write.
			 * \param buffer The buffer.
			 * \param ec The returned error, if any.
			 * \return The count of bytes write.
			 */
			size_t write(implementation_type& impl, const boost::asio::const_buffer& buffer, boost::system::error_code& ec);

		private:

			template <typename ReadHandler>
			class read_operation
			{
				public:

					read_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::mutable_buffer& buffer, ReadHandler handler);
					void operator()() const;

				private:

					typename boost::weak_ptr<raw_implementation_type> m_impl;
					boost::asio::io_service& m_io_service;
					boost::asio::io_service::work m_work;
					boost::asio::mutable_buffer m_buffer;
					ReadHandler m_handler;
			};

			template <typename WriteHandler>
			class write_operation
			{
				public:

					write_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::const_buffer& buffer, WriteHandler handler);
					void operator()() const;

				private:

					typename boost::weak_ptr<raw_implementation_type> m_impl;
					boost::asio::io_service& m_io_service;
					boost::asio::io_service::work m_work;
					boost::asio::const_buffer m_buffer;
					WriteHandler m_handler;
			};

			void shutdown_service();

			boost::asio::io_service m_async_io_service;
			boost::scoped_ptr<boost::asio::io_service::work> m_async_work;
			boost::thread m_async_thread;
	};

	template <typename TapAdapterImplementation>
	inline std::map<std::string, std::string> basic_tap_adapter_service<TapAdapterImplementation>::enumerate()
	{
		return TapAdapterImplementation::enumerate();
	}

	template <typename TapAdapterImplementation>
	boost::asio::io_service::id basic_tap_adapter_service<TapAdapterImplementation>::id;

	template <typename TapAdapterImplementation>
	inline basic_tap_adapter_service<TapAdapterImplementation>::basic_tap_adapter_service(boost::asio::io_service &_io_service) :
		boost::asio::io_service::service(_io_service),
		m_async_work(new boost::asio::io_service::work(m_async_io_service)),
		m_async_thread(boost::bind(&boost::asio::io_service::run, &m_async_io_service))
	{
	}

	template <typename TapAdapterImplementation>
	inline basic_tap_adapter_service<TapAdapterImplementation>::~basic_tap_adapter_service()
	{
		m_async_work.reset();
		m_async_io_service.stop();
		m_async_thread.join();
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::construct(implementation_type& impl)
	{
		impl.reset(new TapAdapterImplementation());
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::destroy(implementation_type& impl)
	{
		impl->close();
		impl.reset();
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::open(implementation_type& impl, const std::string& name, unsigned int mtu, basic_tap_adapter_service<TapAdapterImplementation>::adapter_type type)
	{
		impl->open(name, mtu, type);
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::close(implementation_type& impl)
	{
		impl->close();
	}

	template<typename TapAdapterImplementation> template<typename ReadHandler>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::async_read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, ReadHandler handler)
	{
		this->m_async_io_service.post(read_operation<ReadHandler>(impl, this->get_io_service(), buffer, handler));
	}

	template<typename TapAdapterImplementation> template<typename WriteHandler>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::async_write(implementation_type& impl, const boost::asio::const_buffer& buffer, WriteHandler handler)
	{
		this->m_async_io_service.post(write_operation<WriteHandler>(impl, this->get_io_service(), buffer, handler));
	}

	template<typename TapAdapterImplementation>
	inline size_t basic_tap_adapter_service<TapAdapterImplementation>::read(implementation_type& impl, const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec)
	{
		try
		{
			ec = boost::system::error_code();

			unsigned char* data = boost::asio::buffer_cast<unsigned char*>(buffer);
			size_t data_len = boost::asio::buffer_size(buffer);

			return impl->read(data, data_len);
		}
		catch (boost::system::system_error& ex)
		{
			ec = ex.code();
			return 0;
		}
	}

	template<typename TapAdapterImplementation>
	inline size_t basic_tap_adapter_service<TapAdapterImplementation>::write(implementation_type& impl, const boost::asio::const_buffer& buffer, boost::system::error_code& ec)
	{
		try
		{
			ec = boost::system::error_code();

			const unsigned char* data = boost::asio::buffer_cast<const unsigned char*>(buffer);
			size_t data_len = boost::asio::buffer_size(buffer);

			return impl->write(data, data_len);
		}
		catch (boost::system::system_error& ex)
		{
			ec = ex.code();
			return 0;
		}
	}

	template <typename TapAdapterImplementation>
	template <typename ReadHandler>
	inline basic_tap_adapter_service<TapAdapterImplementation>::read_operation<ReadHandler>::read_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::mutable_buffer& buffer, ReadHandler handler) :
		m_impl(impl),
		m_io_service(io_service),
		m_work(m_io_service),
		m_buffer(buffer),
		m_handler(handler)
	{
	}

	template <typename TapAdapterImplementation>
	template <typename ReadHandler>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::read_operation<ReadHandler>::operator()() const
	{
		implementation_type impl = m_impl.lock();

		if (impl)
		{
			unsigned char* data = boost::asio::buffer_cast<unsigned char*>(m_buffer);
			size_t data_len = boost::asio::buffer_size(m_buffer);

#ifdef MACINTOSH
			try
			{
				const size_t cnt = impl->read(data, data_len);

				boost::system::error_code ec;

				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ec, cnt));
			}
			catch (boost::system::system_error& ex)
			{
				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ex.code(), 0));
			}
#else
			impl->begin_read(data, data_len);

			try
			{
				size_t cnt;

				if (impl->end_read(cnt))
				{
					boost::system::error_code ec;

					this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ec, cnt));
				}
				else
				{
					impl->cancel_read();
					this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, boost::asio::error::operation_aborted, 0));
				}
			}
			catch (boost::system::system_error& ex)
			{
				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ex.code(), 0));
			}
#endif
		}
		else
		{
			this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, boost::asio::error::operation_aborted, 0));
		}
	}

	template <typename TapAdapterImplementation>
	template <typename WriteHandler>
	inline basic_tap_adapter_service<TapAdapterImplementation>::write_operation<WriteHandler>::write_operation(implementation_type& impl, boost::asio::io_service& io_service, const boost::asio::const_buffer& buffer, WriteHandler handler) :
		m_impl(impl),
		m_io_service(io_service),
		m_work(m_io_service),
		m_buffer(buffer),
		m_handler(handler)
	{
	}

	template <typename TapAdapterImplementation>
	template <typename WriteHandler>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::write_operation<WriteHandler>::operator()() const
	{
		implementation_type impl = m_impl.lock();

		if (impl)
		{
			const unsigned char* data = boost::asio::buffer_cast<const unsigned char*>(m_buffer);
			size_t data_len = boost::asio::buffer_size(m_buffer);

#ifdef MACINTOSH
			try
			{
				const size_t cnt = impl->write(data, data_len);

				boost::system::error_code ec;

				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ec, cnt));
			}
			catch (boost::system::system_error& ex)
			{
				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ex.code(), 0));
			}
#else
			impl->begin_write(data, data_len);

			try
			{
				size_t cnt;

				if (impl->end_write(cnt))
				{
					boost::system::error_code ec;

					this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ec, cnt));
				}
				else
				{
					impl->cancel_write();
					this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, boost::asio::error::operation_aborted, 0));
				}
			}
			catch (boost::system::system_error& ex)
			{
				this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, ex.code(), 0));
			}
#endif
		}
		else
		{
			this->m_io_service.post(boost::asio::detail::bind_handler(m_handler, boost::asio::error::operation_aborted, 0));
		}
	}

	template <typename TapAdapterImplementation>
	inline void basic_tap_adapter_service<TapAdapterImplementation>::shutdown_service()
	{
	}
}

#endif /* ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP */

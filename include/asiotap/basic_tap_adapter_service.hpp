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

		private:

			void shutdown_service();
	};

	template <typename TapAdapterImplementation>
	boost::asio::io_service::id basic_tap_adatper_service<TapAdapterImplementation>::id;
	
	template <typename TapAdapterImplementation>
	basic_tap_adatper_service<TapAdapterImplementation>::basic_tap_adatper_service(boost::asio::io_service &_io_service) :
		boost::asio::io_service::service(_io_service)
	{
	}

	template <typename TapAdapterImplementation>
	void basic_tap_adatper_service<TapAdapterImplementation>::construct(implementation_type& impl)
	{
		impl.reset(new TapAdapterImplementation());
	}

	template <typename TapAdapterImplementation>
	void basic_tap_adatper_service<TapAdapterImplementation>::destroy(implementation_type& impl)
	{
		impl.reset();
	}
	
	template <typename TapAdapterImplementation>
	void basic_tap_adatper_service<TapAdapterImplementation>::shutdown_service()
	{
	}
}

#endif /* ASIOTAP_BASIC_TAP_ADAPTER_SERVICE_HPP */

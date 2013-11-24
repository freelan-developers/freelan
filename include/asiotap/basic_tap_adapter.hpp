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
 * \file basic_tap_adapter.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The basic tap adapter class.
 */

#ifndef ASIOTAP_BASIC_TAP_ADAPTER_HPP
#define ASIOTAP_BASIC_TAP_ADAPTER_HPP

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <string>
#include <map>
#include <vector>

namespace asiotap
{
	/**
	 * \brief A basic tap adapter class.
	 */
	template <typename Service>
	class basic_tap_adapter : public boost::asio::basic_io_object<Service>
	{
		public:

			/**
			 * \brief The adapter type.
			 */
			enum adapter_type
			{
				AT_TAP_ADAPTER = Service::raw_implementation_type::AT_TAP_ADAPTER,
				AT_TUN_ADAPTER = Service::raw_implementation_type::AT_TUN_ADAPTER
			};

			/**
			 * \brief The ethernet address type.
			 */
			typedef typename Service::implementation_type::element_type::ethernet_address_type ethernet_address_type;

			/**
			 * \brief An IP address.
			 */
			typedef typename Service::implementation_type::element_type::ip_address ip_address;

			/**
			 * \brief A list of IP addresses.
			 */
			typedef typename Service::implementation_type::element_type::ip_address_list ip_address_list;

			/**
			 * \brief Enumerate the available tap adapter on the system.
			 * \return The identifiers and names of the available tap adapters on the system.
			 */
			static std::map<std::string, std::string> enumerate();

			/**
			 * \brief The constructor.
			 * \param io_service The io_service to use.
			 */
			explicit basic_tap_adapter(boost::asio::io_service& io_service);

			/**
			 * \brief Check if the tap adapter is open.
			 * \return true if the tap adapter is open, false otherwise.
			 */
			bool is_open() const;

			/**
			 * \brief Open the tap adapter.
			 * \param name The name of the tap adapter to open. On Windows a GUID is expected. If name is NULL, a device is selected/created automatically.
			 * \param mtu The mtu of the device. Specify 0 to get an automatic value.
			 * \param type The adapter type.
			 */
			void open(const std::string& name = "", unsigned int mtu = 0, adapter_type type = AT_TAP_ADAPTER);

			/**
			 * \brief Close the tap adapter.
			 */
			void close();

			/**
			 * \brief Set the connected state of the device.
			 * \param connected If true, set the device's state as connected.
			 * \warning The device must be open (see is_open()) or the behavior is undefined.
			 *
			 * This function does nothing on *NIX.
			 */
			void set_connected_state(bool connected);

			/**
			 * \brief Get the device name, as specified during a call to open().
			 * \return The device name. On Windows, a GUID is returned.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			const std::string& name() const;

			/**
			 * \brief Get the device MTU.
			 * \return The device MTU.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			unsigned int mtu() const;

			/**
			 * \brief Get the adapter type.
			 * \return The adapter type.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			adapter_type type() const;

			/**
			 * \brief Get the Ethernet address.
			 * \return The ethernet address.
			 * \warning The device must be opened or the returned value is unspecified.
			 */
			const ethernet_address_type& ethernet_address() const;

			/**
			 * \brief Process to an asynchronous read.
			 * \param buffer The buffer to read the data to.
			 * \param handler The completion handler.
			 */
			template<typename ReadHandler>
			void async_read(const boost::asio::mutable_buffer& buffer, ReadHandler handler);

			/**
			 * \brief Process to an asynchronous write.
			 * \param buffer The buffer to write the data to.
			 * \param handler The completion handler.
			 */
			template<typename WriteHandler>
			void async_write(const boost::asio::const_buffer& buffer, WriteHandler handler);

			/**
			 * \brief Cancel any pending read operation.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel_read();

			/**
			 * \brief Cancel any pending write operation.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel_write();

			/**
			 * \brief Cancel any pending operation on the device.
			 * \warning This is only supported on Windows Vista and earlier versions. If the cancelling fails, an exception is thrown.
			 */
			void cancel();

			/**
			 * \brief Process to a blocking read.
			 * \param buffer The buffer to read the data to.
			 * \warning On error, a boost::system::system_error is thrown.
			 * \return The count of bytes read.
			 */
			size_t read(const boost::asio::mutable_buffer& buffer);

			/**
			 * \brief Process to a blocking read.
			 * \param buffer The buffer to read the data to.
			 * \param ec The returned error code.
			 * \return The count of bytes read.
			 */
			size_t read(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec);

			/**
			 * \brief Process to a blocking write.
			 * \param buffer The buffer to write the data to.
			 * \return The count of bytes written.
			 * \warning On error, a boost::system::system_error is thrown.
			 */
			size_t write(const boost::asio::const_buffer& buffer);

			/**
			 * \brief Process to a blocking write.
			 * \param buffer The buffer to write the data to.
			 * \param ec The returned error code.
			 * \return The count of bytes written.
			 */
			size_t write(const boost::asio::const_buffer& buffer, boost::system::error_code& ec);

			/**
			 * \brief Add an IP address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool add_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool remove_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len);

			/**
			 * \brief Get the list of set IP addresses and prefix lengths.
			 * \return The list of set IP addresses and prefix lengths.
			 */
			ip_address_list get_ip_addresses() const;

			/**
			 * \brief Add an IPv4 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool remove_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len);

			/**
			 * \brief Add an IPv6 address to the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool add_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

			/**
			 * \brief Remove an IP address from the tap adapter.
			 * \param address The address.
			 * \param prefix_len The prefix length, in bits.
			 * \return true if the operation succeeded.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool remove_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len);

			/**
			 * \brief Set the point-to-point address on the tap adaper.
			 * \param address The address.
			 * \return true if the operation succeeded.
			 * \warning Performing this operation on a device that is not in TUN mode has undefined behavior.
			 * \warning Regardless of the return status, if a serious error occurs, an exception will still be thrown.
			 * \warning On most operating system, administrative privileges are usually required to perform this operation.
			 */
			bool set_remote_ip_address_v4(const boost::asio::ip::address_v4& address);
	};

	template <typename Service>
	inline std::map<std::string, std::string> basic_tap_adapter<Service>::enumerate()
	{
		return Service::enumerate();
	}

	template <typename Service>
	inline basic_tap_adapter<Service>::basic_tap_adapter(boost::asio::io_service& _io_service) :
		boost::asio::basic_io_object<Service>(_io_service)
	{
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::is_open() const
	{
		return this->implementation->is_open();
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::open(const std::string& _name, unsigned int _mtu, adapter_type _type)
	{
		this->service.open(this->implementation, _name, _mtu, static_cast<typename Service::raw_implementation_type::adapter_type>(_type));
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::close()
	{
		this->service.close(this->implementation);
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::set_connected_state(bool connected)
	{
		this->implementation->set_connected_state(connected);
	}

	template <typename Service>
	inline const std::string& basic_tap_adapter<Service>::name() const
	{
		return this->implementation->name();
	}

	template <typename Service>
	inline unsigned int basic_tap_adapter<Service>::mtu() const
	{
		return this->implementation->mtu();
	}

	template <typename Service>
	inline typename basic_tap_adapter<Service>::adapter_type basic_tap_adapter<Service>::type() const
	{
		return static_cast<typename basic_tap_adapter<Service>::adapter_type>(this->implementation->type());
	}

	template <typename Service>
	inline const typename basic_tap_adapter<Service>::ethernet_address_type& basic_tap_adapter<Service>::ethernet_address() const
	{
		return this->implementation->ethernet_address();
	}

	template <typename Service>
	template<typename ReadHandler>
	void basic_tap_adapter<Service>::async_read(const boost::asio::mutable_buffer& buffer, ReadHandler handler)
	{
		this->service.async_read(this->implementation, buffer, handler);
	}

	template <typename Service>
	template<typename WriteHandler>
	void basic_tap_adapter<Service>::async_write(const boost::asio::const_buffer& buffer, WriteHandler handler)
	{
		this->service.async_write(this->implementation, buffer, handler);
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::cancel_read()
	{
		this->implementation->cancel_read();
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::cancel_write()
	{
		this->implementation->cancel_write();
	}

	template <typename Service>
	inline void basic_tap_adapter<Service>::cancel()
	{
		this->implementation->cancel();
	}

	template <typename Service>
	inline size_t basic_tap_adapter<Service>::read(const boost::asio::mutable_buffer& buffer)
	{
		boost::system::error_code ec;

		size_t result = this->service.read(this->implementation, buffer, ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}

		return result;
	}

	template <typename Service>
	inline size_t basic_tap_adapter<Service>::read(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec)
	{
		return this->service.read(this->implementation, buffer, ec);
	}

	template <typename Service>
	inline size_t basic_tap_adapter<Service>::write(const boost::asio::const_buffer& buffer)
	{
		boost::system::error_code ec;

		size_t result = this->service.write(this->implementation, buffer, ec);

		if (ec)
		{
			throw boost::system::system_error(ec);
		}

		return result;
	}

	template <typename Service>
	inline size_t basic_tap_adapter<Service>::write(const boost::asio::const_buffer& buffer, boost::system::error_code& ec)
	{
		return this->service.write(this->implementation, buffer, ec);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::add_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
	{
		return this->implementation->add_ip_address(address, prefix_len);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::remove_ip_address(const boost::asio::ip::address& address, unsigned int prefix_len)
	{
		return this->implementation->remove_ip_address(address, prefix_len);
	}

	template <typename Service>
	inline typename basic_tap_adapter<Service>::ip_address_list basic_tap_adapter<Service>::get_ip_addresses() const
	{
		return this->implementation->get_ip_addresses();
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::add_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len)
	{
		return this->implementation->add_ip_address_v4(address, prefix_len);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::remove_ip_address_v4(const boost::asio::ip::address_v4& address, unsigned int prefix_len)
	{
		return this->implementation->remove_ip_address_v4(address, prefix_len);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::add_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len)
	{
		return this->implementation->add_ip_address_v6(address, prefix_len);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::remove_ip_address_v6(const boost::asio::ip::address_v6& address, unsigned int prefix_len)
	{
		return this->implementation->remove_ip_address_v6(address, prefix_len);
	}

	template <typename Service>
	inline bool basic_tap_adapter<Service>::set_remote_ip_address_v4(const boost::asio::ip::address_v4& address)
	{
		return this->implementation->set_remote_ip_address_v4(address);
	}
}

#endif /* ASIOTAP_BASIC_TAP_ADAPTER_HPP */
